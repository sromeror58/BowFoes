#include "list.h"
#include <assert.h>
#include <stdlib.h>

size_t const CAPACITY_FACTOR = 2;

typedef struct list {
  void **data;
  size_t capacity;
  size_t size;
  free_func_t freer;
} list_t;

list_t *list_init(size_t initial_size, free_func_t freer) {
  list_t *list = malloc(sizeof(list_t));
  assert(list != NULL);
  if (initial_size == 0) {
    initial_size++;
  }
  list->data = malloc((initial_size) * sizeof(void *));
  assert(list->data != NULL);
  list->capacity = initial_size;
  list->size = 0;
  list->freer = freer;
  return list;
}

void list_free(list_t *list) {
  if (list->freer != NULL) {
    for (size_t i = 0; i < list->size; i++) {
      list->freer(list->data[i]);
    }
  }
  free(list->data);
  free(list);
}

size_t list_size(list_t *list) { return list->size; }

void *list_get(list_t *list, size_t index) {
  assert(index < list_size(list));
  return list->data[index];
}

void list_add(list_t *list, void *value) {
  assert(list != NULL);
  assert(value != NULL);
  if (list->size >= list->capacity) {
    // Increasing capacity by constant factor to handle adding in more elements
    size_t new_capacity = (list->capacity) * CAPACITY_FACTOR;
    void **new_data = malloc(new_capacity * sizeof(void *));
    assert(new_data != NULL);
    for (size_t i = 0; i < list->size; i++) {
      new_data[i] = list->data[i];
    }
    free(list->data);
    list->capacity = new_capacity;
    list->data = new_data;
  }
  list->data[list->size] = value;
  list->size++;
}

void *list_remove(list_t *list, size_t index) {
  assert(list != NULL);
  assert(index < list->size);
  void *removed = list->data[index];
  // Removing element at a specific index, & shifting subsequent elements to the
  // left
  for (size_t i = index; i < list->size - 1; i++) {
    list->data[i] = list->data[i + 1];
  }
  list->size--;
  return removed;
}
