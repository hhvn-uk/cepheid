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

#define PUREWHITE	((Color){ 0xff, 0xff, 0xff, 0xff })
#define WHITE		((Color){ 0xe6, 0xe6, 0xe6, 0xff })
#define AURORA_GREY	((Color){ 0x1e, 0x1e, 0x1e, 0xff })
#define AURORA_BLUE	((Color){ 0x00, 0x00, 0x3c, 0xff })
#define DARKBLUE	((Color){ 0x00, 0x00, 0x2c, 0xff })
#define AURORA_GREEN	((Color){ 0x64, 0x96, 0x96, 0xff })
#define TRANSPARENT	((Color){ 0x00, 0x00, 0x00, 0x00 })
#define DUSTORANGE	((Color){ 0xbb, 0x88, 0x44, 0xff })
#define ICEBLUE		((Color){ 0x77, 0x77, 0xff, 0xff })

/* colours */
#define COL_FG		WHITE
#define COL_BG		AURORA_BLUE
#define COL_UNSELBG	DARKBLUE
#define COL_BORDER	AURORA_GREY
#define COL_INFO	AURORA_GREEN
#define COL_ORBIT	AURORA_GREEN
#define COL_STAR	DUSTORANGE
#define COL_COMET	ICEBLUE
#define COL_PLANET	WHITE
#define COL_MOON	WHITE
#define COL_DWARF	WHITE
#define COL_ASTEROID	AURORA_GREEN

/* font */
#define FONT_SIZE	10

/* textures */
#define NO_TINT		PUREWHITE /* shrug */
