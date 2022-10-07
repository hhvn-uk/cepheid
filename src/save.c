#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <sys/stat.h>
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
save_read(char *name) {
	char dir[PATH_MAX];
	char *str;

	if (save)
		save_free();

	if (!name || !(save = malloc(sizeof(Save))))
		return;

	snprintf(dir, sizeof(dir), "%s/%s", SAVEDIR, name);

	memset(&save->systems, 0, sizeof(save->systems));

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
	dbsettree(save->db.systems, &save->systems, sys_tree_setter);
	dbwrite(save->db.dir);
}

int
save_exists(char *name) {
	char dir[PATH_MAX];

	snprintf(dir, sizeof(dir), "%s/%s", SAVEDIR, name);
	if (access(dir, F_OK) == 0)
		return 1;
	return 0;
}

int
save_create(char *name) {
	char path[PATH_MAX];
	FILE *f;

	snprintf(path, sizeof(path), "%s/%s", SAVEDIR, name);
	if (mkdir(path, 0755) == -1)
		return -1;

	snprintf(path, sizeof(path), "%s/%s/Systems", SAVEDIR, name);
	if (mkdir(path, 0755) == -1)
		return -1;

	snprintf(path, sizeof(path), "%s/%s/Systems/Sol", SAVEDIR, name);
	if (wdir_sol(path) == -1)
		return -1;

	snprintf(path, sizeof(path), "%s/%s/index", SAVEDIR, name);
	if (!(f = fopen(path, "w")))
		return -1;
	fprintf(f, "selsystem\tSol\n");
	fclose(f);
	return 0;
}
