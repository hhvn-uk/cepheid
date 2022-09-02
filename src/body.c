#include <stdlib.h>
#include <raylib.h>
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

int
body_cmp(Body *b1, Body *b2) {
	float v1, v2;
	Body *p;

	for (p = b1, v1 = 0; p->parent; p = p->parent)
		v1 += (p->type == BODY_COMET ? p->maxdist : p->dist);
	for (p = b2, v2 = 0; p->parent; p = p->parent)
		v2 += (p->type == BODY_COMET ? p->maxdist : p->dist);
	return (v1 == v2 ? 0 : (v1 > v2 ? 1 : -1));
}

static int
body_cmp_sort(const void *a, const void *b) {
	return body_cmp(*(const Body **)a, *(const Body **)b);
}

void
body_sort(Body **bodies, size_t n) {
	qsort(bodies, n, sizeof(Body *), body_cmp_sort);
}