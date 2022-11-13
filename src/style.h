/* undefining default raylib colours (copied from raylib.h v4.1 */
#undef LIGHTGRAY
#undef GRAY
#undef DARKGRAY
#undef YELLOW
#undef GOLD
#undef ORANGE
#undef PINK
#undef RED
#undef MAROON
#undef GREEN
#undef LIME
#undef DARKGREEN
#undef SKYBLUE
#undef BLUE
#undef DARKBLUE
#undef PURPLE
#undef VIOLET
#undef DARKPURPLE
#undef BEIGE
#undef BROWN
#undef DARKBROWN
#undef WHITE
#undef BLACK
#undef BLANK
#undef MAGENTA
#undef RAYWHITE

/* colours */
extern const Color col_fg;
extern const Color col_bg;
extern const Color col_altbg;
extern const Color col_border;
extern const Color col_info;
extern const Color col_orbit;
extern const Color col_body[BODY_LAST];

/* font */
#define FONT_SIZE	10

/* textures */
#define NO_TINT		((Color){ 0xff, 0xff, 0xff, 0xff })
