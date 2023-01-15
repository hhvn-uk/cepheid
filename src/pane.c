#include <raylib.h>
#include "main.h"

#define SCROLLBAR_W 4
#define SCROLL_MULT 50

static Pane *pane = NULL;

void
pane_begin(Pane *p) {
	p->max = 0;
	assert(p->geom->type == UI_RECT);
	BeginScissorMode(p->geom->x, p->geom->y,
			p->geom->w, p->geom->h);
	pane = p;
}

void
pane_end(void) {
	Pane *p = pane;
	Vector m = mouse.vector;

	if (!p) return;

	pane = NULL; /* unset pane so we can draw freely */
	if (p->max > p->geom->h) {
		ui_draw_rect(p->geom->x + p->geom->w - SCROLLBAR_W,
				p->geom->y,
				SCROLLBAR_W,
				p->geom->h,
				col_altbg);
		ui_draw_rect(p->geom->x + p->geom->w - SCROLLBAR_W,
				p->geom->y + p->geom->h * p->off / p->max,
				SCROLLBAR_W,
				(float)p->geom->h / (float)p->max * (float)p->geom->h,
				col_fg);
	}
	if (m.x >= p->geom->x && m.x <= p->geom->x + p->geom->w &&
			m.y >= p->geom->y && m.y <= p->geom->y + p->geom->h)
		pane_scroll(p, mouse.scroll * SCROLL_MULT * -1);
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

Vector
pane_v(Vector v) {
	if (!pane)
		return (Vector) {v.x, v.y};
	return (Vector) {v.x, v.y - pane->off};
}

void
pane_scroll(Pane *p, int incr) {
	p->off += incr;
	if (p->off > p->max - p->geom->h)
		p->off = p->max - p->geom->h;
	if (p->off < 0)
		p->off = 0;
}
