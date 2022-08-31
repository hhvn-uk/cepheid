#include "../main.h"

#define INFOBOXES 3
#define INFOBOX_H (FONT_SIZE * 11 + PAD * 2)
#define INFOBOX_W ((screen.w - PAD * (1 + INFOBOXES)) / INFOBOXES)
#define BUTTONS 50
#define W MIN(screen.w / 20, 50)

View_bodies view_bodies = {
	.sys = NULL,
	.sel = NULL,
	.show = {
		.planet = {1, 1, "Show planets"},
		.moon = {1, 1, "Show moons"},
		.dwarf = {1, 1, "Show dwarf planets"},
		.asteroid = {1, 1, "Show asteroids"},
		.comet = {1, 1, "Show comets"},
		.nomineral = {1, 1, "Show bodies without minerals"},
	},
	.stars = PANESCROLL,
	.bodies = PANESCROLL,
};

void
ui_handle_view_bodies(int nowsel) {
	if (!view_bodies.sys)
		view_bodies.sys = sys_default();
	if (nowsel)
		ui_title("Bodies in %s", view_bodies.sys->name);
}

static int
draw_star(int x, int y, Body *star) {
	ui_print(x, y, col_fg, "%s", star->name);
	return y + FONT_SIZE;
}

static int
draw_body(int x, int y, Body *body) {
	ui_print(x, y, col_fg, "%s", body->name);
	ui_print(x + 3*W, y, col_fg, "%s", body->parent ? body->parent->name : "-");
	ui_print(x + 5*W, y, col_fg, "%s", bodytype_strify(body));
	return y + FONT_SIZE;
}

static int
display_body(Body *body) {
	if (body->type == BODY_STAR || !body->parent)
		return 0;
	if (body->type == BODY_MOON && !view_bodies.show.moon.val)
		return 0;
	if ((body->type == BODY_PLANET || body->parent->type == BODY_PLANET) &&
			!view_bodies.show.planet.val)
		return 0;
	if ((body->type == BODY_DWARF || body->parent->type == BODY_DWARF) &&
			!view_bodies.show.dwarf.val)
		return 0;
	if ((body->type == BODY_ASTEROID || body->parent->type == BODY_ASTEROID) &&
			!view_bodies.show.asteroid.val)
		return 0;
	if ((body->type == BODY_COMET || body->parent->type == BODY_COMET) &&
			!view_bodies.show.comet.val)
		return 0;
	/* TODO: exclude bodies with no minerals */
	return 1;
}

void
ui_draw_view_bodies(void) {
	Rect stars = { PAD, VIEWS_HEIGHT + PAD * 2 + FONT_SIZE,
		screen.w - PAD * 2, FONT_SIZE * 6 };
	Rect checkboxes = { stars.x, stars.y + stars.h + PAD,
		stars.w, FONT_SIZE + PAD};
	Rect bodies = { checkboxes.x, checkboxes.y + checkboxes.h + PAD/2,
		checkboxes.w, screen.h - bodies.y - INFOBOX_H - BUTTONS };
	Rect location = { bodies.x, bodies.y + bodies.h + PAD,
		INFOBOX_W, INFOBOX_H };
	Rect minerals = { location.x + location.w + PAD, location.y,
		INFOBOX_W, INFOBOX_H };
	Rect habitat = { minerals.x + minerals.w + PAD, minerals.y,
		INFOBOX_W, INFOBOX_H };
	int x, y;
	int i;
	Body *body;

	if (!view_bodies.sel)
		bodies.h += INFOBOX_H;

	ui_print(PAD, VIEWS_HEIGHT + PAD, col_fg, "[System selection dropdown]");

	ui_draw_border_around(EXPLODE_RECT(stars), 1);
	view_bodies.stars.geom = &stars;
	pane_begin(&view_bodies.stars);
	x = stars.x + PAD/2;
	y = stars.y + PAD/2;
	ui_print(x, y, col_fg, "Name");
	y += FONT_SIZE * 1.5;
	for (i = 0; i < view_bodies.sys->bodies_len; i++)
		if (view_bodies.sys->bodies[i]->type == BODY_STAR)
			y = draw_star(x, y, view_bodies.sys->bodies[i]);
	pane_end();

	x = checkboxes.x + PAD/2;
	y = checkboxes.y + PAD/2;
	x += ui_draw_checkbox(x, y, &view_bodies.show.planet) + PAD * 2;
	x += ui_draw_checkbox(x, y, &view_bodies.show.moon) + PAD * 2;
	x += ui_draw_checkbox(x, y, &view_bodies.show.dwarf) + PAD * 2;
	x += ui_draw_checkbox(x, y, &view_bodies.show.asteroid) + PAD * 2;
	x += ui_draw_checkbox(x, y, &view_bodies.show.comet) + PAD * 2;
	x += ui_draw_checkbox(x, y, &view_bodies.show.nomineral);
	ui_draw_border_around(checkboxes.x, checkboxes.y, x, checkboxes.h, 1);

	ui_draw_border_around(EXPLODE_RECT(bodies), 1);
	view_bodies.bodies.geom = &bodies;
	pane_begin(&view_bodies.bodies);
	x = bodies.x + PAD/2;
	y = bodies.y + PAD/2;
	ui_print(x, y, col_fg, "Name");
	ui_print(x + 3*W, y, col_fg, "Parent");
	ui_print(x + 5*W, y, col_fg, "Type");
	y += FONT_SIZE * 1.5;
	for (i = 0; i < view_bodies.sys->bodies_len; i++) {
		body = view_bodies.sys->bodies[i];
		if (display_body(body))
			y = draw_body(x, y, body);
	}
	pane_end();

	if (view_bodies.sel) {
		ui_draw_border_around(EXPLODE_RECT(location), 1);
		ui_draw_border_around(EXPLODE_RECT(minerals), 1);
		ui_draw_border_around(EXPLODE_RECT(habitat), 1);
	}
}
