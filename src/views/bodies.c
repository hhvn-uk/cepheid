#include "../main.h"

#define INFOBOXES 3
#define INFOBOX_H (FONT_SIZE * 11 + PAD * 2)
#define INFOBOX_W ((screen.w - PAD * (1 + INFOBOXES)) / INFOBOXES)
#define BUTTONS 50
#define W MIN(screen.w / 20, 50)

static int tree_filter(Tree *t, void *data);
static void tree_printer(int x, int y, Treeview *tv, Tree *t);

static View_bodies *v = &view_bodies;
View_bodies view_bodies = {
	.sel = NULL,
	.show = {
		.planet = {1, 1, "Show planets"},
		.moon = {1, 1, "Show moons"},
		.dwarf = {1, 1, "Show dwarf planets"},
		.asteroid = {1, 1, "Show asteroids"},
		.comet = {1, 1, "Show comets"},
		.nomineral = {1, 1, "Show bodies without mins"},
	},
	.tree = {
		.t = NULL,
		.sel = NULL,
		.selmask = SYSTREE_BODY,
		.colmask = SYSTREE_SYS,
		.filter = tree_filter,
		.print = tree_printer,
	}
};

void
view_bodies_init(void) {
	v->sel = NULL;
	v->tree.t = NULL;
	v->tree.sel = NULL;
}

static int
tree_filter(Tree *t, void *data) {
	Body *b;

	if (t->type == SYSTREE_SYS)
		return 1;

	b = t->data;
	if (b->type == BODY_STAR || !b->parent)
		return 0;
	if (b->type == BODY_MOON && !v->show.moon.val)
		return 0;
	if ((b->type == BODY_PLANET || b->parent->type == BODY_PLANET) && !v->show.planet.val)
		return 0;
	if ((b->type == BODY_DWARF || b->parent->type == BODY_DWARF) && !v->show.dwarf.val)
		return 0;
	if ((b->type == BODY_ASTEROID || b->parent->type == BODY_ASTEROID) && !v->show.asteroid.val)
		return 0;
	if ((b->type == BODY_COMET || b->parent->type == BODY_COMET) && !v->show.comet.val)
		return 0;
	/* TODO: exclude bodies with no mins */
	return 1;
}

static void
tree_printer(int x, int y, Treeview *tv, Tree *t) {
	int sel = tv->sel == t;
	Color c = sel ? col_info : col_fg;
	Body *b;
	int cx = tv->rect.x;
	int cw = tv->rect.w;

	if (!t) {
		ui_print(x, y, col_fg, "Body");
		ui_print(cx + 3*W, y, col_fg, "Parent");
		ui_print(cx + 5*W, y, col_fg, "Type");
	} else if (t->type != SYSTREE_BODY) {
		ui_print(x, y, c, t->name);
	} else {
		b = t->data;
		ui_printw(x, y, cx + 3*W - x, c, "%s", b->name);
		ui_printw(cx + 3*W, y, 2*W, c, "%s", b->parent ? b->parent->name : "");
		ui_printw(cx + 5*W, y, cw, c, "%s", bodytype_strify(b));
	}
}

void
view_bodies_handle(int nowsel) {
	v->prevframe.sel = v->sel;

	if (!nowsel) {
		if (v->tree.sel)
			v->sel = v->tree.sel->data;
		else
			v->sel = NULL;
	} else {
		v->tree.t = &save->systems;
	}
}

void
view_bodies_draw(void) {
	int x, y;

	v->disp = RECT(PAD, VIEWS_HEIGHT + FONT_SIZE,
		screen.w - PAD * 2, FONT_SIZE + PAD);
	v->bodies = RECT(v->disp.x, v->disp.y + v->disp.h + PAD/2,
		v->disp.w, screen.h - v->bodies.y - INFOBOX_H - BUTTONS);
	v->loc = RECT(v->bodies.x, v->bodies.y + v->bodies.h + PAD,
		INFOBOX_W, INFOBOX_H);
	v->mins = RECT(v->loc.x + v->loc.w + PAD, v->loc.y,
		INFOBOX_W, INFOBOX_H);
	v->hab = RECT(v->mins.x + v->mins.w + PAD, v->mins.y,
		INFOBOX_W, INFOBOX_H);

	if (!v->sel)
		v->bodies.h += INFOBOX_H;

	x = v->disp.x + PAD/2;
	y = v->disp.y + PAD/2;
	x += gui_checkbox(x, y, &v->show.planet) + PAD * 2;
	x += gui_checkbox(x, y, &v->show.moon) + PAD * 2;
	x += gui_checkbox(x, y, &v->show.dwarf) + PAD * 2;
	x += gui_checkbox(x, y, &v->show.asteroid) + PAD * 2;
	x += gui_checkbox(x, y, &v->show.comet) + PAD * 2;
	x += gui_checkbox(x, y, &v->show.nomineral);
	ui_draw_border_around(v->disp.x, v->disp.y, x, v->disp.h, 1);

	if (v->sel && !v->prevframe.sel && mouse.y > v->bodies.y + v->bodies.h)
		pane_scroll(&v->tree.pane, INFOBOX_H);

	gui_treeview(EXPLODE_RECT(v->bodies), &v->tree);

	if (v->sel) {
		ui_draw_border_around(EXPLODE_RECT(v->loc), 1);
		x = v->loc.x + PAD/2;
		y = v->loc.y + PAD/2;
		ui_print(x, y, col_info, "Orbital period:");
		ui_print(x + INFOBOX_W / 2, y, col_fg,
				"%.2f days", v->sel->orbdays);
		if (v->sel->type != BODY_COMET) {
			ui_print(x, y += FONT_SIZE, col_info, "Orbital distance:");
			ui_print(x + INFOBOX_W / 2, y, col_fg,
					"%s (%s)", strkm(v->sel->dist),
					strly(v->sel->dist));
		}

		ui_draw_border_around(EXPLODE_RECT(v->mins), 1);
		ui_draw_border_around(EXPLODE_RECT(v->hab), 1);
	}
}
