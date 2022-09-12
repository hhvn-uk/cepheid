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
		Vector2 origin;
	} ruler;
	float kmx, kmy;
	float kmperpx;
	struct {
		int x, y; /* real y = GetScreenHeight() - y */
		int w, h;
	} scale;
	System *sys;
} View_main;

typedef struct {
	System *sys;
	Body *selstar;
	Body *sel;
	struct {
		Checkbox planet;
		Checkbox moon;
		Checkbox dwarf;
		Checkbox asteroid;
		Checkbox comet;
		Checkbox nomineral;
	} show;
	struct {
		Pane stars;
		Pane bodies;
	} pane;
	Geom stars;
	Geom disp;
	Geom bodies;
	Geom loc;
	Geom mins;
	Geom hab;
} View_bodies;

typedef struct {
	struct {
		Geom geom;
	} info;
	int pan;
	Vector2 off;
	float lytopx;
	System *sel;
} View_sys;
