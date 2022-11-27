#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <sys/stat.h>
#include "main.h"

static void
save_free(void) {
	free(save->db.dir);
	free(save->db.races);
	free(save->db.systems);
	free(save->db.fleets);
	tree_delete_root(&save->systems, sys_tree_free);
	free(save);
}

void
save_read(char *name) {
	char dir[PATH_MAX];
	char *str;
	int i;

	if (save)
		save_free();

	if (!name || !(save = emalloc(sizeof(Save))))
		return;

	snprintf(dir, sizeof(dir), "%s/%s", SAVEDIR, name);

	memset(&save->systems, 0, sizeof(save->systems));

	dbdeclare(dir);
	save->db.dir = nstrdup(dir);
	save->db.races = smprintf("%s/Races", dir);
	save->db.systems = smprintf("%s/Systems", dir);
	save->db.fleets = smprintf("%s/Fleets", dir);
	sys_tree_load();

	if ((str = dbget(save->db.dir, "index", "homesystem")))
		save->homesys = sys_get(str);
	else
		save->homesys = NULL;

	for (i = 0; i < VIEW_LAST; i++)
		if (view_init[i])
			view_init[i]();
	return;
};

int
save_changed(void) {
	if (!save)
		return 0;
	return dbchanges(save->db.dir) ? 1 : 0;
}

void
save_write(void) {
	if (view_main.sys)
		dbset(save->db.dir, "index", "selsystem", view_main.sys->name);
	dbsettree(save->db.systems, &save->systems, sys_tree_setter);
	dbwrite(save->db.dir);
}

int
save_delete(char *name) {
	char dir[PATH_MAX];

	snprintf(dir, sizeof(dir), "%s/%s", SAVEDIR, name);
	return rmdirp(dir);
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

	snprintf(path, sizeof(path), "%s/%s/Systems", SAVEDIR, name);
	if (mkdirp(path) == -1)
		return -1;

	snprintf(path, sizeof(path), "%s/%s/Systems/Sol", SAVEDIR, name);
	if (dirs_write("sol", path) == -1)
		return -1;

	/* Ideally this data should be set, so that it is in memory, not on
	 * disk, but for now, write->read->delete->maybe save */
	save_read(name);
	save_delete(name);
	dbset(save->db.dir, "index", "homesystem", "Sol");

	return 0;
}
