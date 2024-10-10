#include "polygon.h"
#include "color.h"
#include "list.h"
#include "vector.h"
#include <assert.h>
#include <math.h>
#include <stdlib.h>

typedef struct polygon {
  list_t *points;
  vector_t velocity;
  double rotation_speed;
  rgb_color_t *color;
  double rotation;
} polygon_t;

size_t const CENTROID_SCALE = 6;
double const INITIAL_ROTANG = 0;

polygon_t *polygon_init(list_t *points, vector_t initial_velocity,
                        double rotation_speed, double red, double green,
                        double blue) {
  polygon_t *polygon = malloc(sizeof(polygon_t));
  assert(polygon != NULL);
  polygon->points = points;
  polygon->velocity = initial_velocity;
  polygon->rotation_speed = rotation_speed;
  polygon->color = color_init(red, green, blue);
  polygon->rotation = INITIAL_ROTANG;
  return polygon;
}

list_t *polygon_get_points(polygon_t *polygon) { return polygon->points; }

void polygon_move(polygon_t *polygon, double time_elapsed) {
  vector_t displacement = vec_multiply(time_elapsed, polygon->velocity);
  polygon_translate(polygon, displacement);
  polygon->rotation += polygon->rotation_speed * time_elapsed;
  polygon_rotate(polygon, polygon_get_rotation(polygon),
                 polygon_centroid(polygon));
}

void polygon_set_velocity(polygon_t *polygon, vector_t vel) {
  polygon->velocity.x = vel.x;
  polygon->velocity.y = vel.y;
}

void polygon_free(polygon_t *polygon) {
  list_t *points = polygon->points;
  color_free(polygon->color);
  list_free(points);
  free(polygon);
}

vector_t *polygon_get_velocity(polygon_t *polygon) {
  return &polygon->velocity;
}

double polygon_area(polygon_t *polygon) {
  double area = 0;
  size_t size = list_size(polygon->points);
  if (size < 3)
    return area;

  // shoelace formula
  for (size_t i = 0; i < size; i++) {
    size_t next_idx = (i + 1) % size;
    vector_t *p1 = list_get(polygon->points, i);
    vector_t *p2 = list_get(polygon->points, next_idx);
    area += vec_cross(*p1, *p2);
  }
  return 0.5 * fabs(area);
}

vector_t polygon_centroid(polygon_t *polygon) {
  double signed_area = 0;
  size_t size = list_size(polygon->points);
  vector_t centroid = {0, 0};

  // signed area using shoelace formula
  for (size_t i = 0; i < size; i++) {
    size_t next_idx = (i + 1) % size;
    vector_t *current = list_get(polygon->points, next_idx);
    vector_t *prev = list_get(polygon->points, i);
    double cross_prod = vec_cross(*prev, *current);
    signed_area += cross_prod;
    centroid.x += (current->x + prev->x) * cross_prod;
    centroid.y += (current->y + prev->y) * cross_prod;
  }

  signed_area *= 0.5;
  centroid.x /= CENTROID_SCALE * signed_area;
  centroid.y /= CENTROID_SCALE * signed_area;
  return centroid;
}

void polygon_translate(polygon_t *polygon, vector_t translation) {
  for (size_t i = 0; i < list_size(polygon->points); i++) {
    vector_t *vertex = list_get(polygon->points, i);
    *vertex = vec_add(*vertex, translation);
  }
}

void polygon_rotate(polygon_t *polygon, double angle, vector_t point) {
  // need to rotate around origin first
  polygon_translate(polygon, vec_negate(point));

  for (size_t i = 0; i < list_size(polygon->points); i++) {
    vector_t *vertex = list_get(polygon->points, i);
    *vertex = vec_rotate(*vertex, angle);
  }

  // then translate back to effectively rotate around point
  polygon_translate(polygon, point);
}

rgb_color_t *polygon_get_color(polygon_t *polygon) { return polygon->color; }

void polygon_set_color(polygon_t *polygon, rgb_color_t *color) {
  rgb_color_t *prev = polygon->color;
  polygon->color = color;
  color_free(prev);
}

void polygon_set_center(polygon_t *polygon, vector_t centroid) {
  polygon_translate(polygon,
                    vec_subtract(centroid, polygon_get_center(polygon)));
}

vector_t polygon_get_center(polygon_t *polygon) {
  return polygon_centroid(polygon);
}

void polygon_set_rotation(polygon_t *polygon, double rot) {
  polygon_rotate(polygon, rot - polygon->rotation, polygon_get_center(polygon));
  polygon->rotation = rot;
}

double polygon_get_rotation(polygon_t *polygon) { return polygon->rotation; }
