#include <ctype.h>
#include "../main.h"

static float min_body_rad[] = {
	[BODY_STAR] = 4,
	[BODY_PLANET] = 3,
	[BODY_COMET] = 2,
	[BODY_DWARF] = 2,
	[BODY_ASTEROID] = 1,
	[BODY_MOON] = 1,
};

View_main view_main = {
	.infobox = {
		.tabs = {
			2, 0, {{NULL, "Display", 0}, {NULL, "Minerals", 0}}
		},
		.names = {
			.dwarf = {1, 1, "Name: dwarf planets"},
			.dwarfn = {1, 0, "Name: numbered dwarf planets"}, /* TODO */
			.asteroid = {1, 0, "Name: asteroids"},
			.asteroidn = {1, 0, "Name: numbered asteroids"}, /* TODO */
			.comet = {1, 1, "Name: comets"},
		},
		.orbit = {
			.dwarf = {1, 0, "Orbit: dwarf planets"},
			.asteroid = {1, 0, "Orbit: asteroids"},
			.comet = {1, 0, "Orbit: comets"},
		},
		.comettail = {1, 1, "Comet tails"}, /* TODO */
		.geom = {
			.type = UI_RECT,
			.x = PAD,
			.y = VIEWS_HEIGHT + PAD,
			.w = 200,
			.h = 400,
		},
		.pane = PANESCROLL,
	},
	.pan = 0,
	.ruler = {.held = 0},
	.kmx = 0,
	.kmy = 0,
	.kmperpx = 500000,
	.scale = {
		.x = PAD,
		.y = PAD, /* from bottom */
		.w = 50,
		.h = 3,
	},
	.sys = NULL,
};

Vector2
kmtopx(Vector2 km) {
	return (Vector2) {
		(GetScreenWidth() / 2) + (km.x - view_main.kmx) / view_main.kmperpx,
		(GetScreenHeight() / 2) + (km.y - view_main.kmy) / view_main.kmperpx
	};
}

Vector2
pxtokm(Vector2 vector) {
	return (Vector2) {
		((vector.x - GetScreenWidth() / 2) * view_main.kmperpx) + view_main.kmx,
		((vector.y - GetScreenHeight() / 2) * view_main.kmperpx) + view_main.kmy
	};
}

void
ui_handle_view_main(int nowsel) {
	Vector2 mouse = GetMousePosition();
	Vector2 delta = GetMouseDelta();
	float wheel = ui_get_scroll();
	float diff;
	Body *furth;

	if (view_main.sys)
		furth = view_main.sys->furthest_body;

#define SCROLL_DIVISOR 10
	if (!ui_collides(view_main.infobox.geom, mouse)) {
		if (wheel) {
			diff = wheel * (view_main.kmperpx/SCROLL_DIVISOR);
			if (diff > 0 || !furth || view_main.kmperpx * GetScreenHeight() <
					2 * (furth->type == BODY_COMET ? furth->maxdist : furth->dist)) {
				view_main.kmperpx -= diff;
				view_main.kmx += (mouse.x - GetScreenWidth() / 2) * diff;
				view_main.kmy += (mouse.y - GetScreenHeight() / 2) * diff;
			}
		}

		if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
			if (view_main.pan) {
				view_main.kmx -= delta.x * view_main.kmperpx;
				view_main.kmy -= delta.y * view_main.kmperpx;
			} else if (!ui_collides(view_main.infobox.geom, mouse)) {
				view_main.pan = 1;
			}
		} else {
			view_main.pan = 0;
		}

		if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) && !view_main.ruler.held) {
			view_main.ruler.held = 1;
			view_main.ruler.origin = pxtokm(mouse);
		} else if (IsMouseButtonUp(MOUSE_BUTTON_RIGHT)) {
			view_main.ruler.held = 0;
		}
	}

	if (!view_main.sys) {
		view_main.sys = sys_default();
	}

	if (nowsel)
		ui_title("Tactical: %s", view_main.sys->name);

	view_main.infobox.names.dwarfn.enabled = view_main.infobox.names.dwarf.val;
	view_main.infobox.names.asteroidn.enabled = view_main.infobox.names.asteroid.val;
}

static int
should_draw_body_checkbox(Body *body, int type, Checkbox *box) {
	if ((body->type == type || (body->parent &&
					body->parent->type == type)) &&
			!box->val)
		return 0;
	return 1;
}

static void
draw_orbit(Body *body) {
	Vector2 parent;
	float pxrad;

	if (!body->parent)
		return;
	if (!should_draw_body_checkbox(body, BODY_DWARF,
				&view_main.infobox.orbit.dwarf))
		return;
	if (!should_draw_body_checkbox(body, BODY_ASTEROID,
				&view_main.infobox.orbit.asteroid))
		return;
	if (!should_draw_body_checkbox(body, BODY_COMET,
				&view_main.infobox.orbit.comet))
		return;

	parent = kmtopx(body->parent->vector);
	pxrad = ui_vectordist(parent, body->pxloc);

	if (pxrad < min_body_rad[body->parent->type])
		return;

	if (body->type == BODY_COMET)
		ui_draw_line_v(parent, body->pxloc, 1, col_orbit);
	else
		ui_draw_ring(parent.x, parent.y, pxrad, col_orbit);
}

static void
draw_body(Body *body) {
	float w;

	if (!ui_onscreen(body->pxloc))
		return;

	/* body */
	if (body->radius / view_main.kmperpx > min_body_rad[body->type])
		w = body->radius / view_main.kmperpx;
	else
		w = min_body_rad[body->type];

	if (body->parent && body->type != BODY_COMET &&
			body->dist / view_main.kmperpx <
			min_body_rad[body->parent->type])
		return;

	ui_draw_circle(body->pxloc.x, body->pxloc.y, w, col_body[body->type]);
	if (body->type == BODY_COMET && view_main.infobox.comettail.val &&
			10 * view_main.kmperpx < body->curdist)
		ui_draw_line_v(body->pxloc, sys_vectorize_around(body->pxloc,
				(Polar){w * 11 / min_body_rad[BODY_COMET],
				body->inward ? body->theta : body->theta + 180}),
				w / min_body_rad[BODY_COMET], col_body[BODY_COMET]);

	/* name */
	if (body->type != BODY_STAR &&
			(body->type == BODY_COMET ? body->maxdist : body->dist)
			/ view_main.kmperpx < ui_textsize(body->name))
		return;
	if (body->parent && body->type != BODY_STAR &&
			ui_vectordist(body->vector, body->parent->vector) <
			min_body_rad[body->type] * view_main.kmperpx)
		return;
	if (isdigit(*body->name) || *body->name == '(') {
		if (!should_draw_body_checkbox(body, BODY_DWARF,
					&view_main.infobox.names.dwarfn))
			return;
		if (!should_draw_body_checkbox(body, BODY_ASTEROID,
					&view_main.infobox.names.asteroidn))
			return;
	}
	if (!should_draw_body_checkbox(body, BODY_DWARF,
				&view_main.infobox.names.dwarf))
		return;
	if (!should_draw_body_checkbox(body, BODY_ASTEROID,
				&view_main.infobox.names.asteroid))
		return;
	if (!should_draw_body_checkbox(body, BODY_COMET,
				&view_main.infobox.names.comet))
		return;

	ui_print(body->pxloc.x + w + 2, body->pxloc.y + w + 2,
			col_fg, "%s", body->name);
}

void
ui_draw_view_main(void) {
	Vector2 mouse = GetMousePosition();
	Vector2 mousekm = pxtokm(mouse);
	Vector2 ruler;
	Geom geom;
	Body *body;
	float dist;
	size_t i;
	float x, y;

	/* debug info */
	ui_print(GetScreenWidth() / 2, VIEWS_HEIGHT + PAD, col_fg, "W: %f | H: %f", (float)screen.w, (float)screen.h);
	ui_print(GetScreenWidth() / 2, VIEWS_HEIGHT + PAD * 2, col_fg, "Xoff: %f | Yoff: %f | km/px: %f",
			view_main.kmx, view_main.kmy, view_main.kmperpx);
	ui_print(GetScreenWidth() / 2, VIEWS_HEIGHT + PAD * 3, col_fg, "X: %f | Y: %f",
			mousekm.x, mousekm.y);
	ui_print(GetScreenWidth() / 2, VIEWS_HEIGHT + PAD * 4, col_fg, "FPS: %d (target: %d)", GetFPS(), TARGET_FPS);

	/* draw system bodies */
	for (i = 0; i < view_main.sys->bodies_len; i++) {
		body = view_main.sys->bodies[i];
		body->pxloc = kmtopx(body->vector);
		draw_orbit(body);
	}
	for (i = 0; i < view_main.sys->bodies_len; i++) {
		body = view_main.sys->bodies[i];
		draw_body(body);
	}

	/* ruler */
	if (view_main.ruler.held) {
		ruler = kmtopx(view_main.ruler.origin);
		ui_draw_line_v(ruler, mouse, 1, col_info);
		dist = ui_vectordist(view_main.ruler.origin, mousekm);
		ui_print(mouse.x + PAD, mouse.y - PAD, col_info, "%s (%s)", strkm(dist), strly(dist));
	}

	/* scale */
	ui_draw_rectangle(view_main.scale.x,
			GetScreenHeight() - view_main.scale.y,
			view_main.scale.w, 1, col_info); /* horizontal */
	ui_draw_rectangle(view_main.scale.x,
			GetScreenHeight() - view_main.scale.y - view_main.scale.h,
			1, view_main.scale.h, col_info); /* left vertical */
	ui_draw_rectangle(view_main.scale.x + view_main.scale.w,
			GetScreenHeight() - view_main.scale.y - view_main.scale.h,
			1, view_main.scale.h, col_info); /* right vertical */
	dist = view_main.scale.w * view_main.kmperpx;
	ui_print(view_main.scale.x + view_main.scale.w + FONT_SIZE / 3,
			GetScreenHeight() - view_main.scale.y - FONT_SIZE / 2,
			col_info, "%s", strkm(dist));

	/* infobox */
	ui_draw_tabbed_window(EXPLODE_RECT(view_main.infobox.geom),
			&view_main.infobox.tabs);
	x = view_main.infobox.geom.x + FONT_SIZE;
	y = view_main.infobox.geom.y + WINDOW_TAB_HEIGHT;
	geom = RECT(x - FONT_SIZE,
			y,
			view_main.infobox.geom.w - WINDOW_BORDER,
			view_main.infobox.geom.h - WINDOW_TAB_HEIGHT - WINDOW_BORDER);
	view_main.infobox.pane.geom = &geom;

	pane_begin(&view_main.infobox.pane);
	ui_draw_checkbox(x, y += FONT_SIZE*1.5, &view_main.infobox.names.dwarf);
	ui_draw_checkbox(x, y += FONT_SIZE*1.5, &view_main.infobox.names.dwarfn);
	ui_draw_checkbox(x, y += FONT_SIZE*1.5, &view_main.infobox.names.asteroid);
	ui_draw_checkbox(x, y += FONT_SIZE*1.5, &view_main.infobox.names.asteroidn);
	ui_draw_checkbox(x, y += FONT_SIZE*1.5, &view_main.infobox.names.comet);
	ui_draw_checkbox(x, y += FONT_SIZE*1.5, &view_main.infobox.orbit.dwarf);
	ui_draw_checkbox(x, y += FONT_SIZE*1.5, &view_main.infobox.orbit.asteroid);
	ui_draw_checkbox(x, y += FONT_SIZE*1.5, &view_main.infobox.orbit.comet);
	ui_draw_checkbox(x, y += FONT_SIZE*1.5, &view_main.infobox.comettail);
	pane_end();
}
