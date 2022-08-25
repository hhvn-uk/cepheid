#include <stddef.h>
#include <stdarg.h>
#include <raylib.h>
#include "struct.h"
#include "style.h"
#include "maths.h"
#include "../db/db.h"

#define ELEMS(array) (sizeof(array)/sizeof(array[0]))

/* main.c */
extern Save *save;

/* str.c */
void *	falloc(size_t size);
void	ffree(void);
char *	vsfprintf(char *fmt, va_list args);
char *	sfprintf(char *fmt, ...); /* return string allocated for current frame */
char *	vsmprintf(char *fmt, va_list args);
char *	smprintf(char *fmt, ...); /* return allocated string */
char *	nstrdup(char *str); /* NULL-safe */
char *	strkmdist(float km);
char *	strlightdist(float km);
int	streq(char *s1, char *s2); /* NULL-safe, no `== 0` required */
int	strprefix(char *str, char *prefix);
char *	strsuffix(char *str, char *suffix);
int	strlistpos(char *str, char **list, size_t len);
float	strnum(char *str);

/* ui.c */
extern Tabs view_tabs;
extern int (*view_handlers[UI_VIEW_LAST])(int);
extern void (*view_drawers[UI_VIEW_LAST])(void);
extern Rect screen_rect;
extern View_main view_main;
extern View_sys view_sys;
void	ui_init(void);
void	ui_update_screen(void);
void	ui_deinit(void);
void	ui_print(int x, int y, Color col, char *format, ...);
int	ui_textsize(char *text);
int	ui_collides(Geom geom, Vector2 point);
int	ui_onscreen(Vector2 point);
int	ui_onscreen_ring(Vector2 centre, float r);
int	ui_onscreen_circle(Vector2 centre, float r);
void	ui_clickable_register(Geom geom, enum UiElements type, void *elem);
void	ui_clickable_handle(Vector2 mouse, MouseButton button, Clickable *clickable);
int	ui_clickable_update(void);
void	ui_clickable_clear(void);
void	ui_draw_views(void);
void	ui_draw_border(int x, int y, int w, int h, int px);
void	ui_draw_ring(int x, int y, float r, Color col);
void	ui_draw_tabs(int x, int y, int w, int h, Tabs *tabs);
void	ui_draw_tabbed_window(int x, int y, int w, int h, Tabs *tabs);
void	ui_draw_checkbox(int x, int y, Checkbox *checkbox);
Vector2	ui_kmtopx(Vector2 km);
Vector2	ui_pxtokm(Vector2 vector);
Vector2 ui_vectordiff(Vector2 a, Vector2 b);
float	ui_vectordist(Vector2 a, Vector2 b);
int	ui_should_draw_body(Body *body, int orbit);
void	ui_draw_body(Body *body);
int	ui_handle_view_main(int nowsel);
int	ui_handle_view_colonies(int nowsel);
int	ui_handle_view_bodies(int nowsel);
int	ui_handle_view_fleets(int nowsel);
int	ui_handle_view_design(int nowsel);
int	ui_handle_view_sys(int nowsel);
int	ui_handle_view_settings(int nowsel);
void	ui_draw_view_main(void);
void	ui_draw_view_colonies(void);
void	ui_draw_view_bodies(void);
void	ui_draw_view_fleets(void);
void	ui_draw_view_design(void);
void	ui_draw_view_sys(void);
void	ui_draw_view_settings(void);

/* system.c */
int	bodytype_enumify(char *name);
char *	bodytype_strify(Body *body);
Vector2	sys_vectorize(Polar polar);
Vector2 sys_vectorize_around(Vector2 around, Polar polar);
Polar	sys_polarize(Vector2 vector);
Polar	sys_polarize_around(Vector2 around, Vector2 vector);
Polar	sys_sum_polar(Polar absolute, Polar relative);
Vector2	sys_get_vector(Body *body);
Polar	sys_get_polar(Body *body);
System *sys_init(char *name);
System *sys_load(System *s, char *name);
System *sys_get(char *name);
System *sys_default(void);

/* save.c */
void 	save_read(char *dir);
void	save_write(void);

/* data.c */
extern Font font;
extern Texture image_tactical;
extern Texture image_colonies;
extern Texture image_bodies;
extern Texture image_fleet;
extern Texture image_design;
extern Texture image_sys;
extern Texture image_settings;
void	data_load(void);
void	data_unload(void);

/* db.c */
int	vdbsetf(char *dir, char *group, char *key, char *fmt, va_list args);
int	dbsetf(char *dir, char *group, char *key, char *fmt, ...);
int	dbsetint(char *dir, char *group, char *key, int val);
int	dbsetfloat(char *dir, char *group, char *key, float val);
void	dbsetbody(System *sys, Body *body);
int	vdbgetf(char *dir, char *group, char *key, char *fmt, va_list args);
int	dbgetf(char *dir, char *group, char *key, char *fmt, ...);
int	dbgetint(char *dir, char *group, char *key);
float	dbgetfloat(char *dir, char *group, char *key);
