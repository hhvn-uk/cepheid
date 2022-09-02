#include <stdio.h>
#include <time.h>
#include <raylib.h>
#include "main.h"

#define DEFSAVE	"default"

Save *save = NULL;

int
main(void) {
	int view_prev;
	Loader *loader;

	loader = loading_open(DATA_LOAD_STEPS + SAVE_READ_STEPS + 3, "Initializing UI");

	ui_init();

	data_load(loader);

	if (!save_exists(DEFSAVE))
		save_create(DEFSAVE);
	save_read(loader, DEFSAVE);

	loading_close(loader);

	/* The window is hidden so that only the loading bar is shown. Hiding
	 * and unhiding the window also has the added effect of making it
	 * behave like a normal window in window managers, rather than having
	 * it float in the middle of the screen. */
	ClearWindowState(FLAG_WINDOW_HIDDEN);
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

		view_prev = view_tabs.sel;

		ui_clickable_update();
		view_handlers[view_tabs.sel](view_prev != view_tabs.sel);

		BeginDrawing();
		ClearBackground(col_bg);
		ui_clickable_clear();
		view_drawers[view_tabs.sel]();
		ui_draw_views();
		EndDrawing();
	}

	data_unload();
	ui_deinit();
	save_write();
	dbcleanup();
	return 0;
}
