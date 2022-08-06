#include <stdlib.h>
#include <string.h>
#include "main.h"

Save *
save_init(char *dir) {
	Save *ret;

	if (!dir || !(ret = malloc(sizeof(Save))))
		return NULL;

	dbdeclare(dir);
	ret->db.dir = nstrdup(dir);
	ret->db.races = smprintf("%s/Races", dir);
	ret->db.systems = smprintf("%s/Systems", dir);
	ret->db.fleets = smprintf("%s/Fleets", dir);
	ret->system = NULL;
	return ret;
};

void
save_write(Save *s) {
	if (s->system && s->system->name)
		dbset(save->db.dir, "index", "selsystem", s->system->name);
	dbwrite(s->db.dir);
}
