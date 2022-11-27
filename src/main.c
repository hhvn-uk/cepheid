#include <stdio.h>
#include <time.h>
#include <raylib.h>
#include <signal.h>
#include <stdlib.h>
#include "main.h"

#define DEFSAVE	"default"

#ifdef TEST
int test_run(void);
#endif /* TEST */

Save *save = NULL;
int sigint = 0;
int sigterm = 0;
int quit = 0;
int view_before_smenu = VIEW_MAIN;

static void
sighandler(int signal) {
	switch (signal) {
	case SIGINT: sigint = 1; break;
	case SIGTERM: sigterm = 1; break;
	}
}

int
main(void) {
	int view_prev;
	struct sigaction sa;


	sa.sa_handler = sighandler;
	sa.sa_flags = SA_RESTART;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);

#ifdef TEST
	return test_run();
#endif /* TEST */

	ui_init();

	data_load();

	/* The window is hidden so that only the loading bar is shown. Hiding
	 * and unhiding the window also has the added effect of making it
	 * behave like a normal window in window managers, rather than having
	 * it float in the middle of the screen. */
	ClearWindowState(FLAG_WINDOW_HIDDEN);
	ui_update_screen();

	while (ui_loop()) {
		if (IsKeyPressed(KEY_ESCAPE))
			view_tabs.sel = VIEW_SMENU;

		if (save && (IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT))) {
			/* AAAAAAAAAAHHHHHHHHHHHH. WHY NOT JUST USE KEY_1, KEY_2..! */
			if (IsKeyPressed(KEY_ONE)) view_tabs.sel = 1;
			else if (IsKeyPressed(KEY_TWO)) view_tabs.sel = 2;
			else if (IsKeyPressed(KEY_THREE)) view_tabs.sel = 3;
			else if (IsKeyPressed(KEY_FOUR)) view_tabs.sel = 4;
			else if (IsKeyPressed(KEY_FIVE)) view_tabs.sel = 5;
			else if (IsKeyPressed(KEY_SIX)) view_tabs.sel = 6;
		}

		view_handle[view_tabs.sel](view_prev != view_tabs.sel);

		BeginDrawing();
		ClearBackground(col_bg);
		view_draw[view_tabs.sel]();
		if (view_tabs.sel != VIEW_SMENU) {
			ui_draw_views();
			view_before_smenu = view_tabs.sel;
		}
		EndDrawing();

		view_prev = view_tabs.sel;
	}

	data_unload();
	ui_deinit();
	dbcleanup();
	return 0;
}
