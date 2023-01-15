#include <stdlib.h>
#include <time.h>
#include "maths.h"
#include "main.h"

#define DOUBLE_CLICK (500.0 * MILLI / NANO)

static Clickable clickable[CLICKABLE_MAX];
static int clickablei = 0;

static struct {
	MouseButton button;
	Vector pos;
	struct timespec tv;
} lastclick = {-1};

/* Mouse handlers */
static void gui_mouse_tabs(MouseButton button, Geom *geom, void *elem);
static void gui_mouse_checkbox(MouseButton button, Geom *geom, void *elem);
static void gui_mouse_button(MouseButton button, Geom *geom, void *elem);
static void gui_mouse_input(MouseButton button, Geom *geom, void *elem);
static void gui_mouse_dropdown(MouseButton button, Geom *geom, void *elem);
static void gui_mouse_treeview(MouseButton button, Geom *geom, void *elem);

/* Key handlers */
static void gui_key_input(void *elem, int *fcount);

/* Other */
static int gui_double_click(void);
static void gui_enter_input(Input *in);
static int gui_form_sub_end(int x, int y, int w);

static struct {
	void (*mouse)(MouseButton button, Geom *geom, void *elem);
	MouseCursor cursor; /* Set this before running the mouse handler, 
			       as that can override it */
	void (*key)(void *elem, int *fcount);
} element[] = {
	[GUI_TAB] 	= {gui_mouse_tabs,	MOUSE_CURSOR_POINTING_HAND,	NULL},
	[GUI_CHECKBOX]	= {gui_mouse_checkbox,	MOUSE_CURSOR_POINTING_HAND,	NULL},
	[GUI_BUTTON]	= {gui_mouse_button,	MOUSE_CURSOR_POINTING_HAND,	NULL},
	[GUI_INPUT]	= {gui_mouse_input,	MOUSE_CURSOR_IBEAM, 		gui_key_input},
	[GUI_FORM]	= {NULL,		MOUSE_CURSOR_DEFAULT,		NULL}, /* Not registered */
	[GUI_DROPDOWN]	= {gui_mouse_dropdown,	MOUSE_CURSOR_POINTING_HAND,	NULL},
	[GUI_TREEVIEW]	= {gui_mouse_treeview,	MOUSE_CURSOR_DEFAULT, 		NULL},
};

static void
gui_mouse_register(Geom geom, enum GuiElements type, void *elem) {
	if (clickablei >= CLICKABLE_MAX)
		return; /* ran out */

	clickable[clickablei].geom = geom;
	clickable[clickablei].type = type;
	clickable[clickablei].elem = elem;
	clickablei++;
}

int
gui_mouse_handle(void) {
	MouseButton button;
	Geom *geom;
	int i;
	int ret = 0;
	int keepfocus = 0;

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
		if (clickable[i].elem && ui_collides(clickable[i].geom, mouse.vector)) {
			ui_cursor(element[clickable[i].type].cursor);
			geom = &clickable[i].geom;
			element[clickable[i].type].mouse(button, geom, clickable[i].elem);
			if (clickable[i].elem == focus.p)
				keepfocus = 1;
			ret = 1;
		}
	}

	clickablei = 0;

	/* clicking outside the focused elememnt unfocuses */
	if (button != -1 && !keepfocus)
		ui_focus(0, NULL);

	if (button != -1) {
		lastclick.pos = mouse.vector;
		lastclick.button = button;
		clock_gettime(CLOCK_REALTIME, &lastclick.tv);
	}

	if (!ret)
		ui_cursor(MOUSE_CURSOR_DEFAULT);

	return ret;
}

static int
gui_double_click(void) {
	struct timespec ctv;
	struct timespec diff;

	if (lastclick.button != MOUSE_BUTTON_LEFT ||
			lastclick.pos.x != mouse.x ||
			lastclick.pos.y != mouse.y)
		return 0;

	clock_gettime(CLOCK_REALTIME, &ctv);
	timespec_diff(&ctv, &lastclick.tv, &diff);

	if (diff.tv_sec == 0 && diff.tv_nsec < DOUBLE_CLICK)
		return 1;

	return 0;
}

static int
gui_key_check(int key, int *fcount) {
	if (IsKeyPressed(key)) {
		*fcount = -10;
		return 1;
	} else if (IsKeyDown(key) && !*fcount) {
		return 1;
	} else {
		return 0;
	}
}

void
gui_key_handle(void) {
	static int fcount = 0;

	/* Register multiple backspaces when held. raylib does this with
	 * "characters", but anything other than a "character" has completely
	 * different handling. (Newlines and backspaces *ARE* ASCII characters.
	 * So... eugh). ncurses's get_blah23_ch_fgfgj is somehow better.
	 *
	 * This also IS NOT ABLE TO respect the X11 settings for outputting
	 * characters while held because... Fuck you, apparently. Thanks
	 * raylib. */
	fcount++;
	if (fcount == (int)TARGET_FPS/15)
		fcount = 0;

	if (focus.p && element[focus.type].key)
		element[focus.type].key(focus.p, &fcount);
}

void
gui_tabs(int x, int y, int w, int h, Tabs *tabs) {
	int fw, fn, ftabw;
	int tabw;
	int padx, pady;
	int iw;
	int cx, selx = -1;
	int i;

	ui_draw_rect(x, y, w, h, col_bg);

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
			ui_draw_rect(cx, y, tabw, h, col_altbg);
		if (tabs->tabs[i].name)
			ui_print(cx + padx + iw, y + pady, col_fg, "%s", tabs->tabs[i].name);
		if (tabs->tabs[i].icon)
			ui_draw_texture(*tabs->tabs[i].icon, cx + (tabs->tabs[i].name ? padx / 2 : padx),
					y + (h - tabs->tabs[i].icon->width) / 2);
		ui_draw_rect(cx + tabw - 1, y, 1, h, col_border);
	}

	if (tabs->sel != tabs->n - 1) {
		if (!tabs->tabs[i].w)
			tabw = ftabw;
		else
			tabw = w / tabs->n;
	}

	ui_draw_border(x, y, w, h, 1);
	if (selx != -1) ui_draw_rect(selx - 1, y + h - 1, tabw + 1, 1, col_bg); /* undraw bottom border */
	if (tabs->sel == 0) ui_draw_rect(x, y + 1, 1, h - 1, col_bg); /* undraw left border */
	if (tabs->sel == tabs->n - 1) ui_draw_rect(x + w - 1, y + 1, 1, h - 1, col_bg); /* undraw right border */

	gui_mouse_register(RECT(x, y, w, h), GUI_TAB, tabs);
}

static void
gui_mouse_tabs(MouseButton button, Geom *geom, void *elem) {
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
	ui_draw_rect(x + 1, y + 1, w - 2, h - 2,
			box->enabled ? (box->val ? col_fg : col_bg) : col_border);
	ui_print(x + w + (w / 2), y + (h / 6), col_fg, "%s", box->label);
	rw = w + (w / 2) + ui_textsize(box->label);
	if (box->enabled)
		gui_mouse_register(RECT(x, y, rw, h),
				GUI_CHECKBOX, box);
	return rw;
}

static void
gui_mouse_checkbox(MouseButton button, Geom *geom, void *elem) {
	Checkbox *checkbox = elem;

	if (button != MOUSE_BUTTON_LEFT)
		return;
	if (!checkbox->def)
		checkbox->def = CHECKBOX_DEFAULT_OFF + checkbox->val;
	checkbox->val = !checkbox->val;
}

void
gui_button(int x, int y, int w, Button *b) {
	int h = BUTTON_HEIGHT;

	ui_draw_border_around(x, y, w, h, 1);
	ui_print(x + (w - ui_textsize(b->label)) / 2, y + PAD/2,
			b->enabled ? col_fg : col_altfg,
			"%s", b->label);

	if (b->enabled)
		gui_mouse_register(RECT(x, y, w, h), GUI_BUTTON, b);
}

static void
gui_mouse_button(MouseButton button, Geom *geom, void *elem) {
	Button *b = elem;

	if (button == MOUSE_BUTTON_LEFT) {
		if (b->submit)
			b->func(GUI_FORM, b->submit);
		else
			b->func(GUI_BUTTON, b);
	}
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
		ui_draw_rect(x, y + h, w, fh - h, col_altbg);
		for (i = 0; i < d->n; i++) {
			ui_print(x + TPX, y + TPY + (i+1) * h, col_fg, "%s", d->str[i]);
		}
	}

	gui_mouse_register(d->rect, GUI_DROPDOWN, d);
	pane_end();
}

static void
gui_mouse_dropdown(MouseButton button, Geom *geom, void *elem) {
	Dropdown *drop = elem;
	int i;

	if (button != MOUSE_BUTTON_LEFT)
		return;
	if (!drop->def)
		drop->def = DROPDOWN_DEFAULT_OFF + drop->sel;
	if (focus.p != drop) {
		ui_focus(GUI_DROPDOWN, drop);
	} else {
		i = (mouse.y - geom->y) / FONT_SIZE;
		if (i != 0 && i <= drop->n)
			drop->sel = i - 1;
		ui_focus(0, NULL);
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
	ui_draw_rect(x, y, w, h, focused ? col_bg : col_altbg);
	if (in->len)
		ui_print(x + TPX, y + TPY, col_fg, "%S", in->wstr);
	else if (!focused && in->placeholder)
		ui_print(x + TPX, y + TPY, col_info, "%s", in->placeholder);
	if (focused) {
		ui_draw_rect(x + TPX + charpx * in->cur, y + TPY, 1, FONT_SIZE - 2, col_fg);
	}
	gui_mouse_register(RECT(x, y, w, h), GUI_INPUT, in);
}

static void
gui_mouse_input(MouseButton button, Geom *geom, void *elem) {
	Input *input = elem;
	int i;

	if (button != MOUSE_BUTTON_LEFT)
		return;
	if (focus.p != input) {
		ui_focus(GUI_INPUT, input);
	} else {
		i = (mouse.x - TPX - geom->x + charpx / 2) / charpx;
		if (i < input->len)
			input->cur = i;
		else if (i > 0)
			input->cur = input->len;
	}
}

/* When enter is pressed in input, or when button submits the input. */
static void
gui_enter_input(Input *in) {
	if (in->onenter(GUI_INPUT, in))
		edittrunc(in->wstr, &in->len, &in->cur);
}

static void
gui_key_input(void *elem, int *fcount) {
	wchar_t c = GetCharPressed();
	Input *in = elem;

	if (IsKeyPressed(KEY_ENTER) && in->onenter) {
		gui_enter_input(in);
	} else if (gui_key_check(KEY_BACKSPACE, fcount) && in->len && in->cur) {
		editrm(in->wstr, &in->len, &in->cur);
		wcstombs(in->str, in->wstr, INPUT_MAX);
	} else if (gui_key_check(KEY_LEFT, fcount) && in->cur) {
		in->cur--;
	} else if (gui_key_check(KEY_RIGHT, fcount) && in->cur != in->len) {
		in->cur++;
	} else if (gui_key_check(KEY_TAB, fcount) && in->onenter == gui_input_next) {
		gui_input_next(GUI_INPUT, in);
	} else if (c && (!in->accept || in->accept(c)) && in->len < INPUT_MAX) {
		editins(in->wstr, &in->len, &in->cur, INPUT_MAX, c);
		wcstombs(in->str, in->wstr, INPUT_MAX);
	}
}

int
gui_input_next(int type, void *elem) {
	ui_focus(GUI_INPUT, elem + sizeof(Input));
	return 0;
}

void
gui_input_clear(Input *in) {
	edittrunc(in->wstr, &in->len, &in->cur);
}

/* This function is an intermediate filter that checks whether an item is
 * collapsed before running the filter defined in the Treeview struct */
static int
gui_treeview_filter(Tree *t, void *data) {
	Treeview *tv = data;

	if (t->u && t->u->collapsed)
		return 0;

	if (tv->filter)
		return tv->filter(t, tv->fdata);
	else
		return 1;
}

void
gui_treeview(int x, int y, int w, int h, Treeview *tv) {
	int depth;
	Tree *p;
	int cx, cy;
	int i;

	ui_draw_border_around(x, y, w, h, 1);

	if (!tv->rect.w && !tv->rect.h) /* zero when unitialized */
		tv->pane = PANESCROLL;
	tv->rect = RECT(x, y, w, h);
	tv->pane.geom = &tv->rect;

	pane_begin(&tv->pane);

	if (tv->print) {
		tv->print(x + PAD, y + FONT_SIZE, tv, NULL);
		cy = y + FONT_SIZE + PAD / 2;
	} else {
		cy = y;
	}

	for (p = NULL; tree_iter_f(tv->t, TREEMAX, &p, &depth, gui_treeview_filter, tv) != -1; ) {
		cy += FONT_SIZE;
		cx = x + PAD * (depth + 2);

		if (tv->colmask & p->type)
			ui_draw_expander(cx - PAD - 2, cy - 1, 9, !p->collapsed);

		if (tv->print)
			tv->print(cx, cy, tv, p);
		else
			ui_print(cx, cy, tv->sel == p ? col_info : col_fg, "%s", p->name);

		for (i = 0, cx = x + PAD; i < depth; i++, cx += PAD)
			ui_draw_rect(cx + 2, cy - 2, 1, FONT_SIZE + 1, col_altbg);
	}

	pane_end();
	gui_mouse_register(tv->rect, GUI_TREEVIEW, tv);
}

static void
gui_mouse_treeview(MouseButton button, Geom *geom, void *elem) {
	Treeview *tv = elem;
	int depth;
	Tree *p;
	int i, pos;
	int click = button == MOUSE_BUTTON_LEFT;

	pos = (mouse.y - geom->y - FONT_SIZE - (tv->print ? FONT_SIZE + PAD / 2 : 0) + tv->pane.off) / FONT_SIZE;

	for (i = 0, p = 0; tree_iter_f(tv->t, TREEMAX, &p, &depth, gui_treeview_filter, tv) != -1 && i <= pos; i++) {
		if (i == pos) {
			if (p->type & tv->colmask && (!(p->type & tv->selmask) ||
						mouse.x < geom->x + PAD * (depth + 2))) {
				if (click)
					p->collapsed = !p->collapsed;
				ui_cursor(MOUSE_CURSOR_POINTING_HAND);
			}
			if (p->type & tv->selmask && (!(p->type & tv->colmask) ||
						mouse.x > geom->x + PAD * (depth + 2))) {
				if (click) {
					if (gui_double_click() && tv->dclick)
						tv->dclick(GUI_TREEVIEW, tv);
					else
						tv->sel = p;
				}
				ui_cursor(MOUSE_CURSOR_POINTING_HAND);
			}
		}
	}
}

static int
gui_form_sub_end(int x, int y, int w) {
	ui_draw_line(x,     y,           x,         y + PAD * 2, 1, col_border);
	ui_draw_line(x + w, y,           x + w,     y + PAD * 2, 1, col_border);
	ui_draw_line(x,     y + PAD * 2, x + w - 1, y + PAD * 2, 1, col_border);

	return PAD;
}

/*
 * TODO: quite a lot, tbh.
 *
 * You know the nicely layed out game creation window from aurora?
 * Some day this should be able to do that.
 *
 */
void
gui_form(int x, int y, int w, int h, Form *form) {
	FormElem *elem;
	FormElem *sub;
	int bn, bx, bw;
	int lx, lw;
	int lpad;
	int cy;
	int tw;
	int i, j;
	int n;

	for (bn = 0, i = 0; i < FORM_BUTTON_MAX; i++)
		if (form->buttons[i])
			bn++;

	if (bn) {
		h -= BUTTON_HEIGHT + PAD;
		bx = x + h + PAD;

		if (h / bn < BUTTON_DEFW + PAD)
			bw = h / bn - PAD;
		else
			bw = BUTTON_DEFW;
	}

	for (i = 0, lx = lw = -1, cy = y, sub = NULL; form->elems[i].type != FORM_END_TYPE && i < FORM_MAX; i++) {
		elem = &form->elems[i];

		switch (elem->type) {
		case FORM_SUBFORM_TYPE:
			if (sub)
				cy += gui_form_sub_end(x, cy, w);

			sub = elem;
			cy += PAD * 2;
			ui_draw_line(x, cy + FONT_SIZE/2, x + PAD - 2, cy + FONT_SIZE/2, 1, col_border);
			tw = ui_print(x + PAD, cy, col_fg, "%s", elem->label) + PAD + 2;
			ui_draw_line(x + tw, cy + FONT_SIZE/2, x + w - 1, cy + FONT_SIZE/2, 1, col_border);
			ui_draw_line(x,     cy + FONT_SIZE/2, x,     cy + PAD * 1.5, 1, col_border);
			ui_draw_line(x + w, cy + FONT_SIZE/2, x + w, cy + PAD * 1.5, 1, col_border);
			cy += PAD * 0.5;
			/* fallthrough */
		case FORM_NEWLINE_TYPE:
			cy += PAD;
			lx = lw = -1;
			continue;
		}

		if (lx == -1 || lw == -1) {
			for (
				j = i, n = 0;
				form->elems[j].type != FORM_END_TYPE &&
				form->elems[j].type != FORM_SUBFORM_TYPE &&
				form->elems[j].type != FORM_NEWLINE_TYPE;
				j++, n++);

			if (sub) {
				lx = x + PAD;
				lw = (w - PAD * 2 - PAD * (n - 1)) / n;
			} else {
				lx = x;
				lw = w - PAD * (n - 1) / n;
			}
		}

		if (elem->required)
			ui_printw(lx, cy, lw, col_altfg, "*");

		if (elem->label)
			tw = ui_printw(lx + 5, cy, lw, col_fg, "%s:  ", elem->label);
		else
			tw = 0;

		switch (elem->type) {
		case GUI_INPUT:
			gui_input(lx + tw, cy, lw - tw, elem->elem);
			break;
		case GUI_CHECKBOX:
			gui_checkbox(lx + tw, cy, elem->elem);
			break;
		case GUI_DROPDOWN:
			gui_dropdown(lx + tw, cy, lw - tw, elem->elem);
			break;
		}

		if (sub) {
			ui_draw_line(x,     cy, x,     cy + PAD, 1, col_border);
			ui_draw_line(x + w, cy, x + w, cy + PAD, 1, col_border);
		}

		lx += lw + PAD;
	}

	if (sub)
		gui_form_sub_end(x, cy, w);

	for (i = 0; i < bn; i++)
		gui_button(x + w - BUTTON_DEFW * (i + 1) - PAD * i, y + h + PAD, bw, form->buttons[i]);
}

int
gui_form_filled(Form *form) {
	FormElem *elem;
	Input *in;
	Dropdown *drop;
	int i;

	for (i = 0; i < FORM_MAX && form->elems[i].type != FORM_END_TYPE; i++) {
		elem = &form->elems[i];

		if (!elem->required)
			continue;

		switch (elem->type) {
		case GUI_INPUT:
			in = elem->elem;
			if (!in->str[0])
				return 0;
			break;
		case GUI_DROPDOWN:
			drop = elem->elem;
			if (drop->sel == -1)
				return 0;
			break;
		}
	}

	return 1;
}

void
gui_form_clear(Form *form) {
	FormElem *elem;
	Input *in;
	Checkbox *ch;
	Dropdown *drop;
	int i;

	for (i = 0; i < FORM_MAX && form->elems[i].type != FORM_END_TYPE; i++) {
		elem = &form->elems[i];

		switch (elem->type) {
		case GUI_INPUT:
			in = elem->elem;
			gui_input_clear(in);
			break;
		case GUI_CHECKBOX:
			ch = elem->elem;
			if (!ch->def)
				continue;
			ch->val = ch->def - CHECKBOX_DEFAULT_OFF;
			break;
		case GUI_DROPDOWN:
			drop = elem->elem;
			if (!drop->def)
				continue;
			drop->sel = drop->def - DROPDOWN_DEFAULT_OFF;
			break;
		}
	}
}
