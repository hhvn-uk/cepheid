/* vim: set syntax=c : */

/* main/tactical */
typedef struct {
	struct {
		Tabs tabs;
		struct {
			Checkbox dwarf;
			Checkbox dwarfn;
			Checkbox asteroid;
			Checkbox asteroidn;
			Checkbox comet;
		} names;
		struct {
			Checkbox dwarf;
			Checkbox asteroid;
			Checkbox comet;
		} orbit;
		Checkbox comettail;
		Geom geom;
		Pane pane;
	} infobox;
	int pan;
	struct {
		int held;
		Vector origin;
	} ruler;
	float kmx, kmy;
	float kmperpx;
	struct {
		int x, y; /* real y = GetScreenHeight() - y */
		int w, h;
	} scale;
	System *sys;
} View_main;

/* bodies */
typedef struct {
	Body *sel;
	struct {
		Checkbox planet;
		Checkbox moon;
		Checkbox dwarf;
		Checkbox asteroid;
		Checkbox comet;
		Checkbox nomineral;
	} show;
	Geom disp;
	Geom bodies;
	Treeview tree;
	Geom loc;
	Geom mins;
	Geom hab;
	struct {
		Body *sel;
	} prevframe;
} View_bodies;

/* sys */
typedef struct {
	struct {
		Geom geom;
	} info;
	int pan;
	struct {
		float x, y;
		float topx;
	} ly;
	System *sel;
} View_sys;

/* smenu */
enum {
	SMENU_NEW,
	SMENU_SAVE,
	SMENU_CONT,
	SMENU_LOAD,
	SMENU_QUIT,
	SMENU_LAST,
	SMENU_BACK = -1,
};

struct Loadable {
	char *name;
	time_t mod;
};

typedef struct {
	Geom main;
	Button b[SMENU_LAST];
	struct {
		int disp;
		Input name;
		Button create;
		Button back;
	} new;
	struct {
		struct Loadable *save;
		char label[128];
	} cont;
	struct {
		int check;
		char *msg;
		Button back;
		Button save;
		Button discard;
		void (*func)(void);
	} save;
	struct {
		int init;
		int disp;
		Tree saves;
		Treeview savelist;
		Button back;
		Button delete;
		Button load;
	} load;
	Button *back;
} View_smenu;
