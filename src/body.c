#include <stdlib.h>
#include <raylib.h>
#include <math.h>
#include "main.h"

static char *bodytype_names[BODY_LAST] = {
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

int
bodytype_enumify(char *name) {
	if (strsuffix(name, "moon") || streq(name, "Moon"))
		return BODY_MOON;
	else if (streq(name, "Star"))
		return BODY_STAR;
	else if (streq(name, "Planet"))
		return BODY_PLANET;
	else if (streq(name, "Dwarf planet"))
		return BODY_DWARF;
	else if (streq(name, "Asteroid"))
		return BODY_ASTEROID;
	else if (streq(name, "Comet"))
		return BODY_COMET;
	else
		return -1;
}

char *
bodytype_strify(Body *body) {
	return bodytype_names[body->type +
		(body->parent ? body->parent->type : 0)];
}

Body *
body_init(char *name) {
	Body *ret;

	ret = malloc(sizeof(Body));
	if (!ret) return NULL;
	ret->name = nstrdup(name);
	ret->t = NULL;
	ret->parent = NULL;
	ret->polar = (Polar) { INFINITY, INFINITY };

	return ret;
}
