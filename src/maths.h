#define YOTTA (ZETTA * 1000)
#define ZETTA (EXA * 1000)
#define EXA (PETA * 1000)
#define PETA (TERA * 1000)
#define TERA (GIGA * 1000)
#define GIGA (MEGA * 1000)
#define MEGA (KILO * 1000)
#define KILO 1000

#define C_MS 299792458 /* c, in m/s */

#define SEC_LEN 1
#define MIN_LEN (60 * SEC_LEN)
#define HOUR_LEN (60 * MIN_LEN)
#define DAY_LEN (24 * HOUR_LEN)
#define WEEK_LEN (7 * DAY_LEN)
#define YEAR_LEN (365.25 * DAY_LEN)
#define MONTH_APPROX (YEAR_LEN / 12)

#define SQUARE(a) (a * a)
