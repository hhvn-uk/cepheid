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
	ret->bodies = NULL;
	ret->bodies_len = 0;
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

/* If s is true, ignore name and load the system.
 * If s is NULL, call sys_init(name); */
System *
sys_load(System *s, char *name) {
	char *dir, *tmp;
	char **bname;
	char **bparent;
	size_t blen, i;
	int pos;

	if (!s) s = sys_init(name);

	s->lypos.x = strnum(dbget(save->db.systems, name, "x"));
	s->lypos.y = strnum(dbget(save->db.systems, name, "y"));

	dir = smprintf("%s/", s->name);
	s->bodies_len = blen = dblistgroups_f(&bname, save->db.systems, &filter_bodyinsystem, dir);
	if (!bname) return NULL;
	bparent = malloc(sizeof(char *) * blen);
	s->bodies = malloc(sizeof(Body *) * blen);

	if (!bparent || !s->bodies)
		return NULL;

	/* first pass: init bodies and parents */
	for (i = 0; i < blen; i++) {
		s->bodies[i] = malloc(sizeof(Body));
		s->bodies[i]->name = nstrdup(bname[i] + strlen(dir));
		bparent[i] = nstrdup(dbget(save->db.systems, bname[i], "parent"));

		tmp = dbget(save->db.systems, bname[i], "type");
		s->bodies[i]->type = bodytype_enumify(tmp);

		switch (s->bodies[i]->type) {
		case BODY_STAR:		s->num.stars++;		break;
		case BODY_PLANET:	s->num.planets++;	break;
		case BODY_DWARF:	s->num.dwarfs++;	break;
		case BODY_ASTEROID:	s->num.asteroids++;	break;
		case BODY_COMET:	s->num.comets++;	break;
		case BODY_MOON:		s->num.moons++;		break;
		}

		s->bodies[i]->radius = strnum(dbget(save->db.systems, bname[i], "radius"));
		s->bodies[i]->mass = strnum(dbget(save->db.systems, bname[i], "mass"));
		s->bodies[i]->orbdays = strnum(dbget(save->db.systems, bname[i], "orbdays"));
		if (s->bodies[i]->type == BODY_COMET) {
			/* mindist is on opposite side of parent */
			s->bodies[i]->mindist = 0 - strnum(dbget(save->db.systems, bname[i], "mindist"));
			s->bodies[i]->maxdist = strnum(dbget(save->db.systems, bname[i], "maxdist"));
			s->bodies[i]->curdist = strnum(dbget(save->db.systems, bname[i], "curdist"));
			s->bodies[i]->theta = strnum(dbget(save->db.systems, bname[i], "theta"));
			s->bodies[i]->inward = strnum(dbget(save->db.systems, bname[i], "inward"));
		} else {
			s->bodies[i]->dist = strnum(dbget(save->db.systems, bname[i], "dist"));
			s->bodies[i]->curtheta = strnum(dbget(save->db.systems, bname[i], "curtheta"));
		}

		/* so sys_get_polar() knows if it's usable */
		s->bodies[i]->polar = (Polar) { INFINITY, INFINITY };
	}

	/* second pass: assign parents (needs bparent[] from first pass) */
	for (i = 0; i < blen; i++) {
		tmp = smprintf("%s%s", dir, bparent[i]);
		if ((pos = strlistpos(tmp, bname, blen)) != -1)
			s->bodies[i]->parent = s->bodies[pos];
		else
			s->bodies[i]->parent = NULL;
		free(tmp);
	}

	/* third pass: get coords (needs parent ptr from second pass) */
	for (i = 0; i < blen; i++) {
		sys_get_polar(s->bodies[i]); /* Builds the cache for us: this is more
						   efficient as it can cache the parent too */
		s->bodies[i]->vector = sys_vectorize(s->bodies[i]->polar);

		/* This could deal with moons, but that's probably not useful.
		 * What about multiple stars in a system? That may need to be
		 * addressed in future */
		if (s->bodies[i]->parent && s->bodies[i]->parent->type == BODY_STAR && (!s->furthest_body ||
				(s->bodies[i]->type == BODY_COMET ? s->bodies[i]->maxdist : s->bodies[i]->dist) >
				(s->furthest_body->type == BODY_COMET ? s->furthest_body->maxdist : s->furthest_body->dist)))
			s->furthest_body = s->bodies[i];
	}

	for (i = 0; i < blen; i++) {
		free(bparent[i]);
	}
	free(bparent);
	dblistfree(bname, blen);
	free(dir);

	body_sort(s->bodies, s->bodies_len);

	return s;
}

System *
sys_get(char *name) {
	/* For now, call sys_load. In future, get the system via save. */
	return sys_load(NULL, name);
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
