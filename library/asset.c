#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <assert.h>

#include "asset.h"
#include "asset_cache.h"
#include "color.h"
#include "sdl_wrapper.h"

typedef struct asset {
  asset_type_t type;
  SDL_Rect bounding_box;
} asset_t;

typedef struct text_asset {
  asset_t base;
  TTF_Font *font;
  const char *text;
  rgb_color_t color;
} text_asset_t;

typedef struct body_asset {
  asset_t base;
  body_t *body;
} body_asset_t;

typedef struct image_asset {
  asset_t base;
  SDL_Texture *texture;
  body_t *body;
} image_asset_t;

typedef struct button_asset {
  asset_t base;
  image_asset_t *image_asset;
  text_asset_t *text_asset;
  button_handler_t handler;
  bool is_rendered;
} button_asset_t;

/**
 * Allocates memory for an asset with the given parameters.
 *
 * @param ty the type of the asset
 * @param bounding_box the bounding box containing the location and dimensions
 * of the asset when it is rendered
 * @return a pointer to the newly allocated asset
 */
static asset_t *asset_init(asset_type_t ty, SDL_Rect bounding_box) {
  asset_t *new;
  switch (ty) {
  case ASSET_IMAGE: {
    new = malloc(sizeof(image_asset_t));
    break;
  }
  case ASSET_FONT: {
    new = malloc(sizeof(text_asset_t));
    break;
  }
  case ASSET_BUTTON: {
    new = malloc(sizeof(button_asset_t));
    break;
  }
  case ASSET_BODY: {
    new = malloc(sizeof(body_asset_t));
    break;
  }
  default: {
    assert(false && "Unknown asset type");
  }
  }
  assert(new);
  new->type = ty;
  new->bounding_box = bounding_box;
  return new;
}

asset_type_t asset_get_type(asset_t *asset) { return asset->type; }

asset_t *asset_make_body(body_t *body) {
  body_asset_t *body_asset = malloc(sizeof(body_asset_t));
  assert(body_asset);
  SDL_Rect box = bounding_box(body);
  body_asset->base = *asset_init(ASSET_BODY, box);
  body_asset->body = body;
  return (asset_t *)body_asset;
}

body_t *asset_get_body(asset_t *asset) {
  if (asset->type == ASSET_BODY) {
    body_asset_t *body_asset = (body_asset_t *)asset;
    return body_asset->body;
  }
  else if (asset->type == ASSET_IMAGE) {
    image_asset_t *image_asset = (image_asset_t *)asset;
    return image_asset->body;
  }
  return NULL;
}

asset_t *asset_make_image(const char *filepath, SDL_Rect bounding_box) {
  image_asset_t *img = malloc(sizeof(image_asset_t));
  assert(img != NULL);
  img->base = *asset_init(ASSET_IMAGE, bounding_box);
  img->texture =
      (SDL_Texture *)asset_cache_obj_get_or_create(ASSET_IMAGE, filepath);
  img->body = NULL;
  return (asset_t *)img;
}

asset_t *asset_make_image_with_body(const char *filepath, body_t *body) {
  SDL_Rect bbox = bounding_box(body);
  image_asset_t *img = malloc(sizeof(image_asset_t));
  assert(img != NULL);
  img->base = *asset_init(ASSET_IMAGE, bbox);
  img->texture =
      (SDL_Texture *)asset_cache_obj_get_or_create(ASSET_IMAGE, filepath);
  img->body = body;
  return (asset_t *)img;
}

asset_t *asset_make_text(const char *filepath, SDL_Rect bounding_box,
                         const char *text, rgb_color_t color) {
  text_asset_t *text_asset = malloc(sizeof(text_asset_t));
  assert(text_asset);
  text_asset->base = *asset_init(ASSET_FONT, bounding_box);
  text_asset->font =
      (TTF_Font *)asset_cache_obj_get_or_create(ASSET_FONT, filepath);
  text_asset->text = text;
  text_asset->color = color;
  return (asset_t *)text_asset;
}

asset_t *asset_make_button(SDL_Rect bounding_box, asset_t *image_asset,
                           asset_t *text_asset, button_handler_t handler) {
  button_asset_t *button = malloc(sizeof(button_asset_t));
  assert(button);
  button->base = *asset_init(ASSET_BUTTON, bounding_box);
  assert((void *)image_asset == NULL || image_asset->type == ASSET_IMAGE);
  assert((void *)text_asset == NULL || text_asset->type == ASSET_FONT);
  button->image_asset = (image_asset_t *)image_asset;
  button->text_asset = (text_asset_t *)text_asset;
  button->handler = handler;
  button->is_rendered = false;
  return (asset_t *)button;
}

void asset_on_button_click(asset_t *button, state_t *state, double x,
                           double y) {
  button_asset_t *button_asset = (button_asset_t *)button;
  if (button_asset->is_rendered &&
      sdl_contained_in_box(x, y, button->bounding_box)) {
    button_asset->handler(state);
    button_asset->is_rendered = false;
  }
}

void asset_render(asset_t *asset) {
  switch (asset->type) {
  case ASSET_BODY: {
    body_asset_t *body_asset = (body_asset_t *)asset;
    sdl_draw_polygon(body_get_polygon(body_asset->body), *body_get_color(body_asset->body));
    break;
  }
  case ASSET_IMAGE: {
    image_asset_t *image = (image_asset_t *)asset;
    if (image->body != NULL) {
      double cur_rot = body_get_rotation(image->body);
      if (cur_rot != 0) {
        body_set_rotation(image->body, 0);
        SDL_Rect box = bounding_box(image->body);
        body_set_rotation(image->body, cur_rot);
        vector_t body_vel = body_get_velocity(image->body);
        double rot = atan2(body_vel.y, body_vel.x);
        sdl_draw_image_with_angle(image->texture, box, rot);
      }
      else {
        SDL_Rect box = bounding_box(image->body);
        sdl_draw_image(image->texture, box);
      }
      
    } else {
      sdl_draw_image(image->texture, asset->bounding_box);
    }
    break;
  }
  case ASSET_FONT: {
    text_asset_t *text_asset = (text_asset_t *)asset;
    sdl_draw_text(text_asset->text, text_asset->font, text_asset->color,
                  asset->bounding_box);
    break;
  }
  case ASSET_BUTTON: {
    button_asset_t *button_asset = (button_asset_t *)asset;
    if (button_asset->image_asset != NULL) {
      asset_render((asset_t *)button_asset->image_asset);
    }
    if (button_asset->text_asset != NULL) {
      asset_render((asset_t *)button_asset->text_asset);
    }
    button_asset->is_rendered = true;
    break;
  }
  default: {
    fprintf(stderr, "Asset is of incompatible type.");
    break;
  }
  }
  sdl_show();
}

void asset_set_image(asset_t *asset, const char *filepath) {
  switch (asset->type) {
      case ASSET_IMAGE: {
        image_asset_t *image = (image_asset_t *)asset;
        image->texture = (SDL_Texture *)asset_cache_obj_get_or_create(ASSET_IMAGE, filepath);
        break;
      }
      case ASSET_BUTTON: {
        button_asset_t *button_asset = (button_asset_t *)asset;
        if (button_asset->image_asset != NULL) {
          button_asset->image_asset->texture = (SDL_Texture *)asset_cache_obj_get_or_create(ASSET_IMAGE, filepath);
        }
        break;
      }
      default: {
        printf("Can't set image of body or font\n");
        break;
      }
    }
}

void asset_destroy(asset_t *asset) { free(asset); }
