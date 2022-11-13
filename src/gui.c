#include "main.h"

static Clickable clickable[CLICKABLE_MAX];
static int clickablei = 0;

static void gui_click_tabs(Vector2 mouse,
		MouseButton button, Geom *geom, void *elem);
static void gui_click_checkbox(Vector2 mouse,
		MouseButton button, Geom *geom, void *elem);
static void gui_click_dropdown(Vector2 mouse,
		MouseButton button, Geom *geom, void *elem);
static void gui_click_input(Vector2 mouse,
		MouseButton button, Geom *geom, void *elem);

static void gui_key_input(void *elem, int *fcount);

static void (*click_handlers[GUI_ELEMS])(Vector2 mouse,
		MouseButton button,
		Geom *geom, void *elem) = {
	[GUI_TAB] = gui_click_tabs,
	[GUI_CHECKBOX] = gui_click_checkbox,
	[GUI_DROPDOWN] = gui_click_dropdown,
	[GUI_INPUT] = gui_click_input,
};

void (*gui_key_handlers[GUI_ELEMS])(void *elem, int *fcount) = {
	[GUI_TAB] = NULL,
	[GUI_CHECKBOX] = NULL,
	[GUI_DROPDOWN] = NULL,
	[GUI_INPUT] = gui_key_input,
};

static void
gui_click_register(Geom geom, enum GuiElements type, void *elem) {
	if (clickablei >= CLICKABLE_MAX)
		return; /* ran out */

	clickable[clickablei].geom = geom;
	clickable[clickablei].type = type;
	clickable[clickablei].elem = elem;
	clickablei++;
}

int
gui_click_handle(void) {
	Vector2 mouse;
	MouseButton button;
	Geom *geom;
	int i;
	int ret = 0;
	int keepfocus = 0;

	mouse = GetMousePosition();
	/* I wish there was a: int GetMouseButton(void) */
	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
		button = MOUSE_BUTTON_LEFT;
	else if (IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE))
		button = MOUSE_BUTTON_MIDDLE;
	else if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
		button = MOUSE_BUTTON_RIGHT;
	else
		button = -1;

	for (i = 0; i < clickablei; i++) {
		if (clickable[i].elem && ui_collides(clickable[i].geom, mouse)) {
			geom = &clickable[i].geom;
			click_handlers[clickable[i].type](mouse,
					button, geom, clickable[i].elem);
			if (clickable[i].elem == focus.p)
				keepfocus = 1;
			ret = 1;
		}
	}

	clickablei = 0;

	/* clicking outside the focused elememnt unfocuses */
	if (button != -1 && !keepfocus)
		ui_update_focus(0, NULL);

	return ret;
}

void
gui_tabs(int x, int y, int w, int h, Tabs *tabs) {
	int fw, fn, ftabw;
	int tabw;
	int padx, pady;
	int iw;
	int cx, selx = -1;
	int i;

	ui_draw_rectangle(x, y, w, h, col_bg);

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
			ui_draw_rectangle(cx, y, tabw, h, col_unselbg);
		ui_print(cx + padx + iw, y + pady, col_fg, "%s", tabs->tabs[i].name);
		if (tabs->tabs[i].icon)
			ui_draw_texture(*tabs->tabs[i].icon, cx + padx / 2,
					y + (h - tabs->tabs[i].icon->width) / 2);
		ui_draw_rectangle(cx + tabw - 1, y, 1, h, col_border);
	}

	if (tabs->sel != tabs->n - 1) {
		if (!tabs->tabs[i].w)
			tabw = ftabw;
		else
			tabw = w / tabs->n;
	}

	ui_draw_border(x, y, w, h, 1);
	if (selx != -1) ui_draw_rectangle(selx - 1, y + h - 1, tabw + 1, 1, col_bg); /* undraw bottom border */
	if (tabs->sel == 0) ui_draw_rectangle(x, y + 1, 1, h - 1, col_bg); /* undraw left border */
	if (tabs->sel == tabs->n - 1) ui_draw_rectangle(x + w - 1, y + 1, 1, h - 1, col_bg); /* undraw right border */

	gui_click_register(RECT(x, y, w, h), GUI_TAB, tabs);
}

static void
gui_click_tabs(Vector2 mouse, MouseButton button, Geom *geom, void *elem) {
	int ftabw, fw, fn;
	int tabw, x, i;
	Tabs *tabs = elem;

	if (button != MOUSE_BUTTON_LEFT)
		return;
	for (fw = geom->w, fn = i = 0; i < tabs->n; i++) {
		if (!tabs->tabs[i].w)
			fn++;
		else
			fw -= tabs->tabs[i].w;
	}
	ftabw = fw / fn;
	for (i = 0, x = geom->x; i < tabs->n; x += tabw, i++) {
		if (i == tabs->n - 1)
			tabw = geom->x + geom->w - x;
		else if (!tabs->tabs[i].w)
			tabw = ftabw;
		else
			tabw = tabs->tabs[i].w;
		if (mouse.x >= x && mouse.x <= x + tabw) {
			tabs->sel = i;
			return;
		}
	}
}

int
gui_checkbox(int x, int y, Checkbox *box) {
	int w, h;
	int rw;

	w = h = FONT_SIZE;
	ui_draw_border(x, y, w, h, 1);
	ui_draw_rectangle(x + 1, y + 1, w - 2, h - 2,
			box->enabled ? (box->val ? col_fg : col_bg) : col_border);
	ui_print(x + w + (w / 2), y + (h / 6), col_fg, "%s", box->label);
	rw = w + (w / 2) + ui_textsize(box->label);
	if (box->enabled)
		gui_click_register(RECT(x, y, rw, h),
				GUI_CHECKBOX, box);
	return rw;
}

static void
gui_click_checkbox(Vector2 mouse, MouseButton button, Geom *geom, void *elem) {
	Checkbox *checkbox = elem;

	if (button != MOUSE_BUTTON_LEFT)
		return;
	checkbox->val = !checkbox->val;
}

void
gui_dropdown(int x, int y, int w, Dropdown *d) {
	int h = FONT_SIZE;
	int fh, ph;
	int focused;
	int i;

	focused = focus.p == d;
	fh = h + (focused ? h * d->n : 0);
	ph = MIN(fh, (screen.h - x) * 0.75);

	ui_draw_border_around(x, y, w, ph, 1);

	d->rect = RECT(x, y, w, fh + TPY);
	d->pane.geom = &d->rect;
	pane_begin(&d->pane);

	if (d->sel != -1)
		ui_print(x + TPX, y + TPY, col_fg, "%s", d->str[d->sel]);
	else if (d->placeholder)
		ui_print(x + TPX, y + TPY, col_info, "%s", d->placeholder);

	if (focused) {
		ui_draw_rectangle(x, y + h, w, fh - h, col_unselbg);
		for (i = 0; i < d->n; i++) {
			ui_print(x + TPX, y + TPY + (i+1) * h, col_fg, "%s", d->str[i]);
		}
	}

	gui_click_register(d->rect, GUI_DROPDOWN, d);
	pane_end();
}

static void
gui_click_dropdown(Vector2 mouse, MouseButton button, Geom *geom, void *elem) {
	Dropdown *drop = elem;
	int i;

	if (button != MOUSE_BUTTON_LEFT)
		return;
	if (focus.p != drop) {
		ui_update_focus(GUI_DROPDOWN, drop);
	} else {
		i = (mouse.y - geom->y) / FONT_SIZE;
		if (i != 0 && i <= drop->n)
			drop->sel = i - 1;
		ui_update_focus(0, NULL);
	}
}

void
gui_input(int x, int y, int w, Input *in) {
	int h = FONT_SIZE;
	int focused = focus.p == in;
	int cw;

	/* Dirty hack: truncate str to length that fits */
	cw = w / charpx - 1;
	if (in->len > cw) {
		in->len = cw;
		in->wstr[cw] = '\0';
	}
	if (in->cur > in->len)
		in->cur = in->len;

	ui_draw_border_around(x, y, w, h, 1);
	ui_draw_rectangle(x, y, w, h, focused ? col_bg : col_unselbg);
	if (in->len)
		ui_print(x + TPX, y + TPY, col_fg, "%S", in->wstr);
	else if (!focused && in->placeholder)
		ui_print(x + TPX, y + TPY, col_info, "%s", in->placeholder);
	if (focused) {
		ui_draw_rectangle(x + TPX + charpx * in->cur, y + TPY, 1, FONT_SIZE, col_fg);
	}
	gui_click_register(RECT(x, y, w, h), GUI_INPUT, in);
}

static void
gui_click_input(Vector2 mouse, MouseButton button, Geom *geom, void *elem) {
	Input *input = elem;
	int i;

	if (button != MOUSE_BUTTON_LEFT)
		return;
	if (focus.p != input) {
		ui_update_focus(GUI_INPUT, input);
	} else {
		i = (mouse.x - TPX - geom->x + charpx / 2) / charpx;
		if (i < input->len)
			input->cur = i;
		else if (i > 0)
			input->cur = input->len;
	}
}

static void
gui_key_input(void *elem, int *fcount) {
	wchar_t c = GetCharPressed();
	Input *in = elem;

	if (IsKeyPressed(KEY_ENTER) && in->onenter) {
		wcstombs(in->str, in->wstr, INPUT_MAX);
		if (in->onenter(in))
			edittrunc(in->wstr, &in->len, &in->cur);
	} else if (ui_keyboard_check(KEY_BACKSPACE, fcount) && in->len && in->cur) {
		editrm(in->wstr, &in->len, &in->cur);
	} else if (ui_keyboard_check(KEY_LEFT, fcount) && in->cur) {
		in->cur--;
	} else if (ui_keyboard_check(KEY_RIGHT, fcount) && in->cur != in->len) {
		in->cur++;
	} else if (c && in->len < INPUT_MAX) {
		editins(in->wstr, &in->len, &in->cur, INPUT_MAX, c);
	}
}