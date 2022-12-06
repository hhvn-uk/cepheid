#include <math.h>
#include "main.h"

Vector
vectorize(Polar p) {
	return (Vector) {
		p.r * cosf_d(p.theta),
		p.r * sinf_d(p.theta)
	};
}

Vector
vectorize_at(Vector at, Polar p) {
	Vector r = vectorize(p);
	return (Vector) {
		at.x + r.x,
		at.y + r.y
	};
}

Vector
vector_diff(Vector a, Vector b) {
	return (Vector){
		fabsf(a.x - b.x),
		fabsf(a.y - b.y)
	};
}

float
vector_dist(Vector a, Vector b) {
	Vector d = vector_diff(a, b);
	return hypotf(d.x, d.y);
}

Polar
polarize(Vector v) {
	return (Polar) {
		hypotf(v.x, v.y),
		atan2f_d(v.y, v.x)
	};
}

Polar
polarize_at(Vector at, Vector vector) {
	Vector v = {vector.y - at.y, vector.x - at.x};
	return polarize(v);
}

#define SQ(x) (x * x)

/* The previous version of this function utilized other functions in this file,
 * however by avoiding the passing around of structs, this version is approx.
 * 3x faster */
Polar
polar_add(Polar abs, Polar rel) {
	return (Polar) {
		sqrtf(SQ(abs.r) + SQ(rel.r) + 2 * abs.r * rel.r * cosf_d(abs.theta - rel.theta)),
		abs.theta + atan2f_d(
				rel.r * sinf_d(rel.theta - abs.theta),
				abs.r + rel.r * cosf_d(rel.theta - abs.theta)
				)
	};
}
