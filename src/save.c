#include <stdlib.h>
#include <string.h>
#include "main.h"

/* Plan for dealing with multiple saves:
 * - saves: linked list with all saves that have been loaded (some data pruned)
 * - save: the current save
 * - save_get(name): get a save from saves/disk by name and load necessary data
 * - save_select(name): set save to save_get(name) + housekeeping
 */

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
save_read(Loader *lscr, char *dir) {
	char *str;

	if (save)
		save_free();

	if (!dir || !(save = malloc(sizeof(Save))))
		return;

	loading_update(lscr, "Initializing DB");
	dbdeclare(dir);
	save->db.dir = nstrdup(dir);
	save->db.races = smprintf("%s/Races", dir);
	save->db.systems = smprintf("%s/Systems", dir);
	save->db.fleets = smprintf("%s/Fleets", dir);
	loading_update(lscr, "Loading systems");
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
