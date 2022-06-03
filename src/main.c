#include <stdio.h>
#include <raylib.h>
#include "main.h"

#define TESTSAVE "testdb"

Save *save = NULL;

int
main(void) {
	int w, h;
	ui_init();
	Tabs test = {
		2, 0, {{"Display", 0}, {"Minerals", 0}}
	};

	save = save_init(TESTSAVE);

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

		BeginDrawing();
		ClearBackground(COL_BG);
		ui_draw_views();
		view_drawers[view_tabs.sel]();
		EndDrawing();
	}

	ui_deinit();
	return 0;
}
