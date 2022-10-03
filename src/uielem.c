#include "main.h"

static void ui_handle_tabs(Vector2 mouse,
		MouseButton button, Geom *geom, void *elem);
static void ui_handle_checkbox(Vector2 mouse,
		MouseButton button, Geom *geom, void *elem);
static void ui_handle_dropdown(Vector2 mouse,
		MouseButton button, Geom *geom, void *elem);
static void ui_handle_input(Vector2 mouse,
		MouseButton button, Geom *geom, void *elem);

void (*ui_elem_handlers[UI_ELEMS])(Vector2 mouse,
		MouseButton button,
		Geom *geom, void *elem) = {
	[UI_TAB] = ui_handle_tabs,
	[UI_CHECKBOX] = ui_handle_checkbox,
	[UI_DROPDOWN] = ui_handle_dropdown,
	[UI_INPUT] = ui_handle_input,
};

void
ui_tabs(int x, int y, int w, int h, Tabs *tabs) {
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

	ui_clickable_register(RECT(x, y, w, h), UI_TAB, tabs);
}

static void
ui_handle_tabs(Vector2 mouse, MouseButton button, Geom *geom, void *elem) {
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
ui_checkbox(int x, int y, Checkbox *box) {
	int w, h;
	int rw;

	w = h = FONT_SIZE;
	ui_draw_border(x, y, w, h, 1);
	ui_draw_rectangle(x + 1, y + 1, w - 2, h - 2,
			box->enabled ? (box->val ? col_fg : col_bg) : col_border);
	ui_print(x + w + (w / 2), y + (h / 6), col_fg, "%s", box->label);
	rw = w + (w / 2) + ui_textsize(box->label);
	if (box->enabled)
		ui_clickable_register(RECT(x, y, rw, h),
				UI_CHECKBOX, box);
	return rw;
}

static void
ui_handle_checkbox(Vector2 mouse, MouseButton button, Geom *geom, void *elem) {
	Checkbox *checkbox = elem;

	if (button != MOUSE_BUTTON_LEFT)
		return;
	checkbox->val = !checkbox->val;
}

void
ui_dropdown(int x, int y, int w, Dropdown *d) {
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

	ui_clickable_register(d->rect, UI_DROPDOWN, d);
	pane_end();
}

static void
ui_handle_dropdown(Vector2 mouse, MouseButton button, Geom *geom, void *elem) {
	Dropdown *drop = elem;
	int i;

	if (button != MOUSE_BUTTON_LEFT)
		return;
	if (focus.p != drop) {
		ui_update_focus(UI_DROPDOWN, drop);
	} else {
		i = (mouse.y - geom->y) / FONT_SIZE;
		if (i != 0 && i <= drop->n)
			drop->sel = i - 1;
		ui_update_focus(0, NULL);
	}
}

void
ui_input(int x, int y, int w, Input *in) {
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
	ui_clickable_register(RECT(x, y, w, h), UI_INPUT, in);
}

static void
ui_handle_input(Vector2 mouse, MouseButton button, Geom *geom, void *elem) {
	Input *input = elem;
	int i;

	if (button != MOUSE_BUTTON_LEFT)
		return;
	if (focus.p != input) {
		ui_update_focus(UI_INPUT, input);
	} else {
		i = (mouse.x - TPX - geom->x + charpx / 2) / charpx;
		if (i < input->len)
			input->cur = i;
		else if (i > 0)
			input->cur = input->len;
	}
}
