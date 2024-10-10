#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <assert.h>

#include "asset.h"
#include "asset_cache.h"
#include "list.h"
#include "sdl_wrapper.h"

static list_t *ASSET_CACHE;

const size_t FONT_SIZE = 18;
const size_t INITIAL_CAPACITY = 5;

typedef struct {
  asset_type_t type;
  const char *filepath;
  void *obj;
} entry_t;

static void asset_cache_free_entry(entry_t *entry) {
  switch (entry->type) {
  case ASSET_IMAGE:
    SDL_DestroyTexture((SDL_Texture *)entry->obj);
    break;
  case ASSET_FONT:
    TTF_CloseFont((TTF_Font *)entry->obj);
    break;
  case ASSET_BUTTON:
    asset_destroy((asset_t *)entry->obj);
    break;
  default:
    fprintf(stderr, "Unkown asset type.");
    break;
  }
  free(entry);
}

void asset_cache_init() {
  ASSET_CACHE =
      list_init(INITIAL_CAPACITY, (free_func_t)asset_cache_free_entry);
}

void asset_cache_destroy() { list_free(ASSET_CACHE); }

void *asset_cache_exist(const char *filepath) {
  for (size_t i = 0; i < list_size(ASSET_CACHE); i++) {
    entry_t *entry = list_get(ASSET_CACHE, i);
    if (entry->filepath != NULL && SDL_strcmp(entry->filepath, filepath) == 0) {
      return (void *)entry;
    }
  }
  return NULL;
}

void *asset_cache_obj_get_or_create(asset_type_t ty, const char *filepath) {
  entry_t *temp = asset_cache_exist(filepath);
  if (temp != NULL) {
    assert(temp->type == ty);
    return (void *)(temp->obj);
  } else {
    entry_t *new = malloc(sizeof(entry_t));
    assert(new);
    new->filepath = filepath;
    switch (ty) {
    case ASSET_IMAGE:
      new->type = ASSET_IMAGE;
      new->obj = load_image(filepath);
      break;
    case ASSET_FONT:
      new->type = ASSET_FONT;
      new->obj = load_font(filepath, FONT_SIZE);
      break;
    default:
      fprintf(stderr, "Type not found");
      break;
    }
    list_add(ASSET_CACHE, new);
    return new->obj;
  }
  return NULL;
}

void asset_cache_register_button(asset_t *button) {
  assert(asset_get_type(button) == ASSET_BUTTON);
  entry_t *new = malloc(sizeof(entry_t));
  assert(new);
  new->type = ASSET_BUTTON;
  new->obj = button;
  new->filepath = NULL;
  list_add(ASSET_CACHE, new);
}

void asset_cache_handle_buttons(state_t *state, double x, double y) {
  for (size_t i = 0; i < list_size(ASSET_CACHE); i++) {
    entry_t *entry = list_get(ASSET_CACHE, i);
    if (entry->type == ASSET_BUTTON) {
      asset_on_button_click(entry->obj, state, x, y);
    }
  }
}
