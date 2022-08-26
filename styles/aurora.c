#include "style.h"

/* Yeah, not exactly like aurora, but this is hardcoded style that sort of
 * resembled aurora before styles were split up */

#define WHITE		{ 0xe6, 0xe6, 0xe6, 0xff }
#define AURORA_BLUE	{ 0x00, 0x00, 0x3c, 0xff }
#define AURORA_GREEN	{ 0x64, 0x96, 0x96, 0xff }

const Color col_fg =		WHITE;
const Color col_bg = 		{ 0x00, 0x00, 0x3c, 0xff };
const Color col_unselbg = 	{ 0x00, 0x00, 0x2c, 0xff };
const Color col_border = 	{ 0x1e, 0x1e, 0x1e, 0xff };
const Color col_info =		AURORA_GREEN;
const Color col_orbit = 	AURORA_GREEN;

const Color col_body[] = {
	[BODY_STAR] =		{ 0xbb, 0x88, 0x44, 0xff },
	[BODY_PLANET] =		WHITE,
	[BODY_COMET] =		{ 0x77, 0x77, 0xff, 0xff },
	[BODY_DWARF] =		WHITE,
	[BODY_ASTEROID] =	AURORA_GREEN,
	[BODY_MOON] =		WHITE,
};
