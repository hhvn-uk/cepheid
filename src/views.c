#include "main.h"

void (*view_init[VIEW_LAST])(void) = {
	[VIEW_MAIN] = view_main_init,
	[VIEW_COLONIES] = NULL,
	[VIEW_BODIES] = NULL,
	[VIEW_FLEETS] = NULL,
	[VIEW_DESIGN] = NULL,
	[VIEW_SYSTEMS] = NULL,
	[VIEW_SETTINGS] = NULL,
};

void (*view_handle[VIEW_LAST])(int) = {
	[VIEW_SMENU] = view_smenu_handle,
	[VIEW_MAIN] = view_main_handle,
	[VIEW_COLONIES] = view_colonies_handle,
	[VIEW_BODIES] = view_bodies_handle,
	[VIEW_FLEETS] = view_fleets_handle,
	[VIEW_DESIGN] = view_design_handle,
	[VIEW_SYSTEMS] = view_sys_handle,
	[VIEW_SETTINGS] = view_settings_handle,
};

void (*view_draw[VIEW_LAST])(void) = {
	[VIEW_SMENU] = view_smenu_draw,
	[VIEW_MAIN] = view_main_draw,
	[VIEW_COLONIES] = view_colonies_draw,
	[VIEW_BODIES] = view_bodies_draw,
	[VIEW_FLEETS] = view_fleets_draw,
	[VIEW_DESIGN] = view_design_draw,
	[VIEW_SYSTEMS] = view_sys_draw,
	[VIEW_SETTINGS] = view_settings_draw,
};

Tabs view_tabs = {
	/* Tactical is the terminology used in Aurora, so I decided to use it
	 * in the ui; in the code it's just called 'main' for my fingers' sake */
	VIEW_LAST, 0, {
		{&image_burger, NULL, VIEWS_HEIGHT},
		{&image_tactical, "Tactical", 0},
		{&image_colonies, "Colonies", 0},
		{&image_bodies, "Bodies", 0},
		{&image_fleet, "Fleets", 0},
		{&image_design, "Design", 0},
		{&image_sys, "Systems", 0},
		{&image_settings, "Settings", 0},
	}
};

void
view_colonies_handle(int nowsel) {
	if (nowsel)
		ui_title("Colonies");
}

void
view_fleets_handle(int nowsel) {
	if (nowsel)
		ui_title("Fleets");
}

void
view_design_handle(int nowsel) {
	if (nowsel)
		ui_title("Design");
}

void
view_settings_handle(int nowsel) {
	if (nowsel)
		ui_title("Settings");
}

void
view_colonies_draw(void) {
	ui_print(PAD, VIEWS_HEIGHT + PAD, col_fg, "Stars/colonies here");
	ui_print(GetScreenWidth() / 2, VIEWS_HEIGHT + PAD, col_fg, "Tabs here");
	ui_print(GetScreenWidth() / 2, GetScreenHeight() / 2, col_fg, "Management stuff here");
}


void
view_fleets_draw(void) {
	ui_print(PAD, VIEWS_HEIGHT + PAD, col_fg, "Groups/fleets/subfleets/ships here");
	ui_print(GetScreenWidth() / 2, VIEWS_HEIGHT + PAD, col_fg, "Tabs here");
	ui_print(GetScreenWidth() / 2, GetScreenHeight() / 2, col_fg, "Management stuff here");
}

void
view_design_draw(void) {
	ui_print(PAD, VIEWS_HEIGHT + PAD, col_fg, "Designations/classes here");
	ui_print(GetScreenWidth() / 4, VIEWS_HEIGHT + PAD, col_fg, "Selectable components here");
	ui_print((GetScreenWidth() / 4) * 2, VIEWS_HEIGHT + PAD, col_fg, "Selected components");
	ui_print((GetScreenWidth() / 4) * 3, VIEWS_HEIGHT + PAD, col_fg, "Class info");
}

void
view_settings_draw(void) {
	ui_print(PAD, VIEWS_HEIGHT + PAD, col_fg, "Settings here");
}
