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
		Rect geom;
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
	struct {
		Rect geom;
	} info;
	int pan;
	Vector2 off;
	float lytopx;
	System *sel;
} View_sys;
