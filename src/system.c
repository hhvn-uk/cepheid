#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <raylib.h>
#include "main.h"

Vector2
sys_vectorize(Polar polar) {
	return (Vector2) {
		polar.r * cosf_d(polar.theta),
		polar.r * sinf_d(polar.theta)
	};
}

Vector2
sys_vectorize_around(Vector2 around, Polar polar) {
	Vector2 relative = sys_vectorize(polar);
	return (Vector2) {
		around.x + relative.x,
		around.y + relative.y
	};
}

Polar
sys_polarize(Vector2 vector) {
	return (Polar) {
		hypotf(vector.x, vector.y),
		atan2f_d(vector.y, vector.x)
	};
}

Polar
sys_polarize_around(Vector2 around, Vector2 vector) {
	Vector2 v = {vector.y - around.y, vector.x - around.x};
	return sys_polarize(v);
}

Polar
sys_sum_polar(Polar absolute, Polar relative) {
	return sys_polarize(sys_vectorize_around(sys_vectorize(absolute), relative));
}

Vector2
sys_get_vector(Body *body) {
	return sys_vectorize(sys_get_polar(body));
}

Polar
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
			polar = sys_sum_polar(sys_get_polar(body->parent),
					(Polar){body->curdist, body->theta});
		}
	} else {
		if (!body->parent) {
			polar.r = body->dist;
			polar.theta = body->curtheta;
		} else {
			polar = sys_sum_polar(sys_get_polar(body->parent),
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
filter_bodyinsystem(void *data, char *path) {
	char *system = data;

	if (!strstr(path, "/index") && strncmp(path, system, strlen(system)) != 0)
		return 1;
	else
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
			bp[i]->vector = sys_vectorize(bp[i]->polar);

			/* This could deal with moons, but that's probably not
			 * useful. What about multiple stars in a system? That
			 * may need to be addressed in future */
			if (bp[i]->parent && bp[i]->parent->type == BODY_STAR && (!s->furthest_body ||
					(bp[i]->type == BODY_COMET ? bp[i]->maxdist : bp[i]->dist) >
					(s->furthest_body->type == BODY_COMET ? s->furthest_body->maxdist : s->furthest_body->dist)))
				s->furthest_body = bp[i];
		}

		body_sort(bp, n);
	}

	free(bp);
	free(bn);
}

char *
sys_tree_getter(char *dir, char *group, char *name, int depth, Tree *t) {
	System *s;
	Body *b;

	if (depth == 1) {
		s = sys_init(name);
		s->t = t;

		s->lypos.x = dbgetfloat(dir, group, "x");
		s->lypos.y = dbgetfloat(dir, group, "y");

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
		}

		b->radius = dbgetfloat(dir, group, "radius");
		b->mass = dbgetfloat(dir, group, "mass");
		b->orbdays = dbgetfloat(dir, group, "orbdays");
		if (b->type == BODY_COMET) {
			/* mindist is on opposite side of parent */
			b->mindist = 0 - dbgetfloat(dir, group, "mindist");
			b->maxdist = dbgetfloat(dir, group, "maxdist");
			b->curdist = dbgetfloat(dir, group, "curdist");
			b->theta = dbgetfloat(dir, group, "theta");
			b->inward = dbgetfloat(dir, group, "inward");
		} else {
			b->dist = dbgetfloat(dir, group, "dist");
			b->curtheta = dbgetfloat(dir, group, "curtheta");
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
		dbsetfloat(dir, group, "x", s->lypos.x);
		dbsetfloat(dir, group, "y", s->lypos.y);
	case SYSTREE_BODY:
		b = t->data;

		if (b->parent)
			dbset(save->db.systems, group, "parent", b->parent->name);

		dbsetfloat(dir, group, "radius", b->radius);
		dbsetfloat(dir, group, "mass", b->mass);
		dbsetfloat(dir, group, "orbdays", b->orbdays);
		if (b->type == BODY_COMET) {
			dbsetfloat(dir, group, "mindist", b->mindist);
			dbsetfloat(dir, group, "maxdist", b->maxdist);
			dbsetfloat(dir, group, "curdist", b->curdist);
			dbsetfloat(dir, group, "theta", b->theta);
			dbsetfloat(dir, group, "inward", b->inward);
		} else {
			dbsetfloat(dir, group, "dist", b->dist);
			dbsetfloat(dir, group, "curtheta", b->curtheta);
		}
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
	else
		return save->homesys;
}
