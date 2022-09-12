#include <stdio.h>
#include <stdlib.h>
#include "main.h"

int
vdbsetf(char *dir, char *group, char *key, char *fmt, va_list args) {
	va_list ap;
	char *str;
	int ret;

	va_copy(ap, args);
	str = vsmprintf(fmt, ap);
	va_end(ap);
	ret = dbset(dir, group, key, str);
	free(str);
	return ret;
}

int
dbsetf(char *dir, char *group, char *key, char *fmt, ...) {
	va_list ap;
	int ret;

	va_start(ap, fmt);
	ret = vdbsetf(dir, group, key, fmt, ap);
	va_end(ap);
	return ret;
}

int
dbsetint(char *dir, char *group, char *key, int val) {
	return dbsetf(dir, group, key, "%d", val);
}

int
dbsetfloat(char *dir, char *group, char *key, float val) {
	return dbsetf(dir, group, key, "%f", val);
}

void
dbsetbody(System *sys, Body *body) {
	char *group;
	char *parent;

	group = smprintf("%s/%s", sys->name, body->name);

	if (body->parent)
		parent = body->parent->name;
	else
		parent = sys->name;
	dbset(save->db.systems, group, "parent", parent);

	dbset(save->db.systems, group, "type", bodytype_strify(body));
	dbsetfloat(save->db.systems, group, "radius", body->radius);
	dbsetfloat(save->db.systems, group, "mass", body->mass);
	dbsetfloat(save->db.systems, group, "orbdays", body->orbdays);
	if (body->type == BODY_COMET) {
		dbsetfloat(save->db.systems, group, "mindist", 0 - body->mindist); /* see sys_load() */
		dbsetfloat(save->db.systems, group, "maxdist", body->maxdist);
		dbsetfloat(save->db.systems, group, "curdist", body->curdist);
		dbsetfloat(save->db.systems, group, "theta", body->theta);
		dbsetfloat(save->db.systems, group, "inward", body->inward);
	} else {
		dbsetfloat(save->db.systems, group, "dist", body->dist);
		dbsetfloat(save->db.systems, group, "curtheta", body->curtheta);
	}
	free(group);
}

int
vdbgetf(char *dir, char *group, char *key, char *fmt, va_list args) {
	va_list ap;
	char *str;
	int ret;

	str = dbget(dir, group, key);
	if (!str)
		return EOF;
	va_copy(ap, args);
	ret = vsscanf(str, fmt, ap);
	va_end(ap);
	free(str);
	return ret;
}

int
dbgetf(char *dir, char *group, char *key, char *fmt, ...) {
	va_list ap;
	int ret;

	va_start(ap, fmt);
	ret = vdbgetf(dir, group, key, fmt, ap);
	va_end(ap);
	return ret;
}

int
dbgetint(char *dir, char *group, char *key) {
	int ret;

	dbgetf(dir, group, key, "%d", &ret);
	return ret;
}

float
dbgetfloat(char *dir, char *group, char *key) {
	float ret;

	dbgetf(dir, group, key, "%f", &ret);
	return ret;
}
