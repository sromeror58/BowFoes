#include "sdl_wrapper.h"
#include "list.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_mixer.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

const char WINDOW_TITLE[] = "CS 3";
const int WINDOW_WIDTH = 1000;
const int WINDOW_HEIGHT = 500;
const double MS_PER_S = 1e3;
const size_t NUM_MIX_CHANNELS = 2;
const size_t MIX_FREQUENCY = 44100;
const size_t CHUNK_SIZE = 2048;
const ssize_t FIRST_FREE_CHANNEL = -1;
const size_t NO_LOOPS = 0;
const ssize_t INFINITE_LOOPS = -1;
const size_t NUM_BOX_POINTS = 4;

/**
 * The coordinate at the center of the screen.
 */
vector_t center;
/**
 * The coordinate difference from the center to the top right corner.
 */
vector_t max_diff;
/**
 * The SDL window where the scene is rendered.
 */
SDL_Window *window;
/**
 * The renderer used to draw the scene.
 */
SDL_Renderer *renderer;
/**
 * Mixers for playing music and sound effects
*/
Mix_Music *backgroundMusic = NULL;
Mix_Chunk* draw_bow_sound = NULL;
Mix_Chunk* shoot_sound = NULL;
Mix_Chunk* hit_sound = NULL;
/**
 * The keypress handler, or NULL if none has been configured.
 */
key_handler_t key_handler = NULL;
/**
 * SDL's timestamp when a key was last pressed or released.
 * Used to mesasure how long a key has been held.
 */
uint32_t key_start_timestamp;
/**
 * The value of clock() when time_since_last_tick() was last called.
 * Initially 0.
 */
clock_t last_clock = 0;

typedef struct sdl_mouse_handlers {
  state_mouse_handler_t start_screen_handler;
  state_mouse_handler_t skin_screen_handler;
  level_mouse_handler_t shot_start_handler;
  level_mouse_handler_t shot_drag_handler;
  level_mouse_handler_t shot_end_handler;
  state_mouse_handler_t state_game_over_handler;
} sdl_mouse_handlers_t;

/**
 * All the mouse handlers, or NULL if none has been configured.
 */
sdl_mouse_handlers_t mouse_handlers;

/** Computes the center of the window in pixel coordinates */
vector_t get_window_center(void) {
  int *width = malloc(sizeof(*width)), *height = malloc(sizeof(*height));
  assert(width != NULL);
  assert(height != NULL);
  SDL_GetWindowSize(window, width, height);
  vector_t dimensions = {.x = *width, .y = *height};
  free(width);
  free(height);
  return vec_multiply(0.5, dimensions);
}

/**
 * Computes the scaling factor between scene coordinates and pixel coordinates.
 * The scene is scaled by the same factor in the x and y dimensions,
 * chosen to maximize the size of the scene while keeping it in the window.
 */
double get_scene_scale(vector_t window_center) {
  // Scale scene so it fits entirely in the window
  double x_scale = window_center.x / max_diff.x,
         y_scale = window_center.y / max_diff.y;
  return x_scale < y_scale ? x_scale : y_scale;
}

/** Maps a scene coordinate to a window coordinate */
vector_t get_window_position(vector_t scene_pos, vector_t window_center) {
  // Scale scene coordinates by the scaling factor
  // and map the center of the scene to the center of the window
  vector_t scene_center_offset = vec_subtract(scene_pos, center);
  double scale = get_scene_scale(window_center);
  vector_t pixel_center_offset = vec_multiply(scale, scene_center_offset);
  vector_t pixel = {.x = round(window_center.x + pixel_center_offset.x),
                    // Flip y axis since positive y is down on the screen
                    .y = round(window_center.y - pixel_center_offset.y)};
  return pixel;
}

/**
 * Converts an SDL key code to a char.
 * 7-bit ASCII characters are just returned
 * and arrow keys are given special character codes.
 */
char get_keycode(SDL_Keycode key) {
  switch (key) {
  case SDLK_LEFT:
    return LEFT_ARROW;
  case SDLK_UP:
    return UP_ARROW;
  case SDLK_RIGHT:
    return RIGHT_ARROW;
  case SDLK_DOWN:
    return DOWN_ARROW;
  case SDLK_SPACE:
    return SPACE_BAR;
  default:
    // Only process 7-bit ASCII characters
    return key == (SDL_Keycode)(char)key ? key : '\0';
  }
}

void sdl_set_music_volume(size_t volume) {
  Mix_VolumeMusic(volume);
}

void sdl_init(vector_t min, vector_t max) {
  // Check parameters
  assert(min.x < max.x);
  assert(min.y < max.y);

  center = vec_multiply(0.5, vec_add(min, max));
  max_diff = vec_subtract(max, center);
  SDL_Init(SDL_INIT_EVERYTHING);
  window = SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT,
                            SDL_WINDOW_RESIZABLE);
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
  TTF_Init();
  Mix_Init(MIX_INIT_MP3 | MIX_INIT_OGG);
  Mix_OpenAudio(MIX_FREQUENCY, AUDIO_S16SYS, NUM_MIX_CHANNELS, CHUNK_SIZE);
  backgroundMusic = Mix_LoadMUS("assets/gamemusic.wav");
  draw_bow_sound = Mix_LoadWAV("assets/draw_bow_sound.wav");
  shoot_sound = Mix_LoadWAV("assets/shoot_sound.wav");
  hit_sound = Mix_LoadWAV("assets/hit_sound.wav");
  Mix_PlayMusic(backgroundMusic, INFINITE_LOOPS);
  sdl_set_music_volume(MIX_MAX_VOLUME);
}

void sdl_set_mouse_handlers(state_mouse_handler_t screen_switch, state_mouse_handler_t skin_screen,level_mouse_handler_t start, level_mouse_handler_t drag, level_mouse_handler_t end, state_mouse_handler_t game_over) {
  mouse_handlers = (sdl_mouse_handlers_t){screen_switch, skin_screen, start, drag, end, game_over};
}

bool sdl_is_done(void *state) {
  SDL_Event *event = malloc(sizeof(*event));
  level_t *cur_level = state_current_level(state);
  assert(event != NULL);
  while (SDL_PollEvent(event)) {
    switch (event->type) {
    case SDL_QUIT:
      free(event);
      return true;
    case SDL_KEYDOWN:
    case SDL_KEYUP:
      // Skip the keypress if no handler is configured
      // or an unrecognized key was pressed
      if (key_handler == NULL)
        break;
      char key = get_keycode(event->key.keysym.sym);
      if (key == '\0')
        break;

      uint32_t timestamp = event->key.timestamp;
      if (!event->key.repeat) {
        key_start_timestamp = timestamp;
      }
      key_event_type_t type =
          event->type == SDL_KEYDOWN ? KEY_PRESSED : KEY_RELEASED;
      double held_time = (timestamp - key_start_timestamp) / MS_PER_S;
      key_handler(key, type, held_time, state);
      break;
    case SDL_MOUSEBUTTONDOWN: {
      if ((state_get_screen(state) != START_SCENE && state_get_screen(state) != SKIN_SCREEN) && !level_game_over(cur_level) && !level_bullet_in_scene(cur_level) && (!level_get_use_ai(cur_level) || level_get_turn(cur_level))) {
        mouse_handlers.shot_start_handler(cur_level, event->motion.x, event->motion.y);
      }
      break;
    }
    case SDL_MOUSEMOTION: {
      if ((state_get_screen(state) != START_SCENE && state_get_screen(state) != SKIN_SCREEN) && !level_game_over(cur_level) && !level_bullet_in_scene(cur_level) && (!level_get_use_ai(cur_level) || level_get_turn(cur_level))) {
        mouse_handlers.shot_drag_handler(cur_level, event->motion.x, event->motion.y);
      }
      break;
    }
    case SDL_MOUSEBUTTONUP: {
      if ((state_get_screen(state) != START_SCENE && state_get_screen(state) != SKIN_SCREEN)) {
          if (level_game_over(cur_level)) {
            mouse_handlers.state_game_over_handler(state, event->motion.x, event->motion.y);
          }
          else if (!level_bullet_in_scene(cur_level) && (!level_get_use_ai(cur_level) || level_get_turn(cur_level))) {
            mouse_handlers.shot_end_handler(cur_level, event->motion.x, event->motion.y);
          }
      }
      else if (state_get_screen(state) == SKIN_SCREEN) {
        mouse_handlers.skin_screen_handler(state, event->motion.x, event->motion.y);
      }
      else {
        mouse_handlers.start_screen_handler(state, event->motion.x, event->motion.y);
      }
      break;
    }
    }
  }
  free(event);
  return false;
}

void sdl_clear(void) {
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  SDL_RenderClear(renderer);
}

void sdl_draw_polygon(polygon_t *poly, rgb_color_t color) {
  list_t *points = polygon_get_points(poly);
  // Check parameters
  size_t n = list_size(points);
  assert(n >= 3);

  vector_t window_center = get_window_center();

  // Convert each vertex to a point on screen
  int16_t *x_points = malloc(sizeof(*x_points) * n),
          *y_points = malloc(sizeof(*y_points) * n);
  assert(x_points != NULL);
  assert(y_points != NULL);
  for (size_t i = 0; i < n; i++) {
    vector_t *vertex = list_get(points, i);
    vector_t pixel = get_window_position(*vertex, window_center);
    x_points[i] = pixel.x;
    y_points[i] = pixel.y;
  }

  // Draw polygon with the given color
  filledPolygonRGBA(renderer, x_points, y_points, n, color.r * 255,
                    color.g * 255, color.b * 255, 255);
  free(x_points);
  free(y_points);
}

void sdl_show(void) {
  // Draw boundary lines
  vector_t window_center = get_window_center();
  vector_t max = vec_add(center, max_diff),
           min = vec_subtract(center, max_diff);
  vector_t max_pixel = get_window_position(max, window_center),
           min_pixel = get_window_position(min, window_center);
  SDL_Rect *boundary = malloc(sizeof(*boundary));
  boundary->x = min_pixel.x;
  boundary->y = max_pixel.y;
  boundary->w = max_pixel.x - min_pixel.x;
  boundary->h = min_pixel.y - max_pixel.y;
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderDrawRect(renderer, boundary);
  free(boundary);
  SDL_RenderPresent(renderer);
}

void sdl_render_scene(scene_t *scene, void *aux) {
  sdl_clear();
  size_t body_count = scene_bodies(scene);
  for (size_t i = 0; i < body_count; i++) {
    body_t *body = scene_get_body(scene, i);
    list_t *shape = body_get_shape(body);
    polygon_t *poly = polygon_init(shape, (vector_t){0, 0}, 0, 0, 0, 0);
    sdl_draw_polygon(poly, *body_get_color(body));
    list_free(shape);
  }
  if (aux != NULL) {
    body_t *body = aux;
    sdl_draw_polygon(body_get_polygon(body), *body_get_color(body));
  }
  sdl_show();
}

void sdl_on_key(key_handler_t handler) { key_handler = handler; }

double time_since_last_tick(void) {
  clock_t now = clock();
  double difference = last_clock
                          ? (double)(now - last_clock) / CLOCKS_PER_SEC
                          : 0.0; // return 0 the first time this is called
  last_clock = now;
  return difference;
}

SDL_Texture *load_image(const char *image_path) {
  if (renderer == NULL) {
    return NULL;
  }
  return IMG_LoadTexture(renderer, image_path); // load the meme from assets
}

void sdl_draw_image(SDL_Texture *img, SDL_Rect bounds) {
  SDL_RenderCopy(renderer, img, NULL, &bounds);
}

void sdl_draw_image_with_angle(SDL_Texture *img, SDL_Rect bounds, double rot) {
  SDL_RenderCopyEx(renderer, img, NULL, &bounds, (-1.0)*(rot*180)/M_PI, NULL, SDL_FLIP_NONE);
}

bool sdl_is_mouse_click(void) {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_MOUSEBUTTONDOWN) {
      return true;
    }
  }
  return false;
}

void sdl_play_sound_effect(sound_effect_t sound_effect) {
  switch (sound_effect) {
    case DRAW_BOW:
      Mix_PlayChannel(FIRST_FREE_CHANNEL, draw_bow_sound, NO_LOOPS);
      break;
    case SHOOT:
      Mix_PlayChannel(FIRST_FREE_CHANNEL, shoot_sound, NO_LOOPS);
      break;
    case HIT:
      Mix_PlayChannel(FIRST_FREE_CHANNEL, hit_sound, NO_LOOPS);
      break;
  }
}

TTF_Font *load_font(const char *font_style_path, size_t font_size) {
  TTF_Init();
  TTF_Font *font_style = TTF_OpenFont(font_style_path, font_size);
  if (font_style == NULL) {
    fprintf(stderr, "Error: Failed to load font - %s\n", TTF_GetError());
  }
  return font_style;
}

void sdl_draw_text(const char *text, TTF_Font *font, rgb_color_t color,
                   SDL_Rect bounds) {
  SDL_Color *new_color = malloc(sizeof(SDL_Color));
  assert(new_color);
  new_color->r = color.r;
  new_color->g = color.g;
  new_color->b = color.b;
  int w, h;
  SDL_Surface *text_surface = TTF_RenderText_Solid(font, text, *new_color);
  SDL_Texture *text_texture =
      SDL_CreateTextureFromSurface(renderer, text_surface);
  SDL_QueryTexture(text_texture, NULL, NULL, &w, &h);
  bounds.w = w;
  bounds.h = h;
  SDL_RenderCopy(renderer, text_texture, NULL, &bounds);
}

SDL_Rect sdl_get_bounds(size_t h, size_t w, size_t x, size_t y) {
  SDL_Rect output;
  output.h = h;
  output.w = w;
  output.x = x;
  output.y = y;
  return output;
}

SDL_Rect bounding_box(body_t *body) {
  list_t *shape = body_get_shape(body);
  vector_t min = {.x = __DBL_MAX__, .y = __DBL_MAX__};
  vector_t max = {.x = -__DBL_MAX__, .y = -__DBL_MAX__};
  for (size_t i = 0; i < list_size(shape); i++) {
    vector_t p = *(vector_t *)list_get(shape, i);
    if (p.x < min.x)
      min.x = p.x;
    if (p.y < min.y)
      min.y = p.y;
    if (p.x > max.x)
      max.x = p.x;
    if (p.y > max.y)
      max.y = p.y;
  }

  vector_t MAX = {.x = WINDOW_WIDTH, .y = WINDOW_HEIGHT};
  SDL_Rect box;
  box.x = (int)min.x;
  box.y = (int)(MAX.y - max.y);
  box.w = (int)(max.x - min.x);
  box.h = (int)(max.y - min.y);
  return box;
}

bool sdl_contained_in_box(double x, double y, SDL_Rect bounding_box) {
  return x >= bounding_box.x && x <= (bounding_box.x + bounding_box.w) &&
         y >= bounding_box.y && y <= (bounding_box.y + bounding_box.h);
}

list_t *sdl_make_rectangle(double x, double y, double w, double h) {
  list_t *shape = list_init(NUM_BOX_POINTS, free);
  vector_t *top_left = malloc(sizeof(vector_t));
  assert (top_left != NULL);
  *top_left = (vector_t){x, y};
  list_add(shape, top_left);

  vector_t *top_right = malloc(sizeof(vector_t));
  assert (top_right != NULL);
  *top_right = (vector_t){x + w, y};
  list_add(shape, top_right);

  vector_t *bottom_right = malloc(sizeof(vector_t));
  assert (bottom_right != NULL);
  *bottom_right = (vector_t){x + w, y - h};
  list_add(shape, bottom_right);

  vector_t *bottom_left = malloc(sizeof(vector_t));
  assert (bottom_left != NULL);
  *bottom_left = (vector_t){x, y - h};
  list_add(shape, bottom_left);

  return shape;
}

