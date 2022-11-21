#include "../main.h"

static View_sys *v = &view_sys;
View_sys view_sys = {
	.info = {
		.geom = {
			.x = 0,
			.y = VIEWS_HEIGHT,
			.w = 300,
			.h = 0, /* see view_sys_handle() */
		},
	},
	.pan = 0,
	.ly = {
		.x = 0,
		.y = 0,
		.topx =  0.05,
	},
	.sel = NULL,
};

void
view_sys_handle(int nowsel) {
	if (nowsel)
		ui_title("Systems");
	v->info.geom.h = screen.h - VIEWS_HEIGHT;
	if (!v->sel)
		v->sel = sys_default();
}

#define SYSDIAM (100 * v->ly.topx)

void
view_sys_draw(void) {
	Tree *t;
	System *s;
	int x, y;
	int cx, cy;

	/* draw map */
	x = v->info.geom.x + v->info.geom.w + screen.w / 2;
	y = v->info.geom.y + screen.h / 2;
	for (t = save->systems.d; t; t = t->n) {
		s = t->data;
		cx = x + s->lypos.x / v->ly.topx;
		cy = y + s->lypos.y / v->ly.topx;
		ui_draw_circle(cx, cy, SYSDIAM, col_fg);
	}

	x = v->info.geom.x;
	y = v->info.geom.y;

	/* draw divider */
	ui_draw_line(x + v->info.geom.w, v->info.geom.y,
			x + v->info.geom.w, screen.h, 1, col_border);

	/* draw info */
	x += PAD;
	y += PAD;
	if (v->sel) {
		ui_print(x, y, col_fg, "%s", v->sel->name);
		ui_draw_line(x, y + FONT_SIZE, x + v->info.geom.w - 20,
					y + FONT_SIZE, 1, col_border);
		y += 30;
		ui_print(x, y,      col_fg, "Stars:     %d", v->sel->num.stars);
		ui_print(x, y + 10, col_fg, "Planets:   %d", v->sel->num.planets);
		ui_print(x, y + 20, col_fg, "Dwarfs:    %d", v->sel->num.dwarfs);
		ui_print(x, y + 30, col_fg, "Asteroids: %d", v->sel->num.asteroids);
		ui_print(x, y + 40, col_fg, "Comets:    %d", v->sel->num.comets);
		ui_print(x, y + 50, col_fg, "Moons:     %d", v->sel->num.moons);
		ui_draw_line(x, y + 62, x + 85, y + 62, 1, col_fg);
		/* ui_print(x, y + 65, col_fg, "Total:     %d", v->sel->bodies_len); TODO: tree_count() */
	}
}
