#include <stddef.h>
#include <sys/types.h>

/* system.c */
typedef struct {
	float r;
	float theta;
} Polar;

/* body->type + body->parent->type == complex type
 *
 * Ex. for orbiting a star:
 *   BODY_PLANET + BODY_STAR == BODY_PLANET (still body->type)
 *
 * Ex. for a moon orbiting a body:
 *   BODY_MOON + BODY_PLANET is unique
 */
enum BodyType {
	BODY_STAR = 0,
	BODY_PLANET,
	BODY_DWARF,
	BODY_ASTEROID,
	BODY_COMET,
	BODY_MOON,
	BODY_LAST = BODY_MOON * 2
};

typedef struct Body Body;
struct Body {
	Body *parent;
	Polar polar;
	Vector2 vector;
	Vector2 pxloc; /* used by ui functions */
	char *name;
	enum BodyType type;
	float radius;
	float mass;
	float orbdays;
	union {
		/* comet */
		struct {
			float mindist;
			float maxdist;
			float curdist;
			float theta;
			int inward;
		};
		/* everything else */
		struct {
			float dist;
			float curtheta;
		};
	};
};

typedef struct {
	char *name;
	Body **bodies;
	size_t bodies_len;
	Body *furthest_body;
	struct {
		int stars;
		int planets;
		int dwarfs;
		int asteroids;
		int comets;
		int moons;
	} num;
	Vector2 lypos;
} System;

/* save.c */
typedef struct {
	struct {
		char *dir;
		char *races;
		char *systems;
		char *fleets;
	} db;
	System *systems;
	size_t systems_len;
	System *homesys;
} Save;

/* ui.c */
enum Geometries {
	UI_RECT,
	UI_CIRCLE,
};

typedef struct {
	enum Geometries type;
	union {
		struct {
			int x, y;
			int w, h;
		};
		struct {
			Vector2 centre;
			int r;
		};
	};
} Geom;

/* pane.c */
#define PANESCROLL {NULL, 1, 0, 0}
#define PANENOSCROLL {NULL, 0, 0, 0}
typedef struct {
	Geom *geom;
	int scroll;
	int max;
	int off;
} Pane;

typedef struct {
	float w, h;
	float diag;
	Vector2 centre;
	Geom rect; /* for passing to functions */
} Screen;

#define TABS_MAX 16
typedef struct {
	int n;
	int sel;
	struct {
		Texture *icon;
		char *name;
		int w; /* 0 = fill */
	} tabs[TABS_MAX];
} Tabs;

typedef struct {
	int enabled;
	int val;
	char *label;
} Checkbox;

typedef struct {
	char *name;
	void (*func)(int);
	int arg;
} Button;

#define INPUT_MAX 512
typedef struct Input Input;
struct Input {
	char str[INPUT_MAX];
	wchar_t wstr[INPUT_MAX]; /* Oh no, not everything will fit. Whatever. */
	char *placeholder;
	int len;
	int cur;
	int (*onenter)(Input *); /* return positive to clear */
};

#define DROPDOWN_MAX 64
typedef struct {
	int n;
	int sel; /* -1 for none */
	char *placeholder;
	char *val[DROPDOWN_MAX];
	/* internal */
	Geom rect;
	Pane pane;
} Dropdown;

enum UiElements {
	UI_TAB,
	UI_CHECKBOX,
	UI_BODY,
	UI_BUTTON,
	UI_INPUT,
	UI_DROPDOWN,
};

typedef struct {
	enum UiElements type;
	void *p;
} Focus;

enum UiViews {
	UI_VIEW_MAIN,
	UI_VIEW_COLONIES,
	UI_VIEW_BODIES,
	UI_VIEW_FLEETS,
	UI_VIEW_DESIGN,
	UI_VIEW_SYSTEMS,
	UI_VIEW_SETTINGS,
	UI_VIEW_LAST
};

#define CLICKABLE_MAX 64
typedef struct {
	Geom geom;
	enum UiElements type;
	void *elem;
} Clickable;

/* loading.c */
#define LOAD_STR_MAX 512
typedef struct {
	char path[64];
	pid_t pid;
	int *step;
	char *data;
} Loader;
