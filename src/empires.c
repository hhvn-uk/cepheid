#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"

void
emp_free(Empire *e) {
	free(e->name);
	free(e);
}

Empire *
emp_byid(Save *s, char *id) {
	int i;

	for (i = EMP_FIRST; i < EMP_MAX; i++)
		if (s->emp[i] && streq(s->emp[i]->id, id))
			return s->emp[i];
	return NULL;
}

Empire *
emp_byname(Save *s, char *name) {
	int i;

	for (i = EMP_FIRST; i < EMP_MAX; i++)
		if (s->emp[i] && streq(s->emp[i]->name, name))
			return s->emp[i];
	return NULL;
}

static int
emp_add(Save *s, Empire *e) {
	int i;

	for (i = EMP_FIRST; i < EMP_MAX; i++){
		if (!s->emp[i]) {
			s->emp[i] = e;
			return i;
		}
	}

	return -1;
}

Empire *
emp_init(Save *s, int player, char *id, char *name) {
	Empire *e = emalloc(sizeof(Empire));

	e->player = 0;
	snprintf(e->id, sizeof(e->id), "%s", id);
	e->name = estrdup(name);

	if (s->emp[EMP_PLAYER] && player)
		warning("player already exists in this save");
	else
		e->player = player;

	if (e->player) {
		e->db = smprintf("%s/player", s->db.emp);
		e->eid = EMP_PLAYER;
		s->emp[e->eid] = e;
	} else {
		e->db = smprintf("%s/%s", s->db.emp, id);
		e->eid = emp_add(s, e);
	}

	return e;
}

void
emp_gen(Save *s, int n) {
	char name[9];
	char id[4];
	int i;

	for (i = 1; i <= n; i++) {
		/* TODO: take names & ids from list */
		snprintf(name, sizeof(name), "Empire %d", i);
		snprintf(id, 4, "EM%d", i);
		emp_init(s, 0, id, name);
	}
}

void
emp_write(Save *s) {
	char *names[EMP_MAX];
	char *ids[EMP_MAX];
	int i;

	for (i = 0; s->emp[i]; i++) {
		names[i] = s->emp[i]->name;
		ids[i] = s->emp[i]->id;
	}

	bdbset(s->db.emp, "index",
			'S', "names", names, i,
			'S', "ids", ids, i);
}

void
emp_read(Save *s) {
	char *names[EMP_MAX];
	char *ids[EMP_MAX];
	int i;

	bdbget(s->db.emp, "index",
			'S', "names", &names, EMP_MAX,
			'S', "ids", &ids, EMP_MAX);

	for (i = 0; i < EMP_MAX && names[i]; i++)
		emp_init(s, !i, ids[i], names[i]);
}
