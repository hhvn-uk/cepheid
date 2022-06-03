#include <stddef.h>

typedef struct Save Save;
struct Save {
	/* db locator strings.
	 * dir the base, everything else is "dir/var" */
	char *dir;
	char *races;
	char *systems;
	char *fleets;
};

/* ui.c */
#define TABS_MAX 16
typedef struct Tabs Tabs;
struct Tabs {
	int n;
	int sel;
	struct {
		char *name;
		int w; /* 0 = fill */
	} tabs[TABS_MAX];
};

enum UiViews {
	UI_VIEW_MAIN,
	UI_VIEW_COLONIES,
	UI_VIEW_FLEETS,
	UI_VIEW_DESIGN,
	UI_VIEW_SYSTEMS,
	UI_VIEW_SETTINGS,
	UI_VIEW_LAST
};

enum UiElements {
	UI_TAB,
};

#define CLICKABLE_MAX 64
typedef struct Clickable Clickable;
struct Clickable {
	int x, y;
	int w, h;
	enum UiElements type;
	void *elem;
};

/* system.c */
typedef struct Polar Polar;
struct Polar {
	float r;
	float theta;
};
