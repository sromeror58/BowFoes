// #include "scene.h"

// struct scene {
//   size_t num_bodies;
//   list_t *bodies;
//   list_t *force_creators;
// };

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "body.h"
#include "forces.h"
#include "list.h"
#include "polygon.h"
#include "scene.h"

size_t const BODY_N = 0;

struct scene {
  size_t num_bodies;
  list_t *bodies;
  list_t *force_creators;
  list_t *force_jobs;
};

scene_t *scene_init(void) {
  scene_t *new = malloc(sizeof(scene_t));
  assert(new != NULL);
  new->bodies = list_init(BODY_N, (free_func_t)body_free);
  new->num_bodies = 0;
  new->force_jobs = list_init(BODY_N, (free_func_t)forces_job_free);
  return new;
}

void scene_tick(scene_t *scene, double dt) {
  for (size_t i = 0; i < list_size(scene->force_jobs); i++) {
    forces_job_run(list_get(scene->force_jobs, i));
  }

  for (ssize_t i = 0; i < (ssize_t)(list_size(scene->bodies)); i++) {
    body_t *body = list_get(scene->bodies, i);
    // Case 1: the body is removed
    if (body_is_removed(body)) {
      for (size_t j = 0; j < list_size(scene->force_jobs); j++) {
        force_job_t *force_job = list_get(scene->force_jobs, j);
        list_t *bodies = force_job_get_bodies(force_job);

        for (size_t k = 0; k < list_size(bodies); k++) {
          body_t *force_body = list_get(bodies, k);
          if (force_body == body) {
            list_remove(scene->force_jobs, j);
            j--;
            forces_job_free(force_job);
            break;
          }
        }
      }
      list_remove(scene->bodies, i);
      i--;
      body_free(body);
      scene->num_bodies--;
    } else {
      // Case 2: the body isn't removed
      body_tick(body, dt);
    }
  }
}

void scene_add_force_creator(scene_t *scene, force_creator_t force_creator,
                             void *aux) {
  scene_add_bodies_force_creator(scene, force_creator, aux, list_init(0, free));
}

void scene_add_bodies_force_creator(scene_t *scene, force_creator_t forcer,
                                    void *aux, list_t *bodies) {
  // we need to keep track of `bodies` for each `forcer` somehow...
  force_job_t *new_force_job = forces_job_init(forcer, aux, bodies);
  list_add(scene->force_jobs, new_force_job);
}

void scene_free(scene_t *scene) {
  list_free(scene->bodies);
  list_free(scene->force_jobs);
  free(scene);
}

size_t scene_bodies(scene_t *scene) { return scene->num_bodies; }

body_t *scene_get_body(scene_t *scene, size_t index) {
  assert(index >= 0 && index < scene->num_bodies);
  return list_get(scene->bodies, index);
}

void scene_add_body(scene_t *scene, body_t *body) {
  list_add(scene->bodies, body);
  scene->num_bodies++;
}

void scene_remove_body(scene_t *scene, size_t index) {
  body_t *new = list_get(scene->bodies, index);
  body_remove(new);
}
