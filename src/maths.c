#include <raylib.h> /* RAD2DEG, DEG2RAD */
#include <math.h>
#include "maths.h"

#undef cosf
float
cosf_d(float x) {
	return cosf(RAD(x));
}

#undef sinf
float
sinf_d(float x) {
	return sinf(RAD(x));
}

#undef atan2f
float
atan2f_d(float y, float x) {
	return DEG(atan2f(y, x));
}
