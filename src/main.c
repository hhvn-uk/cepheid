#include <stdio.h>
#include <raylib.h>
#include "main.h"

#define TESTSAVE "testdb"

Save *save = NULL;

int
main(void) {
	char *system;

	ui_init();
	data_load();

	save = save_init(TESTSAVE);
	system = dbget(save->db.dir, "index", "selsystem");
	if (!system) system = "Sol"; /* TODO: 1. selsystem, 2. player home, 3. uhh? */
	save->system = sys_load(NULL, system);

	while (!WindowShouldClose()) {
		ui_clickable_update();

		if (IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT)) {
			/* AAAAAAAAAAHHHHHHHHHHHH. WHY NOT JUST USE KEY_1, KEY_2..! */
			if (IsKeyPressed(KEY_ONE)) view_tabs.sel = 0;
			else if (IsKeyPressed(KEY_TWO)) view_tabs.sel = 1;
			else if (IsKeyPressed(KEY_THREE)) view_tabs.sel = 2;
			else if (IsKeyPressed(KEY_FOUR)) view_tabs.sel = 3;
			else if (IsKeyPressed(KEY_FIVE)) view_tabs.sel = 4;
			else if (IsKeyPressed(KEY_SIX)) view_tabs.sel = 5;
		}

		view_handlers[view_tabs.sel]();

		BeginDrawing();
		ClearBackground(COL_BG);
		view_drawers[view_tabs.sel]();
		ui_draw_views();
		EndDrawing();
	}

	ui_deinit();

	save_write(save);
	dbcleanup();
	return 0;
}
