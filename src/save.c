#include <stdlib.h>
#include <string.h>
#include "main.h"

Save *
save_init(char *dir) {
	Save *ret;

	if (!dir || !(ret = malloc(sizeof(Save))))
		return NULL;

	ret->dir = strdup(dir);
	ret->races = smprintf("%s/Races", dir);
	ret->systems = smprintf("%s/Systems", dir);
	ret->fleets = smprintf("%s/Fleets", dir);
	return ret;
};
