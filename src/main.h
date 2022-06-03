#include <stddef.h>
#include <stdarg.h>
#include <raylib.h>
#include "struct.h"
#include "style.h"
#include "../db/db.h"

#define ELEMS(array) (sizeof(array)/sizeof(array[0]))

/* main.c */
extern Save *save;

/* str.c */
char *	vsmprintf(char *fmt, va_list args);
char *	smprintf(char *fmt, ...); /* return allocated string */
char *	vsbprintf(char *fmt, va_list args);
char *	sbprintf(char *fmt, ...); /* return string that is usable until next call */

/* ui.c */
#define VIEWS_MAX_WIDTH (UI_VIEW_LAST*100)
#define VIEWS_HEIGHT 25
#define WINDOW_TAB_HEIGHT 20
extern Tabs view_tabs;
extern void (*view_drawers[UI_VIEW_LAST])(void);
void	ui_init(void);
void	ui_deinit(void);
void	ui_print(int x, int y, Color col, char *format, ...);
int	ui_textsize(char *text);
void	ui_clickable_register(int x, int y, int w, int h, enum UiElements type, void *elem);
void	ui_clickable_handle(Vector2 mouse, MouseButton button, Clickable *clickable);
void	ui_clickable_update(void);
void	ui_draw_views(void);
void	ui_draw_border(int x, int y, int w, int h, int px);
void	ui_draw_tabs(Tabs *tabs, int x, int y, int w, int h);
void	ui_draw_tabbed_window(Tabs *tabs, int x, int y, int w, int h);
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
Vector2	system_get_vector(char *system, char *body);
Polar	system_get_polar(char *system, char *body);

/* save.c */
Save *	save_init(char *savedir);

/* data.c */
#include <stddef.h>
extern unsigned char DejaVuSansMono_ttf[];
extern size_t DejaVuSansMono_ttf_size;
