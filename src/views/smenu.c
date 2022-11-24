#include <raylib.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include <time.h>
#include <sys/stat.h>
#include "../main.h"

#define CONTCUR "Back to current game"

#define LOAD_W		300
#define LOAD_H		350
#define LOAD_NAMEW	100
#define BUTTON_W	50

static void savecheck(char *msg, void (*action)(void));
static int savecheck_callback(int type, void *elem);
static int buttonhandler(int type, void *elem);
static void newhandler(void);
static int newhandler_actual(int type, void *elem);
static int newhandler_back(int type, void *elem);
static void quithandler(void);
static void loadhandler(void);
static int loadhandler_actual(int type, void *elem);
static void loadprinter(int x, int y, Treeview *tv, Tree *t);
static void loadadd(char *dir, time_t mod);
static void loadinit(char *sdir);

static View_smenu *v = &view_smenu;
View_smenu view_smenu = {
	.main = {
		.x = PAD,
		.y = PAD,
		.w = 250,
		.h = 0,
	},
	.b = {
		[SMENU_NEW] = {1, "New Game", buttonhandler, SMENU_NEW },
		[SMENU_CONT] = {0, view_smenu.cont.label, buttonhandler, SMENU_CONT },
		[SMENU_SAVE] = {0, "Save Game", buttonhandler, SMENU_SAVE },
		[SMENU_LOAD] = {0, "Load Game", buttonhandler, SMENU_LOAD },
		[SMENU_QUIT] = {1, "Quit", buttonhandler, SMENU_QUIT },
	},
	.new = {
		.disp = 0,
		.name = {
			.placeholder = "Save name",
			.onenter = newhandler_actual,
		},
		.create = {1, "Create", NULL, 0, .submit = &view_smenu.new.name},
		.back = {1, "Back", newhandler_back, 0},
	},
	.cont = {
		.save = NULL,
		.label = "Continue",
	},
	.save = {
		.check = 0,
		.msg = NULL,
		.back = {1, "Back", NULL, -1},
		.save = {1, "Save", NULL, 1},
		.discard = {1, "Discard", NULL, 0},
	},
	.load = {
		.init = 1,
		.disp = 0,
		.saves = {0},
		.savelist = {
			.t = &view_smenu.load.saves,
			.sel = NULL,
			.selmask = 0xff,
			.colmask = 0x00,
			.filter = NULL,
			.print = loadprinter,
			.dclick = loadhandler_actual,
		},
		.load = {0, "Load", loadhandler_actual, 0 },
	}
};

static void
savecheck(char *msg, void (*action)(void)) {
	v->save.check = 1;
	v->save.back.func =
		v->save.save.func =
		v->save.discard.func = savecheck_callback;
	v->save.msg = msg;
	v->save.func = action;
}

static int
savecheck_callback(int type, void *elem) {
	Button *b = elem;
	int arg = b->arg;

	v->save.check = 0;
	if (arg)
		save_write();
	if (arg != -1)
	v->save.func();
	return 0;
}


static int
buttonhandler(int type, void *elem) {
	Button *b = elem;
	int arg = b->arg;

	switch (arg) {
	case SMENU_NEW:
		if (save && save_changed())
			savecheck("The current game hasn't been saved. Save it?", newhandler);
		else
			newhandler();
		break;
	case SMENU_SAVE:
		save_write();
		/* add to event log */
		/* fallthrough */
	case SMENU_CONT:
		if (!save) {
			save_read(v->cont.save->name);
			strcpy(v->cont.label, CONTCUR);
		}
		view_tabs.sel = view_before_smenu;
		break;
	case SMENU_LOAD:
		if (save && save_changed())
			savecheck("The current game hasn't been saved. Save it?", loadhandler);
		else
			loadhandler();
		break;
	case SMENU_QUIT:
		checkbeforequit();
		break;
	}
	return 0;
}

static void
newhandler(void) {
	v->new.disp = 1;
}

static int
newhandler_actual(int type, void *elem) {
	save_create(v->new.name.str);
	save_read(v->new.name.str);
	loadadd(v->new.name.str, time(NULL));
	view_tabs.sel = VIEW_MAIN;
	v->new.disp = 0;
	return 1;
}

static int
newhandler_back(int type, void *elem) {
	gui_input_clear(&v->new.name);
	v->new.disp = 0;
	return 0;
}

static void
quithandler(void) {
	quit = 1;
}

static void
loadhandler(void) {
	v->load.disp = 1;
	/* strcpy(v->cont.label, CONTCUR); */
	/* v->b[SMENU_CONT].enabled = 1; */
}

static int
loadhandler_actual(int type, void *elem) {
	struct Loadable *l;

	l = v->load.savelist.sel->data;
	save_read(l->name);
	view_tabs.sel = VIEW_MAIN;
	v->load.disp = 0;
	return 0;
}

static void
loadprinter(int x, int y, Treeview *tv, Tree *t) {
	struct Loadable *l;
	Color c = (tv->sel == t) ? col_info : col_fg;

	if (!t) {
		ui_print(x + PAD,              y, col_fg, "Name");
		ui_print(x + PAD + LOAD_NAMEW, y, col_fg, "Last saved");
	} else {
		l = t->data;
		ui_printw(x,              y, LOAD_NAMEW, c, "%s", l->name);
		ui_print( x + LOAD_NAMEW, y, c, "%s", strdate(l->mod));
	}
}

static void
loadadd(char *dir, time_t mod) {
	struct Loadable *p;

	p = malloc(sizeof(struct Loadable));
	p->name = nstrdup(dir);
	p->mod = mod;

	tree_add_child(&v->load.saves, p->name, 1, p, NULL);
}

void
checkbeforequit(void) {
	if (save_changed()) {
		view_tabs.sel = VIEW_SMENU;
		savecheck("There are unsaved changes. Save before quitting?", quithandler);
	} else {
		quithandler();
	}
}

static void
loadinit(char *sdir) {
	struct dirent **dirent;
	struct stat st;
	char path[PATH_MAX];
	int n, i;

	n = scandir(sdir, &dirent, NULL, alphasort);
	if (n < 0) return;

	for (i = 0; i < n; i++) {
		snprintf(path, sizeof(path), "%s/%s", sdir, dirent[i]->d_name);
		stat(path, &st);
		if (dirent[i]->d_name[0] != '.' && S_ISDIR(st.st_mode))
			loadadd(dirent[i]->d_name, st.st_mtime);
		free(dirent[i]);
	}
	free(dirent);
}

void
view_smenu_handle(int nowsel) {
	Tree *t;
	struct Loadable *cont, *l;

	v->main.h = PAD + SMENU_LAST * (BUTTON_HEIGHT + PAD);
	v->main.x = (screen.w - v->main.w) / 2;
	v->main.y = (screen.h - v->main.h) / 2;

	if (v->load.init) {
		loadinit(SAVEDIR);
		for (t = v->load.saves.d, cont = NULL; t; t = t->n) {
			l = t->data;
			if (!cont || l->mod > cont->mod)
				cont = l;
		}
		if ((v->cont.save = cont) != NULL) {
			snprintf(v->cont.label, sizeof(v->cont.label),
					"Continue (%s)", v->cont.save->name);
			v->b[SMENU_CONT].enabled = 1;
		}
		if (v->load.saves.d)
			v->b[SMENU_LOAD].enabled = 1;
		v->load.init = 0;
	}

	if (v->load.disp && v->load.savelist.sel)
		v->load.load.enabled = 1;
	else
		v->load.load.enabled = 0;

	v->b[SMENU_SAVE].enabled = save ? 1 : 0;
}

void
view_smenu_draw(void) {
	Color bg = { col_bg.r, col_bg.g, col_bg.b, 0xcc };
	int x, y, w, h;
	int i;

	ui_draw_texture_part(image_splash, 0, 0,
			MAX((image_splash.width - screen.w) / 2, 0),
			MAX((image_splash.height - screen.h) / 2, 0),
			screen.w, screen.h);

	if (v->new.disp) {
		w = PAD * 2 + 300;
		h = PAD * 3 + FONT_SIZE + BUTTON_HEIGHT;
		x = (screen.w - w) / 2;
		y = (screen.h - h) / 2;

		ui_draw_rect(x, y, w, h, col_bg);
		ui_draw_border_around(x, y, w, h, 1);

		x += PAD;
		y += PAD;
		gui_input(x, y, 300, &v->new.name);

		x += w - 50 - PAD * 2,
		gui_button(x, y + PAD * 2, 50, &v->new.create);

		x -= 50 + PAD;
		gui_button(x, y + PAD * 2, 50, &v->new.back);
	} else if (v->save.check) {
		w = PAD * 2 + ui_textsize(v->save.msg);
		h = PAD * 3 + FONT_SIZE + BUTTON_HEIGHT;
		x = (screen.w - w) / 2;
		y = (screen.h - h) / 2;

		ui_draw_rect(x, y, w, h, col_bg);
		ui_draw_border_around(x, y, w, h, 1);

		x += PAD;
		y += PAD;
		ui_print(x, y, col_fg, "%s", v->save.msg);

		x += w - PAD * 2 - 50;
		y += PAD * 2;

		gui_button(x, y, 50, &v->save.back);
		gui_button(x -= 50 + PAD, y, 50, &v->save.discard);
		gui_button(x -= 50 + PAD, y, 50, &v->save.save);
	} else if (v->load.disp) {
		w = PAD * 2 + LOAD_W;
		h = PAD * 3 + LOAD_H + BUTTON_HEIGHT;
		x = (screen.w - w) / 2;
		y = (screen.h - h) / 2;

		ui_draw_rect(x, y, w, h, col_bg);
		ui_draw_border_around(x, y, w, h, 1);

		x += PAD;
		y += PAD;

		gui_treeview(x, y, LOAD_W, LOAD_H, &v->load.savelist);
		gui_button(x + w - BUTTON_W - PAD * 2,
				y + h - PAD * 2 - BUTTON_HEIGHT,
				BUTTON_W, &v->load.load);
	} else {
		ui_draw_rect(EXPLODE_RECT(v->main), bg);
		ui_draw_border_around(EXPLODE_RECT(v->main), 1);

		x = v->main.x + PAD;
		y = v->main.y + PAD;
		w = v->main.w - PAD * 2;

		for (i = 0; i < SMENU_LAST; i++) {
			ui_draw_rect(x, y, w, BUTTON_HEIGHT, col_bg);
			gui_button(x, y, w, &v->b[i]);
			y += BUTTON_HEIGHT + PAD;
		}
	}
}
