#include <stdio.h>
#include <time.h>
#include <raylib.h>
#include "main.h"

#define TESTSAVE "testdb"

Save *save = NULL;

int
main(void) {
	int draw;
	int view_prev = -1;
	time_t lastevent;

	ui_init();
	data_load();
	save_read(TESTSAVE);

	lastevent = time(NULL);

	ui_update_screen();

	while (!WindowShouldClose()) {
		ffree();

		if (IsWindowResized())
			ui_update_screen();

		if (IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT)) {
			/* AAAAAAAAAAHHHHHHHHHHHH. WHY NOT JUST USE KEY_1, KEY_2..! */
			if (IsKeyPressed(KEY_ONE)) view_tabs.sel = 0;
			else if (IsKeyPressed(KEY_TWO)) view_tabs.sel = 1;
			else if (IsKeyPressed(KEY_THREE)) view_tabs.sel = 2;
			else if (IsKeyPressed(KEY_FOUR)) view_tabs.sel = 3;
			else if (IsKeyPressed(KEY_FIVE)) view_tabs.sel = 4;
			else if (IsKeyPressed(KEY_SIX)) view_tabs.sel = 5;
		}

		draw = view_handlers[view_tabs.sel](view_prev != view_tabs.sel);

		if (lastevent == time(NULL))
			draw = 1;

		view_prev = view_tabs.sel;

		if (view_prev != view_tabs.sel ||
				GetMouseWheelMove() ||
				IsWindowResized() ||
				ui_clickable_update() ||
				IsMouseButtonDown(MOUSE_BUTTON_LEFT) ||
				IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
			lastevent = time(NULL);

		BeginDrawing();
		if (draw) {
			ClearBackground(COL_BG);
			ui_clickable_clear();
			view_drawers[view_tabs.sel]();
		}
		ui_draw_views();
		EndDrawing();
	}

	data_unload();
	ui_deinit();
	save_write();
	dbcleanup();
	return 0;
}
