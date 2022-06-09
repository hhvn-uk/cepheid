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
char *	vsmprintf(char *fmt, va_list args);
char *	smprintf(char *fmt, ...); /* return allocated string */
char *	nstrdup(char *str); /* NULL-safe */
char *	strkmdist(float km);
char *	strlightdist(float km);
int	streq(char *s1, char *s2); /* NULL-safe, no `== 0` required */
int	strlistpos(char *str, char **list, size_t len);
float	strnum(char *str);

/* ui.c */
extern Tabs view_tabs;
extern void (*view_handlers[UI_VIEW_LAST])(void);
extern void (*view_drawers[UI_VIEW_LAST])(void);
void	ui_init(void);
void	ui_deinit(void);
void	ui_print(int x, int y, Color col, char *format, ...);
int	ui_textsize(char *text);
int	ui_collides(Rect rect, Vector2 point);
void	ui_clickable_register(int x, int y, int w, int h, enum UiElements type, void *elem);
void	ui_clickable_handle(Vector2 mouse, MouseButton button, Clickable *clickable);
void	ui_clickable_update(void);
void	ui_draw_views(void);
void	ui_draw_border(int x, int y, int w, int h, int px);
void	ui_draw_tabs(int x, int y, int w, int h, Tabs *tabs);
void	ui_draw_tabbed_window(int x, int y, int w, int h, Tabs *tabs);
void	ui_draw_checkbox(int x, int y, Checkbox *checkbox);
Vector2	ui_kmtopx(Vector2 km);
Vector2	ui_pxtokm(Vector2 vector);
Vector2 ui_vectordiff(Vector2 a, Vector2 b);
float	ui_vectordist(Vector2 a, Vector2 b);
int	ui_should_draw_body(Body *body, int orbit);
void	ui_draw_body(Body *body);
void	ui_handle_view_main(void);
void	ui_handle_view_colonies(void);
void	ui_handle_view_fleets(void);
void	ui_handle_view_design(void);
void	ui_handle_view_systems(void);
void	ui_handle_view_settings(void);
void	ui_draw_view_main(void);
void	ui_draw_view_colonies(void);
void	ui_draw_view_fleets(void);
void	ui_draw_view_design(void);
void	ui_draw_view_systems(void);
void	ui_draw_view_settings(void);

/* system.c */
Vector2	system_vectorize(Polar polar);
Vector2 system_vectorize_around(Vector2 around, Polar polar);
Polar	system_polarize(Vector2 vector);
Polar	system_sum_polar(Polar absolute, Polar relative);
Vector2	system_get_vector(Body *body);
Polar	system_get_polar(Body *body);
System *system_init(char *name);
System *system_load(System *s, char *name);

/* save.c */
Save *	save_init(char *savedir);

/* data.c */
#include <stddef.h>
extern unsigned char DejaVuSansMono_ttf[];
extern size_t DejaVuSansMono_ttf_size;
