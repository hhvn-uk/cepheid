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

#define X_PUREWHITE	0xffffffff
#define X_WHITE		0xe6e6e6ff
#define X_AURORA_GREY	0x1e1e1eff
#define X_AURORA_BLUE	0x00003cff
#define X_DARKBLUE      0x00002cff
#define X_AURORA_GREEN	0x649696ff
#define X_TRANSPARENT	0x00000000

#define UNHEX(col) ((Color){ \
		((col & (0xff << 24)) >> 24), \
		((col & (0xff << 16)) >> 16), \
		((col & (0xff << 8)) >> 8), \
		(col & 0xff)})

#define PUREWHITE	UNHEX(X_PUREWHITE)
#define WHITE		UNHEX(X_WHITE)
#define AURORA_GREY	UNHEX(X_AURORA_GREY)
#define AURORA_BLUE	UNHEX(X_AURORA_BLUE)
#define DARKBLUE	UNHEX(X_DARKBLUE)
#define AURORA_GREEN	UNHEX(X_AURORA_GREEN)
#define TRANSPARENT	UNHEX(X_TRANSPARENT)

/* colours */
#define COL_FG		WHITE
#define COL_BG		AURORA_BLUE
#define COL_UNSELBG	DARKBLUE
#define COL_BORDER	AURORA_GREY
#define COL_INFO	AURORA_GREEN
#define COL_ORBIT	AURORA_GREEN

/* font */
#define FONT_SIZE	10

/* textures */
#define NO_TINT		PUREWHITE /* shrug */
