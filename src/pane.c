#include <raylib.h>
#include "main.h"

#define SCROLLBAR_W 4
#define SCROLL_MULT 50

static Pane *pane = NULL;

void
pane_begin(Pane *f) {
	f->max = 0;
	BeginScissorMode(f->geom->x, f->geom->y,
			f->geom->w, f->geom->h);
	pane = f;
}

void
pane_end(void) {
	Pane *f = pane;
	Vector2 m = GetMousePosition();

	if (!f) return;

	pane = NULL; /* unset pane so we can draw freely */
	if (f->max > f->geom->h) {
		ui_draw_rectangle(f->geom->x + f->geom->w - SCROLLBAR_W,
				f->geom->y,
				SCROLLBAR_W,
				f->geom->h,
				col_altbg);
		ui_draw_rectangle(f->geom->x + f->geom->w - SCROLLBAR_W,
				f->geom->y + f->geom->h * f->off / f->max,
				SCROLLBAR_W,
				(float)f->geom->h / (float)f->max * (float)f->geom->h,
				col_fg);
	}
	if (m.x >= f->geom->x && m.x <= f->geom->x + f->geom->w &&
			m.y >= f->geom->y && m.y <= f->geom->y + f->geom->h)
		f->off -= ui_get_scroll() * SCROLL_MULT;
	if (f->off > f->max - f->geom->h)
		f->off = f->max - f->geom->h;
	if (f->off < 0)
		f->off = 0;
	EndScissorMode();
}

int
pane_visible(float miny, float maxy) {
	if (!pane) return 1;

	Rectangle a = {pane->geom->x, miny, 1, maxy - miny};
	Rectangle b = {pane->geom->x, pane->geom->y + pane->off,
		1, pane->geom->h};

	pane_max(maxy);
	return CheckCollisionRecs(a, b);
}

float
pane_max(float y) {
	if (pane && y - pane->geom->y > pane->max)
		pane->max = y - pane->geom->y;
	return y;
}

float
pane_y(float y) {
	if (!pane) return y;
	return y - pane->off;
}

Vector2
pane_v(Vector2 v) {
	if (!pane)
		return (Vector2) {v.x, v.y};
	return (Vector2) {v.x, v.y - pane->off};
}
