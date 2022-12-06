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

static View_main *v = &view_main;
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

void
view_main_init(void) {
	v->kmx = v->kmy = 0;
	v->kmperpx = 500000;

	/* since sys_default() checks this value, set it to NULL first */
	v->sys = NULL;
	v->sys = sys_default();
}

Vector
kmtopx(Vector km) {
	return (Vector) {
		(screen.w / 2) + (km.x - v->kmx) / v->kmperpx,
		(screen.h / 2) + (km.y - v->kmy) / v->kmperpx
	};
}

Vector
pxtokm(Vector vector) {
	return (Vector) {
		((vector.x - screen.w / 2) * v->kmperpx) + v->kmx,
		((vector.y - screen.h / 2) * v->kmperpx) + v->kmy
	};
}

void
view_main_handle(int nowsel) {
	float wheel = mouse.scroll;
	float diff;
	Body *furth;

	if (v->sys)
		furth = v->sys->furthest_body;

#define SCROLL_DIVISOR 10
	if (!ui_collides(v->infobox.geom, mouse.vector)) {
		if (wheel) {
			diff = wheel * (v->kmperpx/SCROLL_DIVISOR);
			if (diff > 0 || !furth || v->kmperpx * screen.h <
					2 * (furth->type == BODY_COMET ? furth->maxdist : furth->dist)) {
				v->kmperpx -= diff;
				v->kmx += (mouse.x - screen.w / 2) * diff;
				v->kmy += (mouse.y - screen.h / 2) * diff;
			}
		}

		if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
			if (v->pan) {
				v->kmx -= mouse.delta.x * v->kmperpx;
				v->kmy -= mouse.delta.y * v->kmperpx;
			} else if (!ui_collides(v->infobox.geom, mouse.vector)) {
				v->pan = 1;
			}
		} else {
			v->pan = 0;
		}

		if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) && !v->ruler.held) {
			v->ruler.held = 1;
			v->ruler.origin = pxtokm(mouse.vector);
		} else if (IsMouseButtonUp(MOUSE_BUTTON_RIGHT)) {
			v->ruler.held = 0;
		}
	}

	if (nowsel)
		ui_title("Tactical: %s", v->sys->name);

	v->infobox.names.dwarfn.enabled = v->infobox.names.dwarf.val;
	v->infobox.names.asteroidn.enabled = v->infobox.names.asteroid.val;
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
	Vector parent;
	float pxrad;

	if (!body->parent)
		return;
	if (!should_draw_body_checkbox(body, BODY_DWARF,
				&v->infobox.orbit.dwarf))
		return;
	if (!should_draw_body_checkbox(body, BODY_ASTEROID,
				&v->infobox.orbit.asteroid))
		return;
	if (!should_draw_body_checkbox(body, BODY_COMET,
				&v->infobox.orbit.comet))
		return;

	parent = kmtopx(body->parent->vector);
	pxrad = vector_dist(parent, body->pxloc);

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
	if (body->radius / v->kmperpx > min_body_rad[body->type])
		w = body->radius / v->kmperpx;
	else
		w = min_body_rad[body->type];

	if (body->parent && body->type != BODY_COMET &&
			body->dist / v->kmperpx <
			min_body_rad[body->parent->type])
		return;

	ui_draw_circle(body->pxloc.x, body->pxloc.y, w, col_body[body->type]);
	if (body->type == BODY_COMET && v->infobox.comettail.val &&
			10 * v->kmperpx < body->curdist)
		ui_draw_line_v(body->pxloc, vectorize_at(body->pxloc,
				(Polar){w * 11 / min_body_rad[BODY_COMET],
				body->inward ? body->theta : body->theta + 180}),
				w / min_body_rad[BODY_COMET], col_body[BODY_COMET]);

	/* name */
	if (body->type != BODY_STAR &&
			(body->type == BODY_COMET ? body->maxdist : body->dist)
			/ v->kmperpx < ui_textsize(body->name))
		return;
	if (body->parent && body->type != BODY_STAR &&
			vector_dist(body->vector, body->parent->vector) <
			min_body_rad[body->type] * v->kmperpx)
		return;
	if (isdigit(*body->name) || *body->name == '(') {
		if (!should_draw_body_checkbox(body, BODY_DWARF,
					&v->infobox.names.dwarfn))
			return;
		if (!should_draw_body_checkbox(body, BODY_ASTEROID,
					&v->infobox.names.asteroidn))
			return;
	}
	if (!should_draw_body_checkbox(body, BODY_DWARF,
				&v->infobox.names.dwarf))
		return;
	if (!should_draw_body_checkbox(body, BODY_ASTEROID,
				&v->infobox.names.asteroid))
		return;
	if (!should_draw_body_checkbox(body, BODY_COMET,
				&v->infobox.names.comet))
		return;

	ui_print(body->pxloc.x + w + 2, body->pxloc.y + w + 2,
			col_fg, "%s", body->name);
}

void
view_main_draw(void) {
	Vector mousekm = pxtokm(mouse.vector);
	Vector ruler;
	Geom geom;
	Tree *t;
	Body *body;
	float dist;
	float x, y;

#ifdef DEBUG
	ui_print(screen.w / 2, VIEWS_HEIGHT + PAD, col_fg, "W: %f | H: %f", (float)screen.w, (float)screen.h);
	ui_print(screen.w / 2, VIEWS_HEIGHT + PAD * 2, col_fg, "Xoff: %f | Yoff: %f | km/px: %f",
			v->kmx, v->kmy, v->kmperpx);
	ui_print(screen.w / 2, VIEWS_HEIGHT + PAD * 3, col_fg, "X: %f | Y: %f",
			mousekm.x, mousekm.y);
#endif /* DEBUG */

	/* draw system bodies */
	for (t = v->sys->t->d; t; t = t->n) {
		body = t->data;
		body->pxloc = kmtopx(body->vector);
		draw_orbit(body);
	}
	for (t = v->sys->t->d; t; t = t->n) {
		body = t->data;
		draw_body(body);
	}

	/* ruler */
	if (v->ruler.held) {
		ruler = kmtopx(v->ruler.origin);
		ui_draw_line_v(ruler, mouse.vector, 1, col_info);
		dist = vector_dist(v->ruler.origin, mousekm);
		ui_print(mouse.x + PAD, mouse.y - PAD, col_info, "%s (%s)", strkm(dist), strly(dist));
	}

	/* scale */
	ui_draw_rect(v->scale.x,
			screen.h - v->scale.y,
			v->scale.w, 1, col_info); /* horizontal */
	ui_draw_rect(v->scale.x,
			screen.h - v->scale.y - v->scale.h,
			1, v->scale.h, col_info); /* left vertical */
	ui_draw_rect(v->scale.x + v->scale.w,
			screen.h - v->scale.y - v->scale.h,
			1, v->scale.h, col_info); /* right vertical */
	dist = v->scale.w * v->kmperpx;
	ui_print(v->scale.x + v->scale.w + FONT_SIZE / 3,
			screen.h - v->scale.y - FONT_SIZE / 2,
			col_info, "%s", strkm(dist));

	/* infobox */
	ui_draw_tabbed_window(EXPLODE_RECT(v->infobox.geom),
			&v->infobox.tabs);
	x = v->infobox.geom.x + FONT_SIZE;
	y = v->infobox.geom.y + WINDOW_TAB_HEIGHT;
	geom = RECT(x - FONT_SIZE,
			y,
			v->infobox.geom.w - WINDOW_BORDER,
			v->infobox.geom.h - WINDOW_TAB_HEIGHT - WINDOW_BORDER);
	v->infobox.pane.geom = &geom;

	pane_begin(&v->infobox.pane);
	gui_checkbox(x, y += FONT_SIZE*1.5, &v->infobox.names.dwarf);
	gui_checkbox(x, y += FONT_SIZE*1.5, &v->infobox.names.dwarfn);
	gui_checkbox(x, y += FONT_SIZE*1.5, &v->infobox.names.asteroid);
	gui_checkbox(x, y += FONT_SIZE*1.5, &v->infobox.names.asteroidn);
	gui_checkbox(x, y += FONT_SIZE*1.5, &v->infobox.names.comet);
	gui_checkbox(x, y += FONT_SIZE*1.5, &v->infobox.orbit.dwarf);
	gui_checkbox(x, y += FONT_SIZE*1.5, &v->infobox.orbit.asteroid);
	gui_checkbox(x, y += FONT_SIZE*1.5, &v->infobox.orbit.comet);
	gui_checkbox(x, y += FONT_SIZE*1.5, &v->infobox.comettail);
	pane_end();
}
