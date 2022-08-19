#include <stdlib.h>
#include <string.h>
#include "main.h"

static void
save_free(void) {
	free(save->db.dir);
	free(save->db.races);
	free(save->db.systems);
	free(save->db.fleets);
	/* free systems? */
	free(save);
}

void
save_read(char *dir) {
	char *str;

	if (save)
		save_free();

	if (!dir || !(save = malloc(sizeof(Save))))
		return;

	dbdeclare(dir);
	save->db.dir = nstrdup(dir);
	save->db.races = smprintf("%s/Races", dir);
	save->db.systems = smprintf("%s/Systems", dir);
	save->db.fleets = smprintf("%s/Fleets", dir);
	if ((str = dbget(save->db.dir, "index", "homesystem")))
		save->homesys = sys_get(str);
	return;
};

void
save_write(void) {
	if (view_main.sys)
		dbset(save->db.dir, "index", "selsystem", view_main.sys->name);
	dbwrite(save->db.dir);
}
