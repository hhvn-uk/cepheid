#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <raylib.h>
#include <assert.h>
#include "main.h"

static Polar
sys_get_polar(Body *body) {
	Polar polar;

	if (body->polar.r != INFINITY &&
			body->polar.theta != INFINITY)
		return body->polar;

	if (body->type == BODY_COMET) {
		if (!body->parent) {
			polar.r = body->curdist;
			polar.theta = body->theta;
		} else {
			polar = polar_add(sys_get_polar(body->parent),
					(Polar){body->curdist, body->theta});
		}
	} else {
		if (!body->parent) {
			polar.r = body->dist;
			polar.theta = body->curtheta;
		} else {
			polar = polar_add(sys_get_polar(body->parent),
					(Polar){body->dist, body->curtheta});
		}
	}

	body->polar = polar;
	return polar;
}

System *
sys_init(char *name) {
	System *ret = malloc(sizeof(System));
	if (!ret) return NULL;
	ret->name = nstrdup(name);
	ret->t = NULL;
	ret->furthest_body = NULL;
	memset(&ret->num, 0, sizeof(ret->num));
	return ret;
}

static int
sys_tree_compar(Tree *a, Tree *b, void *data) {
	System *sa, *sb;
	Body *p;
	float v1, v2;

	assert(a->type == b->type);

	switch (a->type) {
	case SYSTREE_SYS:
		sa = a->data;
		sb = b->data;
		return (abs(sa->lypos.x) + abs(sa->lypos.y)) -
			(abs(sb->lypos.x) + abs(sb->lypos.y));
	case SYSTREE_BODY:
		for (p = a->data, v1 = 0; p->parent; p = p->parent)
			v1 += (p->type == BODY_COMET ? p->maxdist : p->dist);
		for (p = b->data, v2 = 0; p->parent; p = p->parent)
			v2 += (p->type == BODY_COMET ? p->maxdist : p->dist);
		return (v1 == v2 ? 0 : (v1 > v2 ? 1 : -1));
	}

	return 0;
}

void
sys_tree_load(void) {
	Tree *t, *bt;
	System *s;
	char **bn;
	Body **bp;
	int bl, i, n, pos;

	/* initialize systems and bodies */
	dbgettree(save->db.systems, &save->systems, sys_tree_getter);

	/* bn & bp are only free'd at the end to avoid excess (re)allocations */
	bl = 10;
	bn = malloc(bl * sizeof(char *));
	bp = malloc(bl * sizeof(char *));

	for (t = save->systems.d; t; t = t->n) {
		s = t->data;

		/* first pass: init names & pointer arrays */
		for (i = 0, bt = t->d; bt; bt = bt->n, i++) {
			if (i == bl - 1) {
				bl += 10;
				bn = realloc(bn, bl * sizeof(char *));
				bp = realloc(bp, bl * sizeof(char *));
			}

			bn[i] = bt->name;
			bp[i] = bt->data;
		}

		n = i;

		/* second pass: assign parent pointer */
		for (i = 0; i < n; i++) {
			if ((pos = strlistpos(bp[i]->pname, bn, n)) != -1) {
				free(bp[i]->pname);
				bp[i]->parent = bp[pos];
			} else {
				bp[i]->parent = NULL;
			}
		}

		/* third pass: get coords (needs parent ptr from second pass) */
		for (i = 0; i < n; i++) {
			sys_get_polar(bp[i]); /* Builds the cache for us: this
						 is more efficient as it can
						 cache the parent too */
			bp[i]->vector = vectorize(bp[i]->polar);

			/* This could deal with moons, but that's probably not
			 * useful. What about multiple stars in a system? That
			 * may need to be addressed in future */
			if (bp[i]->parent && bp[i]->parent->type == BODY_STAR && (!s->furthest_body ||
					(bp[i]->type == BODY_COMET ? bp[i]->maxdist : bp[i]->dist) >
					(s->furthest_body->type == BODY_COMET ? s->furthest_body->maxdist : s->furthest_body->dist)))
				s->furthest_body = bp[i];
		}
	}

	free(bp);
	free(bn);

	tree_sort(&save->systems, sys_tree_compar, NULL);
}

char *
sys_tree_getter(char *dir, char *group, char *name, int depth, Tree *t) {
	System *s;
	Body *b;

	if (depth == 1) {
		s = sys_init(name);
		s->t = t;

		bdbget(dir, group, 'v', "lypos", &s->lypos);

		t->type = SYSTREE_SYS;
		t->data = s;
		return s->name;
	} else {
		s = t->u->data; /* parent should be a system */
		b = body_init(name);
		b->t = t;

		b->pname = nstrdup(dbget(dir, group, "parent"));

		b->type = bodytype_enumify(dbget(dir, group, "type"));
		switch (b->type) {
		case BODY_STAR:		s->num.stars++;		break;
		case BODY_PLANET:	s->num.planets++;	break;
		case BODY_DWARF:	s->num.dwarfs++;	break;
		case BODY_ASTEROID:	s->num.asteroids++;	break;
		case BODY_COMET:	s->num.comets++;	break;
		case BODY_MOON:		s->num.moons++;		break;
		case BODY_LAST: /* shut up compiler */          break;
		}

		bdbget(dir, group,
		'f', "radius", &b->radius,
		'f', "mass", &b->mass,
		'f', "orbdays", &b->orbdays);
		if (b->type == BODY_COMET) {
			bdbget(dir, group,
			'f', "mindist", &b->mindist,
			'f', "maxdist", &b->maxdist,
			'f', "curdist", &b->curdist,
			'f', "theta", &b->theta,
			'i', "inward", &b->inward);
		} else {
			bdbget(dir, group,
			'f', "dist", &b->dist,
			'f', "curtheta", &b->curtheta);
		}

		t->type = SYSTREE_BODY;
		t->data = b;
		return b->name;
	}
}

void
sys_tree_setter(char *dir, char *group, char *name, int depth, Tree *t) {
	System *s;
	Body *b;

	switch (t->type) {
	case SYSTREE_SYS:
		s = t->data;
		bdbset(dir, group, 'v', "lypos", s->lypos);
		break;
	case SYSTREE_BODY:
		b = t->data;

		if (b->parent)
			dbset(save->db.systems, group, "parent", b->parent->name);

		bdbset(dir, group,
		'f', "radius", b->radius,
		'f', "mass", b->mass,
		'f', "orbdays", b->orbdays);
		if (b->type == BODY_COMET) {
			bdbset(dir, group,
			'f', "mindist", b->mindist,
			'f', "maxdist", b->maxdist,
			'f', "curdist", b->curdist,
			'f', "theta", b->theta,
			'i', "inward", b->inward);
		} else {
			bdbset(dir, group,
			'f', "dist", b->dist,
			'f', "curtheta", b->curtheta);
		}
		break;
	}
}

System *
sys_get(char *name) {
	Tree *t;

	for (t = save->systems.d; t; t = t->n)
		if (streq(t->name, name))
			return t->data;

	return NULL;
}

System *
sys_default(void) {
	char *str;

	if (view_main.sys)
		return view_main.sys;
	else if ((str = dbget(save->db.dir, "index", "selsystem")))
		return sys_get(str);
	else if (save->homesys)
		return save->homesys;
	else if (save->systems.d && save->systems.d->data)
		return save->systems.d->data;
	else
		exit(1);
}
