#include <stddef.h>
#include <stdarg.h>
#include <raylib.h>
#include "struct.h"
#include "views/struct.h"
#include "style.h"
#include "maths.h"
#include "../db/db.h"

#define ELEMS(array) (sizeof(array)/sizeof(array[0]))
#define MAX(a, b) (a > b ? a : b)
#define MIN(a, b) (a < b ? a : b)
#define SORT_FUNC(func, type) \
	static int \
	func##_sort(const void *a, const void *b) { \
		return func(*(type*)a, *(type*)b); \
	}

#ifdef CHECK_FRAME_MEM_FREE
#define CUSTOM_FREE
#endif /* CHECK_FRAME_MEM_FREE */

#if defined(CUSTOM_FREE) && !defined(TEST)
#define free(m) _free(m, __FILE__, __LINE__, __func__)
#endif /* defined(CUSTOM_FREE) && !defined(TEST) */

/* main.c */
extern Save *save;
extern int sigint;
extern int sigterm;
extern int quit;
extern int view_before_smenu;

/* err.c */
#define ERR(code, type, fmt, ...) _err(code, type, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#define error(code, fmt, ...) ERR(code, "error", fmt, ##__VA_ARGS__)
#define warning(fmt, ...) ERR(-1, "warning", fmt, ##__VA_ARGS__)
void	_err(int code, char *type, char *file, int line, const char *func, char *fmt, ...);

/* mem.c */
void *	falloc(size_t size);
void	ffree(void);
char *	fstrdup(char *str);
void *	emalloc(size_t size);
void *	estrdup(char *str);
void *	ecalloc(size_t nmemb, size_t size);
void *	erealloc(void *pt, size_t size);
void	_free(void *mem, char *file, int line, const char *func);

/* str.c */
char *	vsfprintf(char *fmt, va_list args);
char *	sfprintf(char *fmt, ...); /* return string allocated for current frame */
char *	vsmprintf(char *fmt, va_list args);
char *	smprintf(char *fmt, ...); /* return allocated string */
char *	nstrdup(char *str); /* NULL-safe */
char *	strkm(float km);
char *	strly(float km);
char *	strdate(time_t time);
int	streq(char *s1, char *s2); /* NULL-safe, no `== 0` required */
int	strprefix(char *str, char *prefix);
char *	strsuffix(char *str, char *suffix);
int	strlistpos(char *str, char **list, size_t len);
float	strnum(char *str);
size_t	strlistcmp(char **l1, size_t s1, char **l2, size_t s2);
size_t	strsplit(char *str, char *sep, char **list, size_t len);
void	strjoinl(char *sep, char **ret, char *append);
char *	strjoin(char *sep, char **list, size_t len);
char *	strtrunc(char *str, int w);
void	edittrunc(wchar_t *str, int *len, int *cur);
void	editrm(wchar_t *str, int *len, int *cur);
void	editins(wchar_t *str, int *len, int *cur, int size, wchar_t c);

/* coords.c */
Vector	vectorize(Polar p);
Vector	vectorize_at(Vector at, Polar p);
Polar	polarize(Vector v);
Polar	polarize_at(Vector at, Vector vector);
Polar	polar_add(Polar abs, Polar rel);

/* tree.c */
Tree *	tree_add_child(Tree *t, char *name, int type, void *data, Tree **ptr);
int	tree_delete(Tree **t, Treefree freedata);
int	tree_delete_r(Tree **t, Treefree freedata);
int	tree_delete_root(Tree *t, Treefree freedata);
int	tree_iter_f(Tree *t, int maxdepth, Tree **p, int *depth, Treefilter filter, void *fdata);
int	tree_iter(Tree *t, int maxdepth, Tree **p, int *depth);
void	tree_sort_sideways(Tree *t, Treecompar compar, void *cdata);
void	tree_sort(Tree *t, Treecompar compar, void *cdata);

/* ui.c */
#define WINDOW_TAB_HEIGHT 20
#define TARGET_FPS 60
#define WINDOW_BORDER 2
#define PAD 10
#define TPX 2
#define TPY 1
#define RECT(nx, ny, nw, nh) (Geom){UI_RECT, .x = nx, .y = ny, .w = nw, .h = nh}
#define CIRCLE(x, y, nr) (Geom){UI_CIRCLE, .centre={x, y}, .r = nr}
#define CIRCLEV(v, nr) (Geom){UI_CIRCLE, .centre = v, .r = nr}
#define RLIFY_RECT(g) (Rectangle){g.x, g.y, g.w, g.h}
#define EXPLODE_RECT(g) g.x, g.y, g.w, g.h
#define EXPLODE_CIRCLE(g) g.centre.x, g.centre.y, g.r
#define EXPLODE_CIRCLEV(g) g.centre, g.r
extern Screen screen;
extern Focus focus;
extern Mouse mouse;
extern int charpx;
void	ui_init(void);
void	ui_update_screen(void);
void	ui_focus(enum GuiElements type, void *p);
int	ui_loop(void);
void	ui_deinit(void);
void	ui_print(int x, int y, Color col, char *format, ...);
void	ui_printw(int x, int y, int w, Color col, char *format, ...);
void	ui_title(char *fmt, ...);
int	ui_textsize(char *text);
void	ui_cursor(MouseCursor curs);
int	ui_collides(Geom geom, Vector point);
int	ui_onscreen(Vector point);
int	ui_onscreen_ring(Vector centre, float r);
int	ui_onscreen_circle(Vector centre, float r);
void	ui_draw_views(void);
void	ui_draw_rect(int x, int y, int w, int h, Color col);
void	ui_draw_expander(int x, int y, int w, int expanded);
void	ui_draw_border(int x, int y, int w, int h, int px);
void	ui_draw_border_around(int x, int y, int w, int h, int px);
void	ui_draw_ring(int x, int y, float r, Color col);
void	ui_draw_texture(Texture2D texture, int x, int y);
void	ui_draw_texture_part(Texture2D texture, int x, int y, int fx, int fy, int w, int h);
void	ui_draw_circle(int x, int y, float r, Color col);
void	ui_draw_line(int sx, int sy, int ex, int ey, float thick, Color col);
void	ui_draw_line_v(Vector start, Vector end, float thick, Color col);
void	ui_draw_tabbed_window(int x, int y, int w, int h, Tabs *tabs);
Vector	ui_vectordiff(Vector a, Vector b);
float	ui_vectordist(Vector a, Vector b);

/* gui.c */
#define BUTTON_HEIGHT (PAD + FONT_SIZE)
int	gui_mouse_handle(void);
void	gui_key_handle(void);
void	gui_tabs(int x, int y, int w, int h, Tabs *tabs);
int	gui_checkbox(int x, int y, Checkbox *checkbox); /* returns width */
void	gui_button(int x, int y, int w, Button *b);
void	gui_dropdown(int x, int y, int w, Dropdown *d);
void	gui_input(int x, int y, int w, Input *in);
int	gui_input_next(int type, void *elem); /* if inputs are contained in an array, this .onenter advances to the next */
void	gui_input_clear(Input *in);
void	gui_treeview(int x, int y, int w, int h, Treeview *tv);

/* views.c */
#define VIEWS_MAX_WIDTH (VIEW_LAST*100)
#define VIEWS_HEIGHT 25
extern void (*view_init[VIEW_LAST])(void);
extern void (*view_handle[VIEW_LAST])(int);
extern void (*view_draw[VIEW_LAST])(void);
extern Tabs view_tabs;
void	view_colonies_handle(int nowsel);
void	view_fleets_handle(int nowsel);
void	view_design_handle(int nowsel);
void	view_settings_handle(int nowsel);
void	view_colonies_draw(void);
void	view_fleets_draw(void);
void	view_design_draw(void);
void	view_settings_draw(void);

/* views/main.c */
extern View_main view_main;
void	view_main_init(void);
void	view_main_handle(int nowsel);
void	view_main_draw(void);

/* views/bodies.c */
extern View_bodies view_bodies;
void	view_bodies_init(void);
void	view_bodies_handle(int nowsel);
void	view_bodies_draw(void);

/* views/sys.c */
extern View_sys view_sys;
void	view_sys_handle(int nowsel);
void	view_sys_draw(void);

/* views/smenu.c */
extern View_smenu view_smenu;
void	view_smenu_handle(int nowsel);
void	view_smenu_draw(void);
void	checkbeforequit(void);

/* pane.c */
void	pane_begin(Pane *f);
void	pane_end(void);
int	pane_visible(float miny, float maxy); /* calls pane_max automatically */
float	pane_max(float y); /* returns original y */
float	pane_y(float y);
Vector	pane_v(Vector v);
void	pane_scroll(Pane *f, int incr);

/* system.c */
System *sys_init(char *name);
void	sys_tree_load(void);
char *	sys_tree_getter(char *dir, char *group, char *name, int depth, Tree *t);
void	sys_tree_setter(char *dir, char *group, char *name, int depth, Tree *t);
void	sys_tree_free(Tree *t);
System *sys_get(char *name);
System *sys_default(void);
void	sys_free(System *s);

/* body.c */
int	bodytype_enumify(char *name);
char *	bodytype_strify(Body *body);
Body *	body_init(char *name);
void	body_free(Body *b);

/* save.c */
void 	save_read(char *dir);
int	save_changed(void);
void	save_write(void);
int	save_exists(char *name);
int	save_create(char *name);

/* ../data/dirs.c */
int	dirs_write(char *dir, char *to);

/* data.c */
extern Font font;
extern Texture image_tactical;
extern Texture image_colonies;
extern Texture image_bodies;
extern Texture image_fleet;
extern Texture image_design;
extern Texture image_sys;
extern Texture image_settings;
extern Texture image_burger;
extern Texture image_splash;
void	data_load(void);
void	data_unload(void);

/* bdb.c */
#define bdbset(d, g, ...) _bdbset(d, g, __VA_ARGS__, NULL)
#define bdbget(d, g, ...) _bdbget(d, g, __VA_ARGS__, NULL)
void	_bdbset(char *dir, char *group, ...);
void	_bdbget(char *dir, char *group, ...);

/* db.c */
int	vdbsetf(char *dir, char *group, char *key, char *fmt, va_list args);
int	dbsetf(char *dir, char *group, char *key, char *fmt, ...);
int	vdbgetf(char *dir, char *group, char *key, char *fmt, va_list args);
int	dbgetf(char *dir, char *group, char *key, char *fmt, ...);
int	dbgettree(char *dir, Tree *t, Treegetter func);
int	dbsettree(char *dir, Tree *t, Treesetter func);

/* loading.c */
Loader *loading_open(int steps, char *initstr);
void	loading_update(Loader *hand, char *str);
void	loading_close(Loader *hand);

/* maths.c */
float	cosf_d(float x);
float	sinf_d(float x);
float	atan2f_d(float y, float x);

/* time.c */
void	timespec_diff(struct timespec *t1, struct timespec *t2, struct timespec *diff);
