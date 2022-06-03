#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "raylib.h"
#include "main.h"

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
	return system_polarize(
			system_vectorize_around(system_vectorize(absolute), relative));
}

Vector2
system_get_vector(char *system, char *body) {
	return system_vectorize(system_get_polar(system, body));
}

Polar
system_get_polar(char *system, char *body) {
	Polar polar;
	char *group, *dist, *parent;

	group = sbprintf("%s/%s", system, body);
	if (strcmp(dbget(save->systems, group, "type"), "Comet") == 0)
		dist = dbget(save->systems, group, "curdist");
	else
		dist = dbget(save->systems, group, "dist");

	parent = dbget(save->systems, group, "parent");
	polar.r = strtof(dist, NULL);
	polar.theta = strtof(dbget(save->systems, group, "curtheta"), NULL);
	if (!parent)
		return (Polar) {polar.r, polar.theta};
	else
		return system_sum_polar(system_get_polar(system, parent), polar);
}
