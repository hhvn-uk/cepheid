#include <math.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <raylib.h>
#include <wchar.h>
#include "main.h"

Screen screen = { 0 };
Focus focus = { 0 };
Mouse mouse = { 0 };
int charpx; /* thank god for monospaced fonts */

void
ui_init(void) {
	SetWindowState(FLAG_WINDOW_RESIZABLE|FLAG_WINDOW_HIDDEN);
	SetTargetFPS(TARGET_FPS);
	InitWindow(500, 500, "");
	SetExitKey(KEY_NULL);
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
ui_focus(enum GuiElements type, void *p) {
	focus.type = type;
	focus.p = p;
}

int
ui_loop(void) {
	if (WindowShouldClose() || sigint || sigterm)
		checkbeforequit();

	if (((sigint || sigterm) && view_smenu.save.check) || quit)
		return 0;

	ffree();
	if (IsWindowResized())
		ui_update_screen();
	gui_key_handle();
	gui_mouse_handle();

	mouse.vector = GetMousePosition();
	mouse.delta = GetMouseDelta();
	mouse.x = mouse.vector.x;
	mouse.y = mouse.vector.y;
	mouse.scroll = GetMouseWheelMove();
	if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))
		mouse.scroll *= 2;
	return 1;
}

void
ui_deinit(void) {
	CloseWindow();
}

void
ui_print(int x, int y, Color col, char *fmt, ...) {
	va_list ap;
	Vector pos = {x, pane_y(y)};
	char *text;

	if (!pane_visible(y, y + FONT_SIZE))
		return;

	pos.x = x;
	va_start(ap, fmt);
	text = vsmprintf(fmt, ap);
	va_end(ap);
	DrawTextEx(font, text, pos, (float)FONT_SIZE, FONT_SIZE/10, col);
	free(text);
}

void
ui_printw(int x, int y, int w, Color col, char *fmt, ...) {
	va_list ap;
	Vector pos = {x, pane_y(y)};
	char *t1, *t2;

	if (!pane_visible(y, y + FONT_SIZE))
		return;

	pos.x = x;
	va_start(ap, fmt);
	t1 = vsmprintf(fmt, ap);
	va_end(ap);
	t2 = strtrunc(t1, w / charpx);
	DrawTextEx(font, t2, pos, (float)FONT_SIZE, FONT_SIZE/10, col);
	free(t1);
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
	if (!text) return 0;
	return charpx * strlen(text);
}

void
ui_cursor(MouseCursor curs) {
	SetMouseCursor(curs);
}

int
ui_collides(Geom geom, Vector point) {
	switch (geom.type) {
	case UI_CIRCLE:
		return CheckCollisionPointCircle(point,
				EXPLODE_CIRCLEV(geom));
	case UI_RECT: default: /* -Wreturn-type bitches without default */
		return CheckCollisionPointRec(point, RLIFY_RECT(geom));
	}
}

int
ui_onscreen(Vector point) {
	if (!pane_visible(point.y, point.y))
		return 0;
	point.y = pane_y(point.y);
	return ui_collides(screen.rect, point);
}

int
ui_onscreen_ring(Vector centre, float r) {
	float d = vector_dist(centre, screen.centre);

	if (!pane_visible(centre.y - r, centre.y + r))
		return 0;
	centre.y = pane_y(centre.y);

	if (d + screen.diag / 2 < r)
		return 0;
	return CheckCollisionCircleRec(centre, r, RLIFY_RECT(screen.rect));
}

int
ui_onscreen_circle(Vector centre, float r) {
	if (!pane_visible(centre.y - r, centre.y + r))
		return 0;
	centre.y = pane_y(centre.y);
	return CheckCollisionCircleRec(centre, r, RLIFY_RECT(screen.rect));
}

void
ui_draw_views(void) {
	int sw = GetScreenWidth();
	if (sw > VIEWS_MAX_WIDTH) sw = VIEWS_MAX_WIDTH;
	gui_tabs(0, 0, sw, VIEWS_HEIGHT, &view_tabs);
#ifdef DEBUG
	DrawFPS(sw + FONT_SIZE, FONT_SIZE / 2);
#endif /* DEBUG */
}

void
ui_draw_rect(int x, int y, int w, int h, Color col) {
	if (pane_visible(y, y + h))
		DrawRectangle(x, pane_y(y), w, h, col);
}

void
ui_draw_expander(int x, int y, int w, int expanded) {
	int p = (w / 2) - 1;
	if (!pane_visible(y, y + w))
		return;

	if ((w-1) & 1) warning("drawing plus with even width\n");

	DrawRectangle(x, y, w, w, col_altbg);
	DrawRectangle(x + 1, y + p + 1, w - 2, 1, col_fg);
	if (!expanded)
		DrawRectangle(x + p + 1, y + 1, 1, w - 2, col_fg);
}

#define SEGMAX 1500

void
ui_draw_ring(int x, int y, float r, Color col) {
	Vector v = {x, y};
	Polar p;
	float s;
	float prec = screen.diag * 1.5 / (PI * 2 * r) * 360;
	float sdeg = 0, edeg = 360;
	float deg;

	if (!ui_onscreen_ring(v, r))
		return;

	if (!pane_visible(v.y - r, v.y + r))
		return;

	p = polarize_at(v, screen.centre);
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
ui_draw_texture_part(Texture2D texture, int x, int y, int fx, int fy, int w, int h) {
	if (!pane_visible(y, y + h))
		return;
	DrawTextureRec(texture, (Rectangle){fx, fy, w, h}, (Vector){x, y}, NO_TINT);
}

void
ui_draw_circle(int x, int y, float r, Color col) {
	if (!pane_visible(y - r, y + r))
		return;
	DrawCircle(x, pane_y(y), r, col);
}

void
ui_draw_line(int sx, int sy, int ex, int ey, float thick, Color col) {
	Vector s = {sx, sy};
	Vector e = {ex, ey};
	ui_draw_line_v(s, e, thick, col);
}

void
ui_draw_line_v(Vector start, Vector end, float thick, Color col) {
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

void
ui_draw_tabbed_window(int x, int y, int w, int h, Tabs *tabs) {
	ui_draw_rect(x, y, w, h, col_bg);
	gui_tabs(x, y, w, WINDOW_TAB_HEIGHT, tabs);
	ui_draw_border(x, y, w, h, WINDOW_BORDER);
}
