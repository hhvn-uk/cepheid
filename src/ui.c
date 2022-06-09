#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <raylib.h>
#include "main.h"

#define VIEWS_MAX_WIDTH (UI_VIEW_LAST*100)
#define VIEWS_HEIGHT 25
#define WINDOW_TAB_HEIGHT 20
#define TARGET_FPS 60
#define MIN_BODY_DIAM 3

static Font font;
static Clickable clickable[CLICKABLE_MAX];

void (*view_handlers[UI_VIEW_LAST])(void) = {
	[UI_VIEW_MAIN] = ui_handle_view_main,
	[UI_VIEW_COLONIES] = ui_handle_view_colonies,
	[UI_VIEW_FLEETS] = ui_handle_view_fleets,
	[UI_VIEW_DESIGN] = ui_handle_view_design,
	[UI_VIEW_SYSTEMS] = ui_handle_view_systems,
	[UI_VIEW_SETTINGS] = ui_handle_view_settings,
};

void (*view_drawers[UI_VIEW_LAST])(void) = {
	[UI_VIEW_MAIN] = ui_draw_view_main,
	[UI_VIEW_COLONIES] = ui_draw_view_colonies,
	[UI_VIEW_FLEETS] = ui_draw_view_fleets,
	[UI_VIEW_DESIGN] = ui_draw_view_design,
	[UI_VIEW_SYSTEMS] = ui_draw_view_systems,
	[UI_VIEW_SETTINGS] = ui_draw_view_settings,
};

Tabs view_tabs = {
	UI_VIEW_LAST, 0, {{"Main", 0}, {"Colonies", 0}, {"Fleets", 0}, {"Design", 0},
				{"Systems", 0}, {"Settings", 0}}
};

static struct {
	struct {
		Tabs tabs;
		struct {
			Checkbox dwarf;
			Checkbox asteroid;
			Checkbox comet;
		} names;
		struct {
			Checkbox dwarf;
			Checkbox asteroid;
			Checkbox comet;
		} orbit;
		Rect geom;
	} infobox;
	int pan;
	struct {
		int held;
		Vector2 origin;
	} ruler;
	float kmx, kmy;
	float kmperpx;
	struct {
		int x, y; /* real y = GetScreenHeight() - y */
		int w, h;
	} scale;
} view_main = {
	.infobox = {
		.tabs = {
			2, 0, {{"Display", 0}, {"Minerals", 0}}
		},
		.names = {
			.dwarf = {1, "Name: dwarf planets"},
			.asteroid = {0, "Name: asteroid"},
			.comet = {1, "Name: comet"},
		},
		.orbit = {
			.dwarf = {0, "Orbit: dwarf planets"},
			.asteroid = {0, "Orbit: asteroid"},
			.comet = {0, "Orbit: comet"},
		},
		.geom = {
			.x = 10,
			.y = VIEWS_HEIGHT + 10,
			.w = 200,
			.h = 400,
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
};

void
ui_init(void) {
	InitWindow(500, 500, "testing raylib");
	SetWindowState(FLAG_WINDOW_RESIZABLE);
	SetTargetFPS(TARGET_FPS);

	font = LoadFontFromMemory(".ttf",
			DejaVuSansMono_ttf, DejaVuSansMono_ttf_size,
			FONT_SIZE, NULL, 0);
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

int
ui_textsize(char *text) {
	return MeasureTextEx(font, text, FONT_SIZE, FONT_SIZE/10).x;
}

int
ui_collides(Rect rect, Vector2 point) {
	if (point.x >= rect.x && point.x <= rect.x + rect.w &&
			point.y >= rect.y && point.y <= rect.y + rect.h)
		return 1;
	else
		return 0;
}

void
ui_clickable_register(int x, int y, int w, int h, enum UiElements type, void *elem) {
	int i;

	for (i = 0; i < CLICKABLE_MAX; i++) {
		if (!clickable[i].elem) {
			clickable[i].geom.x = x;
			clickable[i].geom.y = y;
			clickable[i].geom.w = w;
			clickable[i].geom.h = h;
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
	int ftabw, fw, fn, tabw, x;
	int i;

	switch (clickable->type) {
	case UI_TAB:
		if (button != MOUSE_BUTTON_LEFT)
			return;
		tabs = clickable->elem;
		for (fw = clickable->geom.w, fn = i = 0; i < tabs->n; i++) {
			if (!tabs->tabs[i].w)
				fn++;
			else
				fw -= tabs->tabs[i].w;
		}
		ftabw = fw / fn;
		for (i = 0, x = clickable->geom.x; i < tabs->n; x += tabw, i++) {
			if (i == tabs->n - 1)
				tabw = clickable->geom.x + clickable->geom.w - x;
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

void
ui_clickable_update(void) {
	Vector2 mouse;
	MouseButton button;
	int i;

	mouse = GetMousePosition();
	/* I wish there was a: int GetMouseButton(void) */
	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) button = MOUSE_BUTTON_LEFT;
	else if (IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE)) button = MOUSE_BUTTON_MIDDLE;
	else if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) button = MOUSE_BUTTON_RIGHT;
	else button = -1;

	for (i = 0; i < CLICKABLE_MAX; i++) {
		if (clickable[i].elem && ui_collides(clickable[i].geom, mouse))
			ui_clickable_handle(mouse, button, &clickable[i]);
		clickable[i].elem = NULL;
	}
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

void
ui_draw_tabs(int x, int y, int w, int h, Tabs *tabs) {
	int fw, fn, ftabw;
	int tabw;
	int padx, pady;
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
		padx = (tabw - ui_textsize(tabs->tabs[i].name)) / 2;
		if (i == tabs->sel)
			selx = cx;
		else
			DrawRectangle(cx, y, tabw, h, COL_UNSELBG);
		ui_print(cx + padx, y + pady, COL_FG, "%s", tabs->tabs[i].name);
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

	ui_clickable_register(x, y, w, h, UI_TAB, tabs);
}

void
ui_draw_checkbox(int x, int y, Checkbox *box) {
	int w, h;

	w = h = FONT_SIZE;
	ui_draw_border(x, y, w, h, 1);
	DrawRectangle(x + 1, y + 1, w - 2, h - 2, box->val ? COL_FG : COL_BG);
	ui_print(x + w + (w / 2), y + (h / 6), COL_FG, "%s", box->label);
	ui_clickable_register(x, y, w + (w / 2) + ui_textsize(box->label), h, UI_CHECKBOX, box);
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

void
ui_handle_view_main(void) {
	Vector2 mouse = GetMousePosition();
	Vector2 delta = GetMouseDelta();
	float wheel;

#define SCROLL_DIVISOR 10
	wheel = GetMouseWheelMove();
	if (wheel < 0)
		view_main.kmperpx += (GetMouseWheelMove() * -1) * (view_main.kmperpx/SCROLL_DIVISOR);
	else if (wheel > 0)
		view_main.kmperpx -= GetMouseWheelMove() * (view_main.kmperpx/SCROLL_DIVISOR);
	if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
		if (view_main.pan) {
			view_main.kmx -= delta.x * view_main.kmperpx;
			view_main.kmy -= delta.y * view_main.kmperpx;
		} else if (!ui_collides(view_main.infobox.geom, mouse)) {
			view_main.pan = 1;
		}
	} else view_main.pan = 0;

	if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) && !view_main.ruler.held &&
			!ui_collides(view_main.infobox.geom, mouse)) {
		view_main.ruler.held = 1;
		view_main.ruler.origin = mouse;
	} else if (IsMouseButtonUp(MOUSE_BUTTON_RIGHT))
		view_main.ruler.held = 0;

}

void
ui_handle_view_colonies(void) {

}

void
ui_handle_view_fleets(void) {

}

void
ui_handle_view_design(void) {

}

void
ui_handle_view_systems(void) {

}

void
ui_handle_view_settings(void) {

}

int
ui_should_draw_body(Body *body, int orbit) {
	if (!orbit && body->type != BODY_COMET && body->dist / view_main.kmperpx < ui_textsize(body->name))
		return 0;
	if ((body->type == BODY_DWARF || (body->parent && body->parent->type == BODY_DWARF)) &&
			(!orbit || !view_main.infobox.orbit.dwarf.val) &&
			(orbit || !view_main.infobox.names.dwarf.val))
		return 0;
	if ((body->type == BODY_ASTEROID || (body->parent && body->parent->type == BODY_ASTEROID)) &&
			(!orbit || !view_main.infobox.orbit.asteroid.val) &&
			(orbit || !view_main.infobox.names.asteroid.val))
		return 0;
	if ((body->type == BODY_COMET || (body->parent && body->parent->type == BODY_COMET)) &&
			(!orbit || !view_main.infobox.orbit.comet.val) &&
			(orbit || !view_main.infobox.names.comet.val))
		return 0;
	return 1;
}

void
ui_draw_body(Body *body) {
	Vector2 parent;
	Vector2 location;
	int w;

	if (body->parent) {
		parent = ui_kmtopx(body->parent->vector);
	} else {
		parent.x = 0;
		parent.y = 0;
	}

	location = ui_kmtopx(body->vector);

	if (ui_should_draw_body(body, 1)) {
		if (body->parent && body->type == BODY_COMET)
			DrawLineV(parent, location, COL_ORBIT);
		else if (body->parent)
			DrawCircleLines(parent.x, parent.y, ui_vectordist(parent, location), COL_ORBIT);
	}
	if (body->radius / view_main.kmperpx > MIN_BODY_DIAM)
		w = body->radius / view_main.kmperpx;
	else
		w = MIN_BODY_DIAM;
	DrawCircle(location.x, location.y, MIN_BODY_DIAM, COL_FG);
	if (ui_should_draw_body(body, 0))
		ui_print(location.x + 5, location.y + 5, COL_FG, "%s", body->name);
}

void
ui_draw_view_main(void) {
	Vector2 mouse = GetMousePosition();
	Vector2 mousekm = ui_pxtokm(mouse);
	Vector2 v[2];
	Vector2 km;
	Polar polar;
	float dist;
	size_t i;

	/* debug info */
	ui_print(GetScreenWidth() / 2, VIEWS_HEIGHT + 10, COL_FG, "Xoff: %f | Yoff: %f | km/px: %f",
			view_main.kmx, view_main.kmy, view_main.kmperpx);
	ui_print(GetScreenWidth() / 2, VIEWS_HEIGHT + 20, COL_FG, "X: %f | Y: %f",
			mousekm.x, mousekm.y);
	ui_print(GetScreenWidth() / 2, VIEWS_HEIGHT + 30, COL_FG, "FPS: %d (target: %d)", GetFPS(), TARGET_FPS);

	/* system bodies */
	if (!save->cache.system)
		save->cache.system = system_load(NULL, "Sol");
	for (i = 0; i < save->cache.system->bodies_len; i++)
		ui_draw_body(save->cache.system->bodies[i]);

	/* ruler */
	if (view_main.ruler.held) {
		DrawLineV(view_main.ruler.origin, mouse, COL_INFO);
		km = ui_pxtokm(view_main.ruler.origin);
		dist = ui_vectordist(km, mousekm);
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
	ui_draw_tabbed_window(view_main.infobox.geom.x, view_main.infobox.geom.y,
			view_main.infobox.geom.w, view_main.infobox.geom.h,
			&view_main.infobox.tabs);
	ui_draw_checkbox(view_main.infobox.geom.x + FONT_SIZE,
			view_main.infobox.geom.y + WINDOW_TAB_HEIGHT + FONT_SIZE*1.5,
			&view_main.infobox.names.dwarf);
	ui_draw_checkbox(view_main.infobox.geom.x + FONT_SIZE,
			view_main.infobox.geom.y + WINDOW_TAB_HEIGHT + FONT_SIZE*3,
			&view_main.infobox.names.asteroid);
	ui_draw_checkbox(view_main.infobox.geom.x + FONT_SIZE,
			view_main.infobox.geom.y + WINDOW_TAB_HEIGHT + FONT_SIZE*4.5,
			&view_main.infobox.names.comet);
	ui_draw_checkbox(view_main.infobox.geom.x + FONT_SIZE,
			view_main.infobox.geom.y + WINDOW_TAB_HEIGHT + FONT_SIZE*6,
			&view_main.infobox.orbit.dwarf);
	ui_draw_checkbox(view_main.infobox.geom.x + FONT_SIZE,
			view_main.infobox.geom.y + WINDOW_TAB_HEIGHT + FONT_SIZE*7.5,
			&view_main.infobox.orbit.asteroid);
	ui_draw_checkbox(view_main.infobox.geom.x + FONT_SIZE,
			view_main.infobox.geom.y + WINDOW_TAB_HEIGHT + FONT_SIZE*9,
			&view_main.infobox.orbit.comet);
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
ui_draw_view_systems(void) {
	ui_print(10, GetScreenHeight() / 2, COL_FG, "System info/settings menu");
	ui_print(GetScreenWidth() / 2, GetScreenHeight() / 2, COL_FG, "Pannable system view");
}

void
ui_draw_view_settings(void) {
	ui_print(10, VIEWS_HEIGHT + 10, COL_FG, "Settings here");
}
