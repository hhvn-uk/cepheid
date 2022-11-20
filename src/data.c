#include <raylib.h>
#include "main.h"

#include "../data/DejaVuSansMono.h"
#include "../data/icons/tactical.h"
#include "../data/icons/colonies.h"
#include "../data/icons/bodies.h"
#include "../data/icons/fleet.h"
#include "../data/icons/design.h"
#include "../data/icons/sys.h"
#include "../data/icons/settings.h"
#include "../data/icons/burger.h"
#include "../data/splash.h"

#define IMAGE(name) \
	static Image raw_image_##name; \
	Texture image_##name

Font font;
IMAGE(tactical);
IMAGE(colonies);
IMAGE(bodies);
IMAGE(fleet);
IMAGE(design);
IMAGE(sys);
IMAGE(settings);
IMAGE(burger);
IMAGE(splash);

#define IMAGE_LOAD(name) \
	raw_image_##name = LoadImageFromMemory(".png", \
			name##_png, name##_png_size); \
	image_##name = LoadTextureFromImage(raw_image_##name); \
	UnloadImage(raw_image_##name)

void
data_load(void) {
	font = LoadFontFromMemory(".ttf", DejaVuSansMono_ttf,
			DejaVuSansMono_ttf_size, FONT_SIZE, NULL, 0);
	charpx = MeasureTextEx(font, ".", FONT_SIZE, FONT_SIZE/10).x + FONT_SIZE/10;
	/* one step per IMAGE_LOAD() */
	IMAGE_LOAD(tactical);
	IMAGE_LOAD(colonies);
	IMAGE_LOAD(bodies);
	IMAGE_LOAD(fleet);
	IMAGE_LOAD(design);
	IMAGE_LOAD(sys);
	IMAGE_LOAD(settings);
	IMAGE_LOAD(burger);
	IMAGE_LOAD(splash);
}

#define IMAGE_UNLOAD(name) \
	UnloadTexture(image_##name)

void
data_unload(void) {
	UnloadFont(font);
	IMAGE_UNLOAD(tactical);
	IMAGE_UNLOAD(colonies);
	IMAGE_UNLOAD(bodies);
	IMAGE_UNLOAD(fleet);
	IMAGE_UNLOAD(design);
	IMAGE_UNLOAD(sys);
	IMAGE_UNLOAD(settings);
	IMAGE_UNLOAD(burger);
	IMAGE_UNLOAD(splash);
}
