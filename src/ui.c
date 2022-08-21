#include <math.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <raylib.h>
#include "main.h"

#define VIEWS_MAX_WIDTH (UI_VIEW_LAST*100)
#define VIEWS_HEIGHT 25
#define WINDOW_TAB_HEIGHT 20
#define TARGET_FPS 60
#define EXPLODE_RECT(r) r.x, r.y, r.w, r.h
#define EXPLODE_CIRCLE(c) c.centre, c.r
#define RLIFY_RECT(r) ((Rectangle){ EXPLODE_RECT(r) })
#define GEOMIFY_RECT(r) ((Geom){UI_RECT, .rect = r})
#define GEOMIFY_CIRCLE(c) ((Geom){UI_CIRCLE, .circle = c})
#define GEOM_RECT(x, y, w, h) ((Geom){UI_RECT, .rect = {x, y, w, h}})
#define GEOM_CIRCLE(centre, r) ((Geom){UI_CIRCLE, .circle = {centre, r})

static Clickable clickable[CLICKABLE_MAX];

static float min_body_rad[] = {
	[BODY_STAR] = 4,
	[BODY_PLANET] = 3,
	[BODY_COMET] = 2.5,
	[BODY_DWARF] = 2,
	[BODY_ASTEROID] = 1,
	[BODY_MOON] = 1,
};

static Color body_col[] = {
	[BODY_STAR] = COL_STAR,
	[BODY_PLANET] = COL_PLANET,
	[BODY_COMET] = COL_COMET,
	[BODY_DWARF] = COL_DWARF,
	[BODY_ASTEROID] = COL_ASTEROID,
	[BODY_MOON] = COL_MOON,
};

/* Return 1 for redraw, 0 to keep prev */
int (*view_handlers[UI_VIEW_LAST])(int) = {
	[UI_VIEW_MAIN] = ui_handle_view_main,
	[UI_VIEW_COLONIES] = ui_handle_view_colonies,
	[UI_VIEW_FLEETS] = ui_handle_view_fleets,
	[UI_VIEW_DESIGN] = ui_handle_view_design,
	[UI_VIEW_SYSTEMS] = ui_handle_view_sys,
	[UI_VIEW_SETTINGS] = ui_handle_view_settings,
};

void (*view_drawers[UI_VIEW_LAST])(void) = {
	[UI_VIEW_MAIN] = ui_draw_view_main,
	[UI_VIEW_COLONIES] = ui_draw_view_colonies,
	[UI_VIEW_FLEETS] = ui_draw_view_fleets,
	[UI_VIEW_DESIGN] = ui_draw_view_design,
	[UI_VIEW_SYSTEMS] = ui_draw_view_sys,
	[UI_VIEW_SETTINGS] = ui_draw_view_settings,
};

Screen screen = { 0 };

Tabs view_tabs = {
	/* Tactical is the terminology used in Aurora, so I decided to use it
	 * in the ui; in the code it's just called 'main' for my fingers' sake */
	UI_VIEW_LAST, 0, {{&image_tactical, "Tactical", 0},
		{&image_colonies, "Colonies", 0},
		{&image_fleet, "Fleets", 0},
		{&image_design, "Design", 0},
		{&image_sys, "Systems", 0},
		{&image_settings, "Settings", 0}}
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
			.rect = {
				.x = 10,
				.y = VIEWS_HEIGHT + 10,
				.w = 200,
				.h = 400,
			},
		},
	},
	.pan = 0,
	.ruler = {.held = 0},
	.kmx = 0,
	.kmy = 0,
	.kmperpx = 500000,
	.scale = {
		.x = 10,
		.y = 10, /* from bottom */
		.w = 50,
		.h = 3,
	},
	.sys = NULL,
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

void
ui_init(void) {
	InitWindow(500, 500, "");
	SetWindowState(FLAG_WINDOW_RESIZABLE);
	SetTargetFPS(TARGET_FPS);
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
ui_deinit(void) {
	CloseWindow();
}

void
ui_print(int x, int y, Color col, char *fmt, ...) {
	va_list ap;
	Vector2 pos;
	char *text;

	pos.x = x;
	pos.y = y;
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
	return MeasureTextEx(font, text, FONT_SIZE, FONT_SIZE/10).x;
}

int
ui_collides(Geom geom, Vector2 point) {
	switch (geom.type) {
	case UI_CIRCLE:
		return CheckCollisionPointCircle(point,
				EXPLODE_CIRCLE(geom.circle));
	case UI_RECT: default: /* -Wreturn-type bitches without default */
		return CheckCollisionPointRec(point, RLIFY_RECT(geom.rect));
	}
}

int
ui_onscreen(Vector2 point) {
	return ui_collides(GEOMIFY_RECT(screen.rect), point);
}

int
ui_ring_rect_collides(Vector2 centre, float r, Rect rect) {
	Vector2 rcent = {rect.x + rect.w / 2, rect.y + rect.h / 2};
	Vector2 d;

	d.x = fabsf(centre.x - rcent.x);
	d.y = fabsf(centre.y - rcent.y);

	if (d.x > (rect.w / 2 + r)) return 0;
	if (d.y > (rect.h / 2 + r)) return 0;

	if (d.x <= (rect.w / 2)) return 1;
	if (d.y <= (rect.h / 2)) return 1;

	return ((d.x - rect.w / 2) * (d.x - rect.w / 2) +
			(d.y - rect.h / 2) * (d.y - rect.h / 2)) <= r * r;
}

int
ui_onscreen_ring(Vector2 centre, float r) {
	float d = ui_vectordist(centre, screen.centre);

	if (d + screen.diag / 2 < r)
		return 0;
	return CheckCollisionCircleRec(centre, r, RLIFY_RECT(screen.rect));
}

int
ui_onscreen_circle(Vector2 centre, float r) {
	return CheckCollisionCircleRec(centre, r, RLIFY_RECT(screen.rect));
}

void
ui_clickable_register(Geom geom, enum UiElements type, void *elem) {
	int i;

	for (i = 0; i < CLICKABLE_MAX; i++) {
		if (!clickable[i].elem) {
			clickable[i].geom.type = geom.type;
			switch (geom.type) {
			case UI_RECT: clickable[i].geom.rect = geom.rect; break;
			case UI_CIRCLE: clickable[i].geom.circle = geom.circle; break;
			}
			clickable[i].type = type;
			clickable[i].elem = elem;
			return;
		}
	}

	/* welp, we ran out */
}

void
ui_clickable_handle(Vector2 mouse, MouseButton button, Clickable *clickable) {
	Tabs *tabs;
	Checkbox *checkbox;
	Rect *rect;
	/* Circle *circle; */
	int ftabw, fw, fn, tabw, x;
	int i;

	rect = &clickable->geom.rect;
	/* circle = &clickable->geom.circle; */

	switch (clickable->type) {
	case UI_TAB:
		if (button != MOUSE_BUTTON_LEFT)
			return;
		tabs = clickable->elem;
		for (fw = rect->w, fn = i = 0; i < tabs->n; i++) {
			if (!tabs->tabs[i].w)
				fn++;
			else
				fw -= tabs->tabs[i].w;
		}
		ftabw = fw / fn;
		for (i = 0, x = rect->x; i < tabs->n; x += tabw, i++) {
			if (i == tabs->n - 1)
				tabw = rect->x + rect->w - x;
			else if (!tabs->tabs[i].w)
				tabw = ftabw;
			else
				tabw = tabs->tabs[i].w;
			if (mouse.x >= x && mouse.x <= x + tabw) {
				tabs->sel = i;
				return;
			}
		}
		break;
	case UI_CHECKBOX:
		if (button != MOUSE_BUTTON_LEFT)
			return;
		checkbox = clickable->elem;
		checkbox->val = !checkbox->val;
		break;
	}
}

int
ui_clickable_update(void) {
	Vector2 mouse;
	MouseButton button;
	int i;
	int ret = 0;

	mouse = GetMousePosition();
	/* I wish there was a: int GetMouseButton(void) */
	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) button = MOUSE_BUTTON_LEFT;
	else if (IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE)) button = MOUSE_BUTTON_MIDDLE;
	else if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) button = MOUSE_BUTTON_RIGHT;
	else button = -1;

	for (i = 0; i < CLICKABLE_MAX; i++) {
		if (clickable[i].elem && ui_collides(clickable[i].geom, mouse)) {
			ui_clickable_handle(mouse, button, &clickable[i]);
			ret = 1;
		}
	}

	/* Handle bodies seperately for efficiency:
	 * - body->pxloc can be used instead of a geometry passed to
	 *   ui_clickable_register()
	 * - bodies offscreen can't be clicked, so they can be skipped.
	 */

	return ret;
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
	ui_draw_tabs(0, 0, sw, VIEWS_HEIGHT, &view_tabs);
}

void
ui_draw_border(int x, int y, int w, int h, int px) {
	DrawRectangle(x, y, w, px, COL_BORDER); /* top */
	DrawRectangle(x, y, px, h, COL_BORDER); /* left */
	DrawRectangle(x, y + h - px, w, px, COL_BORDER); /* bottom */
	DrawRectangle(x + w - px, y, px, h, COL_BORDER); /* right */
}

#define SEGMAX 2500

void
ui_draw_ring(int x, int y, float r, Color col) {
	Vector2 v = {x, y};
	Polar p;
	float s;
	float prec = screen.diag * 2 / (PI * 2 * r) * 360;
	float sdeg = 0, edeg = 360;
	float deg;

	if (!ui_onscreen_ring(v, r))
		return;

	p = sys_polarize_around(v, screen.centre);
	deg = p.theta;

	/* Draw the section of the ring (+ wriggle room) that will be onscreen
	 * be (start/end)ing prec degrees before/after the screen's centre
	 * relative to the ring's centre. */
	sdeg = deg + prec;
	edeg = deg - prec;

	if (r < 100)
		s = r;
	else if (r < SEGMAX)
		s = r / log10(r);
	else
		s = SEGMAX;

	/* Less segments are needed if there is less ring to place them in. */
	s *= (sdeg - edeg) / 360;

	DrawRing(v, r - 1, r, sdeg, edeg, s, col);
}

void
ui_draw_tabs(int x, int y, int w, int h, Tabs *tabs) {
	int fw, fn, ftabw;
	int tabw;
	int padx, pady;
	int iw;
	int cx, selx = -1;
	int i;

	DrawRectangle(x, y, w, h, COL_BG);

	for (fw = w, fn = i = 0; i < tabs->n; i++) {
		if (!tabs->tabs[i].w)
			fn++;
		else
			fw -= tabs->tabs[i].w;
	}

	ftabw = fw / fn;
	pady = (h - FONT_SIZE) / 2;

	for (i = 0, cx = x; i < tabs->n; i++, cx += tabw) {
		if (i == tabs->n - 1)
			tabw = x + w - cx;
		else if (!tabs->tabs[i].w)
			tabw = ftabw;
		else
			tabw = tabs->tabs[i].w;

		if (tabs->tabs[i].icon)
			iw = tabs->tabs[i].icon->width;
		else
			iw = 0;

		padx = (tabw - ui_textsize(tabs->tabs[i].name) - iw) / 2;
		if (i == tabs->sel)
			selx = cx;
		else
			DrawRectangle(cx, y, tabw, h, COL_UNSELBG);
		ui_print(cx + padx + iw, y + pady, COL_FG, "%s", tabs->tabs[i].name);
		if (tabs->tabs[i].icon)
			DrawTexture(*tabs->tabs[i].icon, cx + padx / 2,
					y + (h - tabs->tabs[i].icon->width) / 2,
					COL_FG);
		DrawRectangle(cx + tabw - 1, y, 1, h, COL_BORDER);
	}

	if (tabs->sel != tabs->n - 1) {
		if (!tabs->tabs[i].w)
			tabw = ftabw;
		else
			tabw = w / tabs->n;
	}

	ui_draw_border(x, y, w, h, 1);
	if (selx != -1) DrawRectangle(selx - 1, y + h - 1, tabw + 1, 1, COL_BG); /* undraw bottom border */
	if (tabs->sel == 0) DrawRectangle(x, y + 1, 1, h - 1, COL_BG); /* undraw left border */
	if (tabs->sel == tabs->n - 1) DrawRectangle(x + w - 1, y + 1, 1, h - 1, COL_BG); /* undraw right border */

	ui_clickable_register(GEOM_RECT(x, y, w, h), UI_TAB, tabs);
}

void
ui_draw_checkbox(int x, int y, Checkbox *box) {
	int w, h;

	w = h = FONT_SIZE;
	ui_draw_border(x, y, w, h, 1);
	DrawRectangle(x + 1, y + 1, w - 2, h - 2,
			box->enabled ? (box->val ? COL_FG : COL_BG) : COL_BORDER);
	ui_print(x + w + (w / 2), y + (h / 6), COL_FG, "%s", box->label);
	if (box->enabled)
		ui_clickable_register(GEOM_RECT(x, y,
					w + (w / 2) + ui_textsize(box->label), h),
				UI_CHECKBOX, box);
}

Vector2
ui_kmtopx(Vector2 km) {
	return (Vector2) {
		(GetScreenWidth() / 2) + (km.x - view_main.kmx) / view_main.kmperpx,
		(GetScreenHeight() / 2) + (km.y - view_main.kmy) / view_main.kmperpx
	};
}

Vector2
ui_pxtokm(Vector2 vector) {
	return (Vector2) {
		((vector.x - GetScreenWidth() / 2) * view_main.kmperpx) + view_main.kmx,
		((vector.y - GetScreenHeight() / 2) * view_main.kmperpx) + view_main.kmy
	};
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
	DrawRectangle(x, y, w, h, COL_BG);
	ui_draw_tabs(x, y, w, WINDOW_TAB_HEIGHT, tabs);
	ui_draw_border(x, y, w, h, 2);
}

int
ui_handle_view_main(int nowsel) {
	Vector2 mouse = GetMousePosition();
	Vector2 delta = GetMouseDelta();
	float wheel = GetMouseWheelMove();
	float diff;
	Body *furth;

	if (view_main.sys)
		furth = view_main.sys->furthest_body;

#define SCROLL_DIVISOR 10
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

	if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) && !view_main.ruler.held &&
			!ui_collides(view_main.infobox.geom, mouse)) {
		view_main.ruler.held = 1;
		view_main.ruler.origin = ui_pxtokm(mouse);
	} else if (IsMouseButtonUp(MOUSE_BUTTON_RIGHT)) {
		view_main.ruler.held = 0;
	}

	if (!view_main.sys) {
		view_main.sys = sys_default();
	}

	if (nowsel)
		ui_title("Tactical: %s", view_main.sys->name);

	view_main.infobox.names.dwarfn.enabled = view_main.infobox.names.dwarf.val;
	view_main.infobox.names.asteroidn.enabled = view_main.infobox.names.asteroid.val;

	/* so that the debug stuff prints */
	if (delta.x || delta.y)
		return 1;

	return 0;
}

int
ui_handle_view_colonies(int nowsel) {
	if (nowsel)
		ui_title("Colonies");
	return 1;
}

int
ui_handle_view_fleets(int nowsel) {
	if (nowsel)
		ui_title("Fleets");
	return 1;
}

int
ui_handle_view_design(int nowsel) {
	if (nowsel)
		ui_title("Design");
	return 1;
}

int
ui_handle_view_sys(int nowsel) {
	if (nowsel)
		ui_title("Systems");
	view_sys.info.geom.h = GetScreenHeight() - VIEWS_HEIGHT;
	if (!view_sys.sel)
		view_sys.sel = sys_default();
	return 1;
}

int
ui_handle_view_settings(int nowsel) {
	if (nowsel)
		ui_title("Settings");
	return 1;
}

static int
ui_should_draw_body_checkbox(Body *body, int type, Checkbox *box) {
	if ((body->type == type || (body->parent &&
					body->parent->type == type)) &&
			!box->val)
		return 0;
	return 1;
}

int
ui_should_draw_body(Body *body, int orbit) {
	if (orbit) {
		if (!body->parent)
			return 0;
		if (orbit < min_body_rad[body->parent->type])
			return 0;
		if (!ui_should_draw_body_checkbox(body, BODY_DWARF,
					&view_main.infobox.orbit.dwarf))
			return 0;
		if (!ui_should_draw_body_checkbox(body, BODY_ASTEROID,
					&view_main.infobox.orbit.asteroid))
			return 0;
		if (!ui_should_draw_body_checkbox(body, BODY_COMET,
					&view_main.infobox.orbit.comet))
			return 0;
	} else {
		if (!ui_onscreen(body->pxloc))
			return 0;
		if (body->type != BODY_STAR &&
				(body->type == BODY_COMET ? body->maxdist : body->dist)
				/ view_main.kmperpx < ui_textsize(body->name))
			return 0;
		if (body->parent && body->type != BODY_STAR &&
				ui_vectordist(body->vector, body->parent->vector) <
				min_body_rad[body->type] * view_main.kmperpx)
			return 0;
		if (isdigit(*body->name) || *body->name == '(') {
			if (!ui_should_draw_body_checkbox(body, BODY_DWARF,
						&view_main.infobox.names.dwarfn))
				return 0;
			if (!ui_should_draw_body_checkbox(body, BODY_ASTEROID,
						&view_main.infobox.names.asteroidn))
				return 0;
		}
		if (!ui_should_draw_body_checkbox(body, BODY_DWARF,
					&view_main.infobox.names.dwarf))
			return 0;
		if (!ui_should_draw_body_checkbox(body, BODY_ASTEROID,
					&view_main.infobox.names.asteroid))
			return 0;
		if (!ui_should_draw_body_checkbox(body, BODY_COMET,
					&view_main.infobox.names.comet))
			return 0;
	}
	return 1;
}

void
ui_draw_body(Body *body) {
	Vector2 parent;
	float pxrad;
	int w;

	if (body->parent) {
		parent = ui_kmtopx(body->parent->vector);
	} else {
		parent.x = 0;
		parent.y = 0;
	}

	if (body->parent) {
		pxrad = ui_vectordist(parent, body->pxloc);
		if (ui_should_draw_body(body, pxrad)) {
			if (body->type == BODY_COMET)
				DrawLineV(parent, body->pxloc, COL_ORBIT);
			else
				ui_draw_ring(parent.x, parent.y, pxrad, COL_ORBIT);
		}
	}
	if (body->radius / view_main.kmperpx > min_body_rad[body->type])
		w = body->radius / view_main.kmperpx;
	else
		w = min_body_rad[body->type];
	DrawCircle(body->pxloc.x, body->pxloc.y, w, body_col[body->type]);
	if (body->type == BODY_COMET && view_main.infobox.comettail.val &&
			10 * view_main.kmperpx < body->curdist)
		DrawLineEx(body->pxloc, sys_vectorize_around(body->pxloc,
				(Polar){w * 11 / min_body_rad[BODY_COMET],
				body->inward ? body->theta : body->theta + 180}),
				w / min_body_rad[BODY_COMET], COL_COMET);
	if (ui_should_draw_body(body, 0))
		ui_print(body->pxloc.x + w + 2, body->pxloc.y + w + 2,
				COL_FG, "%s", body->name);
}

void
ui_draw_view_main(void) {
	Vector2 mouse = GetMousePosition();
	Vector2 mousekm = ui_pxtokm(mouse);
	Vector2 ruler;
	Body *body;
	float dist;
	size_t i;
	float x, y;

	/* debug info */
	ui_print(GetScreenWidth() / 2, VIEWS_HEIGHT + 10, COL_FG, "W: %f | H: %f", (float)screen.w, (float)screen.h);
	ui_print(GetScreenWidth() / 2, VIEWS_HEIGHT + 20, COL_FG, "Xoff: %f | Yoff: %f | km/px: %f",
			view_main.kmx, view_main.kmy, view_main.kmperpx);
	ui_print(GetScreenWidth() / 2, VIEWS_HEIGHT + 30, COL_FG, "X: %f | Y: %f",
			mousekm.x, mousekm.y);
	ui_print(GetScreenWidth() / 2, VIEWS_HEIGHT + 40, COL_FG, "FPS: %d (target: %d)", GetFPS(), TARGET_FPS);

	/* draw system bodies */
	for (i = 0; i < view_main.sys->bodies_len; i++) {
		body = view_main.sys->bodies[i];
		body->pxloc = ui_kmtopx(body->vector);
		ui_draw_body(body);
	}

	/* ruler */
	if (view_main.ruler.held) {
		ruler = ui_kmtopx(view_main.ruler.origin);
		DrawLineV(ruler, mouse, COL_INFO);
		dist = ui_vectordist(view_main.ruler.origin, mousekm);
		ui_print(mouse.x + 10, mouse.y - 10, COL_INFO, "%s (%s)", strkmdist(dist), strlightdist(dist));
	}

	/* scale */
	DrawRectangle(view_main.scale.x,
			GetScreenHeight() - view_main.scale.y,
			view_main.scale.w, 1, COL_INFO); /* horizontal */
	DrawRectangle(view_main.scale.x,
			GetScreenHeight() - view_main.scale.y - view_main.scale.h,
			1, view_main.scale.h, COL_INFO); /* left vertical */
	DrawRectangle(view_main.scale.x + view_main.scale.w,
			GetScreenHeight() - view_main.scale.y - view_main.scale.h,
			1, view_main.scale.h, COL_INFO); /* right vertical */
	dist = view_main.scale.w * view_main.kmperpx;
	ui_print(view_main.scale.x + view_main.scale.w + FONT_SIZE / 3,
			GetScreenHeight() - view_main.scale.y - FONT_SIZE / 2,
			COL_INFO, "%s", strkmdist(dist));

	/* infobox */
	ui_draw_tabbed_window(EXPLODE_RECT(view_main.infobox.geom.rect),
			&view_main.infobox.tabs);
	x = view_main.infobox.geom.rect.x + FONT_SIZE;
	y = view_main.infobox.geom.rect.y + WINDOW_TAB_HEIGHT;
	ui_draw_checkbox(x, y += FONT_SIZE*1.5, &view_main.infobox.names.dwarf);
	ui_draw_checkbox(x, y += FONT_SIZE*1.5, &view_main.infobox.names.dwarfn);
	ui_draw_checkbox(x, y += FONT_SIZE*1.5, &view_main.infobox.names.asteroid);
	ui_draw_checkbox(x, y += FONT_SIZE*1.5, &view_main.infobox.names.asteroidn);
	ui_draw_checkbox(x, y += FONT_SIZE*1.5, &view_main.infobox.names.comet);
	ui_draw_checkbox(x, y += FONT_SIZE*1.5, &view_main.infobox.orbit.dwarf);
	ui_draw_checkbox(x, y += FONT_SIZE*1.5, &view_main.infobox.orbit.asteroid);
	ui_draw_checkbox(x, y += FONT_SIZE*1.5, &view_main.infobox.orbit.comet);
	ui_draw_checkbox(x, y += FONT_SIZE*1.5, &view_main.infobox.comettail);
}

void
ui_draw_view_colonies(void) {
	ui_print(10, VIEWS_HEIGHT + 10, COL_FG, "Stars/colonies here");
	ui_print(GetScreenWidth() / 2, VIEWS_HEIGHT + 10, COL_FG, "Tabs here");
	ui_print(GetScreenWidth() / 2, GetScreenHeight() / 2, COL_FG, "Management stuff here");
}

void
ui_draw_view_fleets(void) {
	ui_print(10, VIEWS_HEIGHT + 10, COL_FG, "Groups/fleets/subfleets/ships here");
	ui_print(GetScreenWidth() / 2, VIEWS_HEIGHT + 10, COL_FG, "Tabs here");
	ui_print(GetScreenWidth() / 2, GetScreenHeight() / 2, COL_FG, "Management stuff here");
}

void
ui_draw_view_design(void) {
	ui_print(10, VIEWS_HEIGHT + 10, COL_FG, "Designations/classes here");
	ui_print(GetScreenWidth() / 4, VIEWS_HEIGHT + 10, COL_FG, "Selectable components here");
	ui_print((GetScreenWidth() / 4) * 2, VIEWS_HEIGHT + 10, COL_FG, "Selected components");
	ui_print((GetScreenWidth() / 4) * 3, VIEWS_HEIGHT + 10, COL_FG, "Class info");
}

void
ui_draw_view_sys(void) {
	int x, y;

	x = view_sys.info.geom.x + view_sys.info.geom.w;
	y = view_sys.info.geom.y;

	/* draw map */

	/* draw divider */
	DrawLine(x, y, x, y + view_sys.info.geom.h, COL_BORDER);

	/* draw info */
	x = view_sys.info.geom.x + 10;
	y += 10;
	if (view_sys.sel) {
		ui_print(x, y, COL_FG, "%s", view_sys.sel->name);
		DrawLine(x, y + FONT_SIZE, x + view_sys.info.geom.w - 20,
					y + FONT_SIZE, COL_BORDER);
		y += 30;
		ui_print(x, y,      COL_FG, "Stars:     %d", view_sys.sel->num.stars);
		ui_print(x, y + 10, COL_FG, "Planets:   %d", view_sys.sel->num.planets);
		ui_print(x, y + 20, COL_FG, "Asteroids: %d", view_sys.sel->num.asteroids);
		ui_print(x, y + 30, COL_FG, "Comets:    %d", view_sys.sel->num.comets);
		ui_print(x, y + 40, COL_FG, "Moons:     %d", view_sys.sel->num.moons);
		DrawLine(x, y + 52, x + 85, y + 52, COL_FG);
		ui_print(x, y + 55, COL_FG, "Total:     %d", view_sys.sel->bodies_len);
	}
}

void
ui_draw_view_settings(void) {
	ui_print(10, VIEWS_HEIGHT + 10, COL_FG, "Settings here");
}
