#include "../main.h"

#define INFOBOXES 3
#define INFOBOX_H (FONT_SIZE * 11 + PAD * 2)
#define INFOBOX_W ((screen.w - PAD * (1 + INFOBOXES)) / INFOBOXES)
#define BUTTONS 50
#define W MIN(screen.w / 20, 50)

static View_bodies *v = &view_bodies;
View_bodies view_bodies = {
	.sys = NULL,
	.selstar = NULL,
	.sel = NULL,
	.show = {
		.planet = {1, 1, "Show planets"},
		.moon = {1, 1, "Show moons"},
		.dwarf = {1, 1, "Show dwarf planets"},
		.asteroid = {1, 1, "Show asteroids"},
		.comet = {1, 1, "Show comets"},
		.nomineral = {1, 1, "Show bodies without mins"},
	},
	.pane = {
		.stars = PANESCROLL,
		.bodies = PANESCROLL,
	},
};

static int
display_body(Body *body) {
	if (body->type == BODY_STAR || !body->parent)
		return 0;
	if (body->type == BODY_MOON && !v->show.moon.val)
		return 0;
	if ((body->type == BODY_PLANET || body->parent->type == BODY_PLANET) &&
			!v->show.planet.val)
		return 0;
	if ((body->type == BODY_DWARF || body->parent->type == BODY_DWARF) &&
			!v->show.dwarf.val)
		return 0;
	if ((body->type == BODY_ASTEROID || body->parent->type == BODY_ASTEROID) &&
			!v->show.asteroid.val)
		return 0;
	if ((body->type == BODY_COMET || body->parent->type == BODY_COMET) &&
			!v->show.comet.val)
		return 0;
	/* TODO: exclude bodies with no mins */
	return 1;
}

void
ui_handle_view_bodies(int nowsel) {
	Vector2 m = GetMousePosition();
	int pos, i, j;

	if (!v->sys)
		v->sys = sys_default();

	if (!nowsel) {
		if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
				ui_collides_rect(v->bodies, m)) {
			pos = (m.y + v->pane.bodies.off - v->bodies.y - PAD)
				/ FONT_SIZE;
			for (i = j = 0; i < v->sys->bodies_len && j < pos; i++)
				if (display_body(v->sys->bodies[i]))
					if (++j == pos)
						v->sel = v->sys->bodies[i];
		}
	}

	if (nowsel)
		ui_title("Bodies in %s", v->sys->name);
}

static int
draw_star(int x, int y, Body *star) {
	ui_print(x, y, col_fg, "%s", star->name);
	return y + FONT_SIZE;
}

static int
draw_body(int x, int y, Body *body) {
	Color col;

	if (body == v->sel)
		col = col_info;
	else
		col = col_fg;

	ui_print(x, y, col, "%s", body->name);
	ui_print(x + 3*W, y, col, "%s", body->parent ? body->parent->name : "-");
	ui_print(x + 5*W, y, col, "%s", bodytype_strify(body));
	return y + FONT_SIZE;
}

void
ui_draw_view_bodies(void) {
	int x, y;
	int i;
	Body *body;

	v->stars = (Rect){ PAD, VIEWS_HEIGHT + PAD * 2 + FONT_SIZE,
		screen.w - PAD * 2, FONT_SIZE * 6 };
	v->disp = (Rect){ v->stars.x, v->stars.y + v->stars.h + PAD,
		v->stars.w, FONT_SIZE + PAD};
	v->bodies = (Rect){ v->disp.x, v->disp.y + v->disp.h + PAD/2,
		v->disp.w, screen.h - v->bodies.y - INFOBOX_H - BUTTONS };
	v->loc = (Rect){ v->bodies.x, v->bodies.y + v->bodies.h + PAD,
		INFOBOX_W, INFOBOX_H };
	v->mins = (Rect){ v->loc.x + v->loc.w + PAD, v->loc.y,
		INFOBOX_W, INFOBOX_H };
	v->hab = (Rect){ v->mins.x + v->mins.w + PAD, v->mins.y,
		INFOBOX_W, INFOBOX_H };

	if (!v->sel)
		v->bodies.h += INFOBOX_H;

	ui_print(PAD, VIEWS_HEIGHT + PAD, col_fg, "[System selection dropdown]");

	ui_draw_border_around(EXPLODE_RECT(v->stars), 1);
	v->pane.stars.geom = &v->stars;
	pane_begin(&v->pane.stars);
	x = v->stars.x + PAD/2;
	y = v->stars.y + PAD/2;
	ui_print(x, y, col_fg, "Name");
	y += FONT_SIZE * 1.5;
	for (i = 0; i < v->sys->bodies_len; i++)
		if (v->sys->bodies[i]->type == BODY_STAR)
			y = draw_star(x, y, v->sys->bodies[i]);
	pane_end();

	x = v->disp.x + PAD/2;
	y = v->disp.y + PAD/2;
	x += ui_draw_checkbox(x, y, &v->show.planet) + PAD * 2;
	x += ui_draw_checkbox(x, y, &v->show.moon) + PAD * 2;
	x += ui_draw_checkbox(x, y, &v->show.dwarf) + PAD * 2;
	x += ui_draw_checkbox(x, y, &v->show.asteroid) + PAD * 2;
	x += ui_draw_checkbox(x, y, &v->show.comet) + PAD * 2;
	x += ui_draw_checkbox(x, y, &v->show.nomineral);
	ui_draw_border_around(v->disp.x, v->disp.y, x, v->disp.h, 1);

	ui_draw_border_around(EXPLODE_RECT(v->bodies), 1);
	v->pane.bodies.geom = &v->bodies;
	pane_begin(&v->pane.bodies);
	x = v->bodies.x + PAD/2;
	y = v->bodies.y + PAD/2;
	ui_print(x, y, col_fg, "Name");
	ui_print(x + 3*W, y, col_fg, "Parent");
	ui_print(x + 5*W, y, col_fg, "Type");
	y += FONT_SIZE * 1.5;
	for (i = 0; i < v->sys->bodies_len; i++) {
		body = v->sys->bodies[i];
		if (display_body(body))
			y = draw_body(x, y, body);
	}
	pane_end();

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
					"%s (%s)", strkmdist(v->sel->dist),
					strlightdist(v->sel->dist));
		}

		ui_draw_border_around(EXPLODE_RECT(v->mins), 1);
		ui_draw_border_around(EXPLODE_RECT(v->hab), 1);
	}

	DrawFPS(50, 50);
}
