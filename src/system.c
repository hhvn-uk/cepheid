#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "raylib.h"
#include "main.h"

char *body_names[BODY_LAST] = {
	[BODY_STAR] = "Star",
	[BODY_PLANET] = "Planet",
	[BODY_DWARF] = "Dwarf planet",
	[BODY_ASTEROID] = "Asteroid",
	[BODY_COMET] = "Comet",
	[BODY_MOON] = "Moon",
	[BODY_PLANET + BODY_MOON] = "Moon",
	[BODY_DWARF + BODY_MOON] = "Dwarf planet moon",
	[BODY_ASTEROID + BODY_MOON] = "Asteroid moon",
	[BODY_COMET + BODY_MOON] = "Comet moon",
};

Vector2
system_vectorize(Polar polar) {
	return (Vector2) {
		polar.r * cosf(polar.theta),
		polar.r * sinf(polar.theta)
	};
}

Vector2
system_vectorize_around(Vector2 around, Polar polar) {
	Vector2 relative = system_vectorize(polar);
	return (Vector2) {
		around.x + relative.x,
		around.y + relative.y
	};
}

Polar
system_polarize(Vector2 vector) {
	return (Polar) {
		hypotf(vector.x, vector.y),
		atan2f(vector.y, vector.x)
	};
}

Polar
system_sum_polar(Polar absolute, Polar relative) {
	return system_polarize(system_vectorize_around(system_vectorize(absolute), relative));
}

Vector2
system_get_vector(Body *body) {
	return system_vectorize(system_get_polar(body));
}

Polar
system_get_polar(Body *body) {
	Polar polar;

	if (body->polar.r != INFINITY &&
			body->polar.theta != INFINITY)
		return body->polar;

	if (body->type == BODY_COMET) {
		if (!body->parent) {
			polar.r = body->curdist;
			polar.theta = body->theta;
		} else {
			polar = system_sum_polar(system_get_polar(body->parent),
					(Polar){body->curdist, body->theta});
		}
	} else {
		if (!body->parent) {
			polar.r = body->dist;
			polar.theta = body->curtheta;
		} else {
			polar = system_sum_polar(system_get_polar(body->parent),
					(Polar){body->dist, body->curtheta});
		}
	}

	body->polar = polar;
	return polar;
}

System *
system_init(char *name) {
	System *ret = malloc(sizeof(System));
	if (!ret) return NULL;
	ret->name = strdup(name);
	ret->bodies = NULL;
	ret->bodies_len = 0;
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
 * If s is NULL, call system_init(name); */
System *
system_load(System *s, char *name) {
	char *dir, *tmp;
	char **bname;
	char **bparent;
	size_t blen, i;
	int pos;

	if (!s) s = system_init(name);

	dir = smprintf("%s/", s->name);
	s->bodies_len = blen = dblistgroups_f(&bname, save->systems, &filter_bodyinsystem, dir);
	if (!bname) return NULL;
	bparent = malloc(sizeof(char *) * blen);
	s->bodies = malloc(sizeof(Body *) * blen);

	if (!bparent || !s->bodies)
		return NULL;

	/* first pass: init bodies and parents */
	for (i = 0; i < blen; i++) {
		s->bodies[i] = malloc(sizeof(Body));
		s->bodies[i]->name = nstrdup(bname[i] + strlen(dir));
		bparent[i] = dbget(save->systems, bname[i], "parent");

		tmp = dbget(save->systems, bname[i], "type");
		if (streq(tmp, "Star"))
			s->bodies[i]->type = BODY_STAR;
		else if (streq(tmp, "Planet"))
			s->bodies[i]->type = BODY_PLANET;
		else if (streq(tmp, "Dwarf planet"))
			s->bodies[i]->type = BODY_DWARF;
		else if (streq(tmp, "Asteroid"))
			s->bodies[i]->type = BODY_ASTEROID;
		else if (streq(tmp, "Comet"))
			s->bodies[i]->type = BODY_COMET;
		else if (streq(tmp, "Moon"))
			s->bodies[i]->type = BODY_MOON;

		s->bodies[i]->radius = strnum(dbget(save->systems, bname[i], "radius"));
		s->bodies[i]->mass = strnum(dbget(save->systems, bname[i], "mass"));
		s->bodies[i]->orbdays = strnum(dbget(save->systems, bname[i], "orbdays"));
		if (s->bodies[i]->type == BODY_COMET) {
			/* mindist is on opposite side of parent */
			s->bodies[i]->mindist = 0 - strnum(dbget(save->systems, bname[i], "mindist"));
			s->bodies[i]->maxdist = strnum(dbget(save->systems, bname[i], "maxdist"));
			s->bodies[i]->curdist = strnum(dbget(save->systems, bname[i], "curdist"));
			s->bodies[i]->theta = strnum(dbget(save->systems, bname[i], "theta"));
			s->bodies[i]->inward = strnum(dbget(save->systems, bname[i], "inward"));
		} else {
			s->bodies[i]->dist = strnum(dbget(save->systems, bname[i], "dist"));
			s->bodies[i]->curtheta = strnum(dbget(save->systems, bname[i], "curtheta"));
		}

		/* so system_get_polar() knows if it's usable */
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
		system_get_polar(s->bodies[i]); /* Builds the cache for us: this is more
						   efficient as it can cache the parent too */
		s->bodies[i]->vector = system_vectorize(s->bodies[i]->polar);
	}

	for (i = 0; i < blen; i++) {
		free(bparent[i]);
	}
	free(bparent);
	dblistfree(bname, blen);
	free(dir);

	return s;
}
