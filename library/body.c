#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "body.h"

struct body {
  polygon_t *poly;
  double mass;
  vector_t force;
  vector_t impulse;
  bool removed;
  void *info;
  free_func_t info_freer;
  bool rotate_with_velocity;
};

const double INITIAL_ROTSPEED = 0;
const double VELOCITY_AVG_FACTOR = 0.5;

body_t *body_init(list_t *shape, double mass, rgb_color_t color) {
  return body_init_with_info(shape, mass, color, NULL, NULL);
}

body_t *body_init_with_info(list_t *shape, double mass, rgb_color_t color,
                            void *info, free_func_t info_freer) {
  body_t *new = malloc(sizeof(body_t));
  assert(new != NULL);

  new->poly = polygon_init(shape, VEC_ZERO, INITIAL_ROTSPEED, color.r, color.g,
                           color.b);
  new->mass = mass;
  new->force = VEC_ZERO;
  new->impulse = VEC_ZERO;
  new->removed = false;
  new->rotate_with_velocity = false;
  new->info = info;
  new->info_freer = info_freer;
  return new;
}

polygon_t *body_get_polygon(body_t *body) { return body->poly; }

void *body_get_info(body_t *body) { return body->info; }

void body_free(body_t *body) {
  if (body->info_freer != NULL && body->info != NULL) {
    body->info_freer(body->info);
  }
  polygon_free(body->poly);
  free(body);
}

list_t *body_get_shape(body_t *body) {
  list_t *shape = polygon_get_points(body->poly);
  list_t *returned = list_init(list_size(shape), free);
  assert(returned);
  for (size_t i = 0; i < list_size(shape); i++) {
    vector_t *vertex = malloc(sizeof(vector_t));
    assert(vertex);
    *vertex = *(vector_t *)list_get(shape, i);
    list_add(returned, vertex);
  }
  return returned;
}

vector_t body_get_centroid(body_t *body) {
  return polygon_get_center(body->poly);
}

vector_t body_get_velocity(body_t *body) {
  vector_t *return_vector = polygon_get_velocity(body->poly);
  return *return_vector;
}

rgb_color_t *body_get_color(body_t *body) {
  return polygon_get_color(body->poly);
}

void body_set_color(body_t *body, rgb_color_t *col) {
  polygon_set_color(body->poly, col);
}

void body_set_shape(body_t *body, list_t *shape) {
  polygon_t *cur_poly = body->poly;
  vector_t *cur_vel = polygon_get_velocity(cur_poly);
  double cur_rotation = polygon_get_rotation(cur_poly);
  rgb_color_t *cur_color = polygon_get_color(cur_poly);
  polygon_t *new_poly = polygon_init(shape, *cur_vel, cur_rotation, (*cur_color).r, (*cur_color).g, (*cur_color).b);
  body->poly = new_poly;
  polygon_free(cur_poly);
}

void body_set_centroid(body_t *body, vector_t x) {
  polygon_set_center(body->poly, x);
}

void body_set_velocity(body_t *body, vector_t v) {
  polygon_set_velocity(body->poly, v);
}

void body_set_rotate_with_velocity(body_t *body, bool rotate_with_velocity) {
  body->rotate_with_velocity = rotate_with_velocity;
}

double body_get_rotation(body_t *body) {
  return polygon_get_rotation(body->poly);
}

void body_set_rotation(body_t *body, double angle) {
  polygon_set_rotation(body->poly, angle);
}

/**
 * Gets the new velocity of the body after applying forces and impulses on
 * the body after an elapsed time of dt.
 *
 * @param body a pointer to a body returned from body_init()
 * @param dt the number of seconds elapsed since the last tick
 */
vector_t get_new_velocity(body_t *body, double dt) {
  vector_t acceleration_force = vec_multiply(1.0 / body->mass, body->force);
  vector_t velocity_change_impulse =
      vec_multiply(1.0 / body->mass, body->impulse);
  vector_t velocity_change_force = vec_multiply(dt, acceleration_force);
  vector_t velocity_change =
      vec_add(velocity_change_impulse, velocity_change_force);
  vector_t new_velocity = vec_add(body_get_velocity(body), velocity_change);
  return new_velocity;
}

void body_tick(body_t *body, double dt) {
  vector_t new_velocity = get_new_velocity(body, dt);
  vector_t update_velocity = vec_multiply(
      VELOCITY_AVG_FACTOR, vec_add(body_get_velocity(body), new_velocity));
  vector_t displacement = vec_multiply(dt, update_velocity);
  polygon_translate(body->poly, displacement);
  if (body->rotate_with_velocity) {
    polygon_set_rotation(body->poly, atan2(new_velocity.y, new_velocity.x));
  }
  body_set_velocity(body, new_velocity);
  body->force = VEC_ZERO;
  body->impulse = VEC_ZERO;
}

double body_get_mass(body_t *body) { return body->mass; }

void body_add_force(body_t *body, vector_t force) {
  body->force = vec_add(body->force, force);
}

void body_add_impulse(body_t *body, vector_t impulse) {
  body->impulse = vec_add(body->impulse, impulse);
}

void body_remove(body_t *body) { body->removed = true; }

bool body_is_removed(body_t *body) { return body->removed; }

void body_reset(body_t *body) {
  body->force = VEC_ZERO;
  body->impulse = VEC_ZERO;
}