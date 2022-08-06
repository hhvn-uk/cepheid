#include <raylib.h>
#include "main.h"

#define IMAGE(name) \
	static Image raw_image_##name; \
	Texture image_##name

Font font;
IMAGE(tactical);
IMAGE(colonies);
IMAGE(fleet);
IMAGE(design);
IMAGE(sys);
IMAGE(settings);

unsigned char DejaVuSansMono_ttf[] = {
#include "../data/DejaVuSansMono.h"
};

unsigned char tactical_png[] = {
#include "../data/icons/tactical.h"
};

unsigned char colonies_png[] = {
#include "../data/icons/colonies.h"
};

unsigned char fleet_png[] = {
#include "../data/icons/fleet.h"
};

unsigned char design_png[] = {
#include "../data/icons/design.h"
};

unsigned char sys_png[] = {
#include "../data/icons/sys.h"
};

unsigned char settings_png[] = {
#include "../data/icons/settings.h"
};

#define IMAGE_LOAD(name) \
	raw_image_##name = LoadImageFromMemory(".png", \
			name##_png, sizeof(name##_png)); \
	image_##name = LoadTextureFromImage(raw_image_##name)

void
data_load(void) {
	font = LoadFontFromMemory(".ttf", DejaVuSansMono_ttf,
			sizeof(DejaVuSansMono_ttf), FONT_SIZE, NULL, 0);
	IMAGE_LOAD(tactical);
	IMAGE_LOAD(colonies);
	IMAGE_LOAD(fleet);
	IMAGE_LOAD(design);
	IMAGE_LOAD(sys);
	IMAGE_LOAD(settings);
}

#define IMAGE_UNLOAD(name) \
	UnloadTexture(image_##name); \
	UnloadImage(raw_image_##name);

void
data_unload(void) {
	UnloadFont(font);
	IMAGE_UNLOAD(tactical);
	IMAGE_UNLOAD(colonies);
	IMAGE_UNLOAD(fleet);
	IMAGE_UNLOAD(design);
	IMAGE_UNLOAD(sys);
	IMAGE_UNLOAD(settings);
}
