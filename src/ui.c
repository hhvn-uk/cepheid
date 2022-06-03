#include <raylib.h>
#include "main.h"

static Font font;
static Clickable clickable[CLICKABLE_MAX];

void (*view_drawers[UI_VIEW_LAST])(void) = {
	[UI_VIEW_MAIN] = ui_draw_view_main,
	[UI_VIEW_COLONIES] = ui_draw_view_colonies,
	[UI_VIEW_FLEETS] = ui_draw_view_fleets,
	[UI_VIEW_DESIGN] = ui_draw_view_design,
	[UI_VIEW_SYSTEMS] = ui_draw_view_systems,
	[UI_VIEW_SETTINGS] = ui_draw_view_settings,
};

Tabs view_tabs = {
	UI_VIEW_LAST, 0, {{"Main", 0}, {"Colonies", 0}, {"Fleets", 0}, {"Design", 0},
				{"Systems", 0}, {"Settings", 0}}
};

void
ui_init(void) {
	InitWindow(500, 500, "testing raylib");
	SetWindowState(FLAG_WINDOW_RESIZABLE);
	SetTargetFPS(30);

	font = LoadFontFromMemory(".ttf",
			DejaVuSansMono_ttf, DejaVuSansMono_ttf_size,
			FONT_SIZE, NULL, 0);
}

void
ui_deinit(void) {
	CloseWindow();
}

void
ui_print(int x, int y, Color col, char *fmt, ...) {
	va_list ap;
	Vector2 pos;

	pos.x = x;
	pos.y = y;
	va_start(ap, fmt);
	DrawTextEx(font, vsbprintf(fmt, ap), pos, (float)FONT_SIZE, FONT_SIZE/10, col);
	va_end(ap);
}

int
ui_textsize(char *text) {
	return MeasureTextEx(font, text, FONT_SIZE, FONT_SIZE/10).x;
}

void
ui_clickable_register(int x, int y, int w, int h, enum UiElements type, void *elem) {
	int i;

	for (i = 0; i < CLICKABLE_MAX; i++) {
		if (!clickable[i].elem) {
			clickable[i].x = x;
			clickable[i].y = y;
			clickable[i].w = w;
			clickable[i].h = h;
			clickable[i].type = type;
			clickable[i].elem = elem;
			return;
		}
	}

	/* welp, we ran out */
}

void
ui_clickable_handle(Vector2 mouse, MouseButton button, Clickable *clickable) {
	Tabs *tabs;
	int ftabw, fw, fn, tabw, x;
	int i;

	switch (clickable->type) {
	case UI_TAB:
		if (button != MOUSE_BUTTON_LEFT)
			return;
		tabs = clickable->elem;
		for (fw = clickable->w, fn = i = 0; i < tabs->n; i++) {
			if (!tabs->tabs[i].w)
				fn++;
			else
				fw -= tabs->tabs[i].w;
		}
		ftabw = fw / fn;
		for (i = 0, x = clickable->x; i < tabs->n; x += tabw, i++) {
			if (i == tabs->n - 1)
				tabw = clickable->x + clickable->w - x;
			else if (!tabs->tabs[i].w)
				tabw = ftabw;
			else
				tabw = tabs->tabs[i].w;
			if (mouse.x >= x && mouse.x <= x + tabw) {
				tabs->sel = i;
				return;
			}
		}
		break;
	}
}

void
ui_clickable_update(void) {
	Vector2 mouse;
	MouseButton button;
	int i;

	mouse = GetMousePosition();
	/* I wish there was a: int GetMouseButton(void) */
	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) button = MOUSE_BUTTON_LEFT;
	else if (IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE)) button = MOUSE_BUTTON_MIDDLE;
	else if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) button = MOUSE_BUTTON_RIGHT;
	else button = -1;

	for (i = 0; i < CLICKABLE_MAX; i++) {
		if (clickable[i].elem &&
				mouse.x >= clickable[i].x &&
				mouse.x <= clickable[i].x + clickable[i].w &&
				mouse.y >= clickable[i].y &&
				mouse.y <= clickable[i].y + clickable[i].h)
			ui_clickable_handle(mouse, button, &clickable[i]);
		clickable[i].elem = NULL;
	}
}

void
ui_draw_views(void) {
	int sw = GetScreenWidth();
	if (sw > VIEWS_MAX_WIDTH) sw = VIEWS_MAX_WIDTH;
	ui_draw_tabs(&view_tabs, 0, 0, sw, VIEWS_HEIGHT);
}

void
ui_draw_border(int x, int y, int w, int h, int px) {
	DrawRectangle(x, y, w, px, COL_BORDER); /* top */
	DrawRectangle(x, y, px, h, COL_BORDER); /* left */
	DrawRectangle(x, y + h - px, w, px, COL_BORDER); /* bottom */
	DrawRectangle(x + w - px, y, px, h, COL_BORDER); /* right */
}

void
ui_draw_tabs(Tabs *tabs, int x, int y, int w, int h) {
	int fw, fn, ftabw;
	int tabw;
	int padx, pady;
	int cx, selx = -1;
	int i;

	for (fw = w, fn = i = 0; i < tabs->n; i++) {
		if (!tabs->tabs[i].w)
			fn++;
		else
			fw -= tabs->tabs[i].w;
	}

	ftabw = fw / fn;
	pady = (h - FONT_SIZE) / 2;

	for (i = 0, cx = x; i < tabs->n; i++, cx += tabw) {
		if (i == tabs->n - 1)
			tabw = x + w - cx;
		else if (!tabs->tabs[i].w)
			tabw = ftabw;
		else
			tabw = tabs->tabs[i].w;
		padx = (tabw - ui_textsize(tabs->tabs[i].name)) / 2;
		if (i == tabs->sel)
			selx = cx;
		else
			DrawRectangle(cx, y, tabw, h, COL_UNSELBG);
		ui_print(cx + padx, y + pady, COL_FG, "%s", tabs->tabs[i].name);
		DrawRectangle(cx + tabw - 1, y, 1, h, COL_BORDER);
	}

	if (tabs->sel != tabs->n - 1) {
		if (!tabs->tabs[i].w)
			tabw = ftabw;
		else
			tabw = w / tabs->n;
	}

	ui_draw_border(x, y, w, h, 1);
	if (selx != -1) DrawRectangle(selx - 1, y + h - 1, tabw + 1, 1, COL_BG); /* undraw bottom border */
	if (tabs->sel == 0) DrawRectangle(x, y + 1, 1, h - 1, COL_BG); /* undraw left border */
	if (tabs->sel == tabs->n - 1) DrawRectangle(x + w - 1, y + 1, 1, h - 1, COL_BG); /* undraw right border */

	ui_clickable_register(x, y, w, h, UI_TAB, tabs);
}

void
ui_draw_tabbed_window(Tabs *tabs, int x, int y, int w, int h) {
	ui_draw_tabs(tabs, x, y, w, WINDOW_TAB_HEIGHT);
	ui_draw_border(x, y, w, h, 2);
}

void
ui_draw_view_main(void) {
	static Tabs window_tabs = {
		2, 0, {{"Display", 0}, {"Minerals", 0}}
	};
	ui_draw_tabbed_window(&window_tabs, 10, VIEWS_HEIGHT + 10, 200, 400);
	ui_print(GetScreenWidth() / 2, GetScreenHeight() / 2, COL_FG, "Pannable body view here");
}

void
ui_draw_view_colonies(void) {
	ui_print(10, VIEWS_HEIGHT + 10, COL_FG, "Stars/colonies here");
	ui_print(GetScreenWidth() / 2, VIEWS_HEIGHT + 10, COL_FG, "Tabs here");
	ui_print(GetScreenWidth() / 2, GetScreenHeight() / 2, COL_FG, "Management stuff here");
}

void
ui_draw_view_fleets(void) {
	ui_print(10, VIEWS_HEIGHT + 10, COL_FG, "Groups/fleets/subfleets/ships here");
	ui_print(GetScreenWidth() / 2, VIEWS_HEIGHT + 10, COL_FG, "Tabs here");
	ui_print(GetScreenWidth() / 2, GetScreenHeight() / 2, COL_FG, "Management stuff here");
}

void
ui_draw_view_design(void) {
	ui_print(10, VIEWS_HEIGHT + 10, COL_FG, "Designations/classes here");
	ui_print(GetScreenWidth() / 4, VIEWS_HEIGHT + 10, COL_FG, "Selectable components here");
	ui_print((GetScreenWidth() / 4) * 2, VIEWS_HEIGHT + 10, COL_FG, "Selected components");
	ui_print((GetScreenWidth() / 4) * 3, VIEWS_HEIGHT + 10, COL_FG, "Class info");
}

void
ui_draw_view_systems(void) {
	ui_print(10, GetScreenHeight() / 2, COL_FG, "System info/settings menu");
	ui_print(GetScreenWidth() / 2, GetScreenHeight() / 2, COL_FG, "Pannable system view");
}

void
ui_draw_view_settings(void) {
	ui_print(10, VIEWS_HEIGHT + 10, COL_FG, "Settings here");
}
