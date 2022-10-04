#include <math.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <raylib.h>
#include <wchar.h>
#include "main.h"

static Clickable clickable[CLICKABLE_MAX];

/* Return 1 for redraw, 0 to keep prev */
void (*view_handlers[UI_VIEW_LAST])(int) = {
	[UI_VIEW_MAIN] = ui_handle_view_main,
	[UI_VIEW_COLONIES] = ui_handle_view_colonies,
	[UI_VIEW_BODIES] = ui_handle_view_bodies,
	[UI_VIEW_FLEETS] = ui_handle_view_fleets,
	[UI_VIEW_DESIGN] = ui_handle_view_design,
	[UI_VIEW_SYSTEMS] = ui_handle_view_sys,
	[UI_VIEW_SETTINGS] = ui_handle_view_settings,
};

void (*view_drawers[UI_VIEW_LAST])(void) = {
	[UI_VIEW_MAIN] = ui_draw_view_main,
	[UI_VIEW_COLONIES] = ui_draw_view_colonies,
	[UI_VIEW_BODIES] = ui_draw_view_bodies,
	[UI_VIEW_FLEETS] = ui_draw_view_fleets,
	[UI_VIEW_DESIGN] = ui_draw_view_design,
	[UI_VIEW_SYSTEMS] = ui_draw_view_sys,
	[UI_VIEW_SETTINGS] = ui_draw_view_settings,
};

Screen screen = { 0 };
Focus focus = { 0 };

Tabs view_tabs = {
	/* Tactical is the terminology used in Aurora, so I decided to use it
	 * in the ui; in the code it's just called 'main' for my fingers' sake */
	UI_VIEW_LAST, 0, {{&image_tactical, "Tactical", 0},
		{&image_colonies, "Colonies", 0},
		{&image_bodies, "Bodies", 0},
		{&image_fleet, "Fleets", 0},
		{&image_design, "Design", 0},
		{&image_sys, "Systems", 0},
		{&image_settings, "Settings", 0}}
};

View_sys view_sys = {
	.info = {
		.geom = {
			.x = 0,
			.y = VIEWS_HEIGHT,
			.w = 300,
			.h = 0, /* see ui_handle_view_sys() */
		},
	},
	.pan = 0,
	.off = {
		.x = 0,
		.y = 0,
	},
	.lytopx = 0.025,
	.sel = NULL,
};

int charpx; /* thank god for monospaced fonts */

void
ui_init(void) {
	SetWindowState(FLAG_WINDOW_RESIZABLE|FLAG_WINDOW_HIDDEN);
	SetTargetFPS(TARGET_FPS);
	SetExitKey(KEY_NULL);
	InitWindow(500, 500, "");
}

void
ui_update_screen(void) {
	screen.w = screen.rect.w = GetScreenWidth();
	screen.h = screen.rect.h = GetScreenHeight();
	screen.centre.x = screen.w / 2;
	screen.centre.y = screen.h / 2;
	screen.diag = sqrt(SQUARE(screen.w) + SQUARE(screen.h));
}

void
ui_update_focus(enum UiElements type, void *p) {
	focus.type = type;
	focus.p = p;
}

int
ui_loop(void) {
	if (WindowShouldClose())
		return 0;

	ffree();
	if (IsWindowResized())
		ui_update_screen();
	ui_keyboard_handle();
	ui_clickable_update();
	return 1;
}

void
ui_deinit(void) {
	CloseWindow();
}

void
ui_print(int x, int y, Color col, char *fmt, ...) {
	va_list ap;
	Vector2 pos;
	char *text;

	if (!pane_visible(y, y + FONT_SIZE))
		return;

	pos.x = x;
	pos.y = pane_y(y);
	va_start(ap, fmt);
	text = vsmprintf(fmt, ap);
	va_end(ap);
	DrawTextEx(font, text, pos, (float)FONT_SIZE, FONT_SIZE/10, col);
	free(text);
}

void
ui_title(char *fmt, ...) {
	char *title;
	va_list ap;

	va_start(ap, fmt);
	title = vsmprintf(fmt, ap);
	va_end(ap);
	SetWindowTitle(title);
	free(title);
}

int
ui_textsize(char *text) {
	return charpx * strlen(text);
}

float
ui_get_scroll(void) {
	float ret = GetMouseWheelMove();
	if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))
		ret *= 2;
	return ret;
}

int
ui_collides(Geom geom, Vector2 point) {
	switch (geom.type) {
	case UI_CIRCLE:
		return CheckCollisionPointCircle(point,
				EXPLODE_CIRCLEV(geom));
	case UI_RECT: default: /* -Wreturn-type bitches without default */
		return CheckCollisionPointRec(point, RLIFY_RECT(geom));
	}
}

int
ui_onscreen(Vector2 point) {
	if (!pane_visible(point.y, point.y))
		return 0;
	point.y = pane_y(point.y);
	return ui_collides(screen.rect, point);
}

int
ui_onscreen_ring(Vector2 centre, float r) {
	float d = ui_vectordist(centre, screen.centre);

	if (!pane_visible(centre.y - r, centre.y + r))
		return 0;
	centre.y = pane_y(centre.y);

	if (d + screen.diag / 2 < r)
		return 0;
	return CheckCollisionCircleRec(centre, r, RLIFY_RECT(screen.rect));
}

int
ui_onscreen_circle(Vector2 centre, float r) {
	if (!pane_visible(centre.y - r, centre.y + r))
		return 0;
	centre.y = pane_y(centre.y);
	return CheckCollisionCircleRec(centre, r, RLIFY_RECT(screen.rect));
}

void
ui_clickable_register(Geom geom, enum UiElements type, void *elem) {
	int i;

	for (i = 0; i < CLICKABLE_MAX; i++) {
		if (!clickable[i].elem) {
			clickable[i].geom = geom;
			clickable[i].type = type;
			clickable[i].elem = elem;
			return;
		}
	}

	/* welp, we ran out */
}

int
ui_clickable_update(void) {
	Vector2 mouse;
	MouseButton button;
	Geom *geom;
	int i;
	int ret = 0;
	int keepfocus = 0;

	mouse = GetMousePosition();
	/* I wish there was a: int GetMouseButton(void) */
	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
		button = MOUSE_BUTTON_LEFT;
	else if (IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE))
		button = MOUSE_BUTTON_MIDDLE;
	else if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
		button = MOUSE_BUTTON_RIGHT;
	else
		button = -1;

	for (i = 0; i < CLICKABLE_MAX; i++) {
		if (clickable[i].elem && ui_collides(clickable[i].geom, mouse)) {
			geom = &clickable[i].geom;
			ui_elem_handlers[clickable[i].type](mouse,
					button, geom, clickable[i].elem);
			if (clickable[i].elem == focus.p)
				keepfocus = 1;
			ret = 1;
		}
	}

	/* clicking outside the focused elememnt unfocuses */
	if (button != -1 && !keepfocus)
		ui_update_focus(0, NULL);

	/* Handle bodies seperately for efficiency:
	 * - body->pxloc can be used instead of a geometry passed to
	 *   ui_clickable_register()
	 * - bodies offscreen can't be clicked, so they can be skipped.
	 */

	return ret;
}

static int
ui_keyboard_check(int key, int *fcount) {
	if (IsKeyPressed(key)) {
		*fcount = -10;
		return 1;
	} else if (IsKeyDown(key) && !*fcount) {
		return 1;
	} else {
		return 0;
	}
}

void
ui_keyboard_handle(void) {
	static int fcount = 0;
	wchar_t c;
	Input *in;

	/* Register multiple backspaces when held. raylib does this with
	 * "characters", but anything other than a "character" has completely
	 * different handling. (Newlines and backspaces *ARE* ASCII characters.
	 * So... eugh). ncurses's get_blah23_ch_fgfgj is somehow better.
	 *
	 * This also IS NOT ABLE TO respect the X11 settings for outputting
	 * characters while held because... Fuck you, apparently. Thanks
	 * raylib. */
	fcount++;
	if (fcount == (int)TARGET_FPS/15)
		fcount = 0;

	if (focus.p && focus.type == UI_INPUT) {
		in = focus.p;
		c = GetCharPressed();

		if (IsKeyPressed(KEY_ENTER) && in->onenter) {
			wcstombs(in->str, in->wstr, INPUT_MAX);
			if (in->onenter(in))
				edittrunc(in->wstr, &in->len, &in->cur);
		} else if (ui_keyboard_check(KEY_BACKSPACE, &fcount) && in->len && in->cur) {
			editrm(in->wstr, &in->len, &in->cur);
		} else if (ui_keyboard_check(KEY_LEFT, &fcount) && in->cur) {
			in->cur--;
		} else if (ui_keyboard_check(KEY_RIGHT, &fcount) && in->cur != in->len) {
			in->cur++;
		} else if (c && in->len < INPUT_MAX) {
			editins(in->wstr, &in->len, &in->cur, INPUT_MAX, c);
		}

	}
}

void
ui_clickable_clear(void) {
	int i;
	for (i = 0; i < CLICKABLE_MAX; i++)
		clickable[i].elem = NULL;
}

void
ui_draw_views(void) {
	int sw = GetScreenWidth();
	if (sw > VIEWS_MAX_WIDTH) sw = VIEWS_MAX_WIDTH;
	ui_tabs(0, 0, sw, VIEWS_HEIGHT, &view_tabs);
}

void
ui_draw_rectangle(int x, int y, int w, int h, Color col) {
	if (pane_visible(y, y + h))
		DrawRectangle(x, pane_y(y), w, h, col);
}


#define SEGMAX 1500

void
ui_draw_ring(int x, int y, float r, Color col) {
	Vector2 v = {x, y};
	Polar p;
	float s;
	float prec = screen.diag * 1.5 / (PI * 2 * r) * 360;
	float sdeg = 0, edeg = 360;
	float deg;

	if (!ui_onscreen_ring(v, r))
		return;

	if (!pane_visible(v.y - r, v.y + r))
		return;

	p = sys_polarize_around(v, screen.centre);
	deg = p.theta;

	/* Draw the section of the ring (+ wriggle room) that will be onscreen
	 * be (start/end)ing prec degrees before/after the screen's centre
	 * relative to the ring's centre. */
	sdeg = deg + prec;
	edeg = deg - prec;

	if (r < SEGMAX)
		s = r / log10(r);
	else
		s = SEGMAX;

	/* Less segments are needed if there is less ring to place them in. */
	s *= (sdeg - edeg) / 360;

	v.y = pane_y(v.y);
	DrawRing(v, r - 1, r, sdeg, edeg, s, col);
}

void
ui_draw_texture(Texture2D texture, int x, int y) {
	if (!pane_visible(y, y + texture.height))
		return;
	DrawTexture(texture, x, pane_y(y), NO_TINT);
}

void
ui_draw_circle(int x, int y, float r, Color col) {
	if (!pane_visible(y - r, y + r))
		return;
	DrawCircle(x, pane_y(y), r, col);
}

void
ui_draw_line(int sx, int sy, int ex, int ey, float thick, Color col) {
	Vector2 s = {sx, sy};
	Vector2 e = {ex, ey};
	ui_draw_line_v(s, e, thick, col);
}

void
ui_draw_line_v(Vector2 start, Vector2 end, float thick, Color col) {
	DrawLineEx(pane_v(start), pane_v(end), thick, col);
}

void
ui_draw_border(int x, int y, int w, int h, int px) {
	if (!pane_visible(y, y + h))
		return;
	DrawRectangleLinesEx((Rectangle){x, y, w, h}, px, col_border);
}

void
ui_draw_border_around(int x, int y, int w, int h, int px) {
	ui_draw_border(x - px, y - px, w + px * 2, h + px * 2, px);
}

Vector2
ui_vectordiff(Vector2 a, Vector2 b) {
	float x = a.x - b.x;
	float y = a.y - b.y;
	if (x < 0)
		x *= -1;
	if (y < 0)
		y *= -1;
	return (Vector2) {x, y};
}

float
ui_vectordist(Vector2 a, Vector2 b) {
	Vector2 diff = ui_vectordiff(a, b);
	return sqrtf(diff.x * diff.x + diff.y * diff.y);
}

void
ui_draw_tabbed_window(int x, int y, int w, int h, Tabs *tabs) {
	ui_draw_rectangle(x, y, w, h, col_bg);
	ui_tabs(x, y, w, WINDOW_TAB_HEIGHT, tabs);
	ui_draw_border(x, y, w, h, WINDOW_BORDER);
}

void
ui_handle_view_colonies(int nowsel) {
	if (nowsel)
		ui_title("Colonies");
}

void
ui_handle_view_fleets(int nowsel) {
	if (nowsel)
		ui_title("Fleets");
}

void
ui_handle_view_design(int nowsel) {
	if (nowsel)
		ui_title("Design");
}

void
ui_handle_view_sys(int nowsel) {
	if (nowsel)
		ui_title("Systems");
	view_sys.info.geom.h = GetScreenHeight() - VIEWS_HEIGHT;
	if (!view_sys.sel)
		view_sys.sel = sys_default();
}

void
ui_handle_view_settings(int nowsel) {
	if (nowsel)
		ui_title("Settings");
}

void
ui_draw_view_colonies(void) {
	ui_print(PAD, VIEWS_HEIGHT + PAD, col_fg, "Stars/colonies here");
	ui_print(GetScreenWidth() / 2, VIEWS_HEIGHT + PAD, col_fg, "Tabs here");
	ui_print(GetScreenWidth() / 2, GetScreenHeight() / 2, col_fg, "Management stuff here");
}


void
ui_draw_view_fleets(void) {
	ui_print(PAD, VIEWS_HEIGHT + PAD, col_fg, "Groups/fleets/subfleets/ships here");
	ui_print(GetScreenWidth() / 2, VIEWS_HEIGHT + PAD, col_fg, "Tabs here");
	ui_print(GetScreenWidth() / 2, GetScreenHeight() / 2, col_fg, "Management stuff here");
}

void
ui_draw_view_design(void) {
	ui_print(PAD, VIEWS_HEIGHT + PAD, col_fg, "Designations/classes here");
	ui_print(GetScreenWidth() / 4, VIEWS_HEIGHT + PAD, col_fg, "Selectable components here");
	ui_print((GetScreenWidth() / 4) * 2, VIEWS_HEIGHT + PAD, col_fg, "Selected components");
	ui_print((GetScreenWidth() / 4) * 3, VIEWS_HEIGHT + PAD, col_fg, "Class info");
}

void
ui_draw_view_sys(void) {
	int x, y;

	x = view_sys.info.geom.x + view_sys.info.geom.w;
	y = view_sys.info.geom.y;

	/* draw map */

	/* draw divider */
	ui_draw_line(x, y, x, y + view_sys.info.geom.h, 1, col_border);

	/* draw info */
	x = view_sys.info.geom.x + PAD;
	y += PAD;
	if (view_sys.sel) {
		ui_print(x, y, col_fg, "%s", view_sys.sel->name);
		ui_draw_line(x, y + FONT_SIZE, x + view_sys.info.geom.w - 20,
					y + FONT_SIZE, 1, col_border);
		y += 30;
		ui_print(x, y,      col_fg, "Stars:     %d", view_sys.sel->num.stars);
		ui_print(x, y + 10, col_fg, "Planets:   %d", view_sys.sel->num.planets);
		ui_print(x, y + 20, col_fg, "Dwarfs:    %d", view_sys.sel->num.dwarfs);
		ui_print(x, y + 30, col_fg, "Asteroids: %d", view_sys.sel->num.asteroids);
		ui_print(x, y + 40, col_fg, "Comets:    %d", view_sys.sel->num.comets);
		ui_print(x, y + 50, col_fg, "Moons:     %d", view_sys.sel->num.moons);
		ui_draw_line(x, y + 62, x + 85, y + 62, 1, col_fg);
		/* ui_print(x, y + 65, col_fg, "Total:     %d", view_sys.sel->bodies_len); TODO: tree_count() */
	}
}

void
ui_draw_view_settings(void) {
	ui_print(PAD, VIEWS_HEIGHT + PAD, col_fg, "Settings here");
}
