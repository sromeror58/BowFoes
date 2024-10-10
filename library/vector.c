#include <math.h>
#include <stdlib.h>
#include <stdbool.h>
#include "vector.h"

const vector_t VEC_ZERO = {0, 0};

bool vec_equals(vector_t v1, vector_t v2) {
    return v1.x == v2.x && v1.y == v2.y;
}

vector_t vec_add(vector_t v1, vector_t v2) {
  vector_t result;
  result.x = v1.x + v2.x;
  result.y = v1.y + v2.y;
  return result;
};

vector_t vec_subtract(vector_t v1, vector_t v2) {
  vector_t result;
  result.x = v1.x - v2.x;
  result.y = v1.y - v2.y;
  return result;
};

vector_t vec_negate(vector_t v) {
  vector_t result;
  result.x = -1 * v.x;
  result.y = -1 * v.y;
  return result;
}

vector_t vec_multiply(double scalar, vector_t v) {
  vector_t result;
  result.x = scalar * v.x;
  result.y = scalar * v.y;
  return result;
}

double vec_dot(vector_t v1, vector_t v2) { return v1.x * v2.x + v1.y * v2.y; }

double vec_cross(vector_t v1, vector_t v2) { return v1.x * v2.y - v1.y * v2.x; }

vector_t vec_rotate(vector_t v, double angle) {
  vector_t result;
  result.x = v.x * cos(angle) - v.y * sin(angle);
  result.y = v.x * sin(angle) + v.y * cos(angle);
  return result;
}

double vec_get_length(vector_t v) {
  return sqrt(v.x*v.x + v.y*v.y);
}
