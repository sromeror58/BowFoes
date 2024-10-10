#include <assert.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>

#include "level.h"
#include "character.h"
#include "collision.h"
#include "state.h"
#include "asset.h"
#include "forces.h"
#include "asset_cache.h"
#include "color.h"
#include "sdl_wrapper.h"

// Level
const vector_t SCREEN_MIN = {0, 0};
const vector_t SCREEN_MAX = {1000, 500};
const size_t ASSET_MEMORY = 12;
const ssize_t LEFT_WALL_X = -999;
const size_t RIGHT_WALL_X_OFFSET = 2;
const size_t WALL_HEIGHT_FACTOR = 3;
const size_t WALL_WIDTH_LEFT = 1001;
const size_t WALL_WIDTH_RIGHT = 1000;
const size_t GROUND_Y = 2;
const size_t GROUND_HEIGHT = 1000;

// Bullet
const size_t BULLET_MEMORY = 10;
const char *BULLET_PATH = "assets/arrow.png";
const double BULLET_MASS = 10;
const double BULLET_WIDTH = 50;
const double BULLET_HEIGHT = 20;
const vector_t BULLET_INITIAL_POSITION_1 = {150, 90};
const vector_t BULLET_INITIAL_POSITION_2 = {850, 90};
const double SHOT_MAX_SPEED = 600;
const double BULLET_ELASTICITY = 0;
const rgb_color_t BLACK = (rgb_color_t){0, 0, 0};

// Helper dots
const size_t CIRC_NPOINTS = 30;
const size_t NUM_HELPER_DOTS = 5;
const rgb_color_t WHITE_DOT_COLOR = (rgb_color_t){1.0, 1.0, 1.0};
const double DOT_RADIUS = 2;
const double DOT_VELOCITY_SCALE_FACTOR = 0.2;
const double HALF_SCALE_FACTOR = 0.5;
const size_t HELPER_DOT_COLORS = 2;

//Start Screen
const char *START_SCREEN_PATH = "assets/startscreenbackground.png";
const char *GAME_TITLE_FONTPATH = "assets/Impacted.ttf";
const SDL_Rect GAME_TITLE_BOUNDING_BOX = (SDL_Rect){400, 200, 1000, 500};
const size_t BUTTON_LIST_LENGTH = 7;

//Skin Screen
const char *SKIN_SCREEN_PATH = "assets/skin_screen_background.png";
const size_t NUM_SKINS = 4;
const size_t NUM_SKIN_SCREEN_BUTTONS = 5;
const char *skin_button_selected_paths[] = {"assets/elven_button_selected.png", "assets/archer_button_selected.png", "assets/goblin_button_selected.png", "assets/knight_button_selected.png"};
const char *skin_paths[] = {"assets/elven.png", "assets/archer.png", "assets/goblin.png", "assets/knight.png"};
const char *skin_flipped_paths[] = {"assets/elven_flipped.png", "assets/archer_flipped.png", "assets/goblin_flipped.png", "assets/knight_flipped.png"};
const SDL_Rect SKIN_DISPLAY_BOX = {400, 250, 200, 200};
const size_t SKIN_BUTTON_INCREMENT = 1;

// Game Over
const char *GAME_OVER = "GAME OVER";
const char *PLAYER_ONE_WIN_PATH = "assets/player_one_wins.png";
const SDL_Rect OVER_ONE_BOUNDING_BOX = (SDL_Rect) {238, 0, 525, 300};
const char *PLAYER_TWO_WIN_PATH = "assets/player_two_wins.png";
const SDL_Rect OVER_TWO_BOUNDING_BOX = (SDL_Rect) {238, 0, 525, 300};
const char *GAME_OVER_FONT = "assets/Impacted.ttf";
const size_t GAME_OVER_LIST_LENGTH = 5;
const size_t GAME_BUTTON_LENGTH = 3;

// Health Bar
const size_t HEALTH_BAR_BORDER_IDX = 0;
const size_t HEALTH_BAR_HEALTH_IDX = 1;

// Character
const char *CHARACTER_PATH = "assets/character.png";
const vector_t INITIAL_CHARACTER_POS = {70, 50};
const vector_t CHARACTER_ONE_HEALTH_POSITION = {30, 380};
const char *CHARACTER_TWO_PATH = "assets/character2.png";
const vector_t INITIAL_CHARACTER_TWO_POS = {850, 50};
const vector_t CHARACTER_TWO_HEALTH_POSITION = {800, 380};
const double CHARACTER_MAX_HEALTH = 5000.0;
const double BOTTOM_BUFFER = 100;
const double BUFFER = 85;

typedef struct level {
    list_t *assets;
    character_t *character_one;
    character_t *character_two;
    body_t *left_wall;
    body_t *right_wall;
    body_t *ground;
    scene_t *scene;
    list_t *bullets;
    list_t *helper_dots;
    screen_t screen_name;
    bool turn;
    bool use_ai;
    size_t ai_difficulty;
    double ai_countdown;
    vector_t char_platform_velocity;
    list_t *game_over_assets;
    vector_t gravity;
} level_t;

typedef struct start_screen {
  asset_t *background;
  list_t *buttons;
} start_screen_t;

typedef struct skin_screen {
  skin_t skin;
  asset_t *background;
  list_t *buttons;
  asset_t *skin_display;
} skin_screen_t;

typedef struct button_info {
  const char *image_path;
  const char *font_path;
  SDL_Rect image_box;
  SDL_Rect text_box;
  rgb_color_t text_color;
  const char *text;
  button_handler_t handler;
} button_info_t;

const button_info_t start_screen_buttons[] = {
  {.image_path = "assets/skin_button.png",
     .font_path = "assets/Impacted.ttf",
     .image_box = (SDL_Rect){600, 300, 100, 100},
     .text_box = (SDL_Rect){885, 325, 50, 50},
     .text_color = (rgb_color_t){1.0, 1.0, 1.0},
     .text = "SKIN",
     .handler = NULL},
  {.image_path = "assets/level_one.png",
     .font_path = "assets/Impacted.ttf",
     .image_box = (SDL_Rect){50, 150, 100, 100},
     .text_box = (SDL_Rect){85, 325, 50, 50},
     .text_color = (rgb_color_t){1.0, 1.0, 1.0},
     .text = "",
     .handler = NULL},
     
     {.image_path = "assets/level_two.png",
     .font_path = "assets/Impacted.ttf",
     .image_box = (SDL_Rect){250, 150, 100, 100},
     .text_box = (SDL_Rect){285, 325, 50, 50},
     .text_color = (rgb_color_t){1.0, 1.0, 1.0},
     .text = "",
     .handler = NULL},

     {.image_path = "assets/level_three.png",
     .font_path = "assets/Impacted.ttf",
     .image_box = (SDL_Rect){450, 150, 100, 100},
     .text_box = (SDL_Rect){485, 325, 50, 50},
     .text_color = (rgb_color_t){1.0, 1.0, 1.0},
     .text = "",
     .handler = NULL},
     
     {.image_path = "assets/level_four.png",
     .font_path = "assets/Impacted.ttf",
     .image_box = (SDL_Rect){650, 150, 100, 100},
     .text_box = (SDL_Rect){685, 325, 50, 50},
     .text_color = (rgb_color_t){1.0, 1.0, 1.0},
     .text = "",
     .handler = NULL},

     {.image_path = "assets/level_five.png",
     .font_path = "assets/Impacted.ttf",
     .image_box = (SDL_Rect){850, 150, 100, 100},
     .text_box = (SDL_Rect){885, 325, 50, 50},
     .text_color = (rgb_color_t){1.0, 1.0, 1.0},
     .text = "",
     .handler = NULL},

     {.image_path = "assets/two_player.png",
     .font_path = "assets/Impacted.ttf",
     .image_box = (SDL_Rect){300, 300, 100, 100},
     .text_box = (SDL_Rect){885, 325, 50, 50},
     .text_color = (rgb_color_t){1.0, 1.0, 1.0},
     .text = "",
     .handler = NULL}}; 

const button_info_t game_over_buttons[] = {
    {.image_path = "assets/retry.png",
     .font_path = "assets/Impacted.ttf",
     .image_box = (SDL_Rect){250, 300, 100, 100},
     .text_box = (SDL_Rect){215, 325, 50, 50},
     .text_color = (rgb_color_t){1.0, 1.0, 1.0},
     .text = "",
     .handler = NULL},
     
     {.image_path = "assets/home.png",
     .font_path = "assets/Impacted.ttf",
     .image_box = (SDL_Rect){450, 300, 100, 100},
     .text_box = (SDL_Rect){412, 325, 50, 50},
     .text_color = (rgb_color_t){1.0, 1.0, 1.0},
     .text = "",
     .handler = NULL},

     {.image_path = "assets/next.png",
     .font_path = "assets/Impacted.ttf",
     .image_box = (SDL_Rect){650, 300, 100, 100},
     .text_box = (SDL_Rect){610, 325, 50, 50},
     .text_color = (rgb_color_t){1.0, 1.0, 1.0},
     .text = "",
     .handler = NULL}};


const button_info_t skin_screen_buttons[] = {
    {.image_path = "assets/back.png",
     .font_path = "assets/Impacted.ttf",
     .image_box = (SDL_Rect){25, 25, 100, 100},
     .text_box = (SDL_Rect){215, 325, 50, 50},
     .text_color = (rgb_color_t){1.0, 1.0, 1.0},
     .text = "",
     .handler = NULL},

     {.image_path = "assets/elven_button.png",
     .font_path = "assets/Impacted.ttf",
     .image_box = (SDL_Rect){250, 100, 100, 100},
     .text_box = (SDL_Rect){215, 325, 50, 50},
     .text_color = (rgb_color_t){1.0, 1.0, 1.0},
     .text = "",
     .handler = NULL},

     {.image_path = "assets/archer_button.png",
     .font_path = "assets/Impacted.ttf",
     .image_box = (SDL_Rect){383, 100, 100, 100},
     .text_box = (SDL_Rect){215, 325, 50, 50},
     .text_color = (rgb_color_t){1.0, 1.0, 1.0},
     .text = "",
     .handler = NULL},

     {.image_path = "assets/goblin_button.png",
     .font_path = "assets/Impacted.ttf",
     .image_box = (SDL_Rect){513, 100, 100, 100},
     .text_box = (SDL_Rect){215, 325, 50, 50},
     .text_color = (rgb_color_t){1.0, 1.0, 1.0},
     .text = "",
     .handler = NULL},

     {.image_path = "assets/knight_button.png",
     .font_path = "assets/Impacted.ttf",
     .image_box = (SDL_Rect){650, 100, 100, 100},
     .text_box = (SDL_Rect){215, 325, 50, 50},
     .text_color = (rgb_color_t){1.0, 1.0, 1.0},
     .text = "",
     .handler = NULL},
     };

start_screen_t *start_screen_init() {
  start_screen_t *new = malloc(sizeof(start_screen_t));
  assert(new);

  // background
  SDL_Rect bounding_box = sdl_get_bounds(SCREEN_MAX.y, SCREEN_MAX.x, VEC_ZERO.x, VEC_ZERO.y);
  new->background = asset_make_image(START_SCREEN_PATH, bounding_box);

  // buttons
  TTF_Init();
  new->buttons = list_init(BUTTON_LIST_LENGTH, (free_func_t)asset_destroy);
  for (size_t i = 0; i < BUTTON_LIST_LENGTH; i++) {
    asset_t *font_asset = asset_make_text(start_screen_buttons[i].font_path, start_screen_buttons[i].image_box, start_screen_buttons[i].text, start_screen_buttons[i].text_color);
    asset_t *image_asset = asset_make_image(start_screen_buttons[i].image_path, start_screen_buttons[i].image_box);
    asset_t *button = asset_make_button(start_screen_buttons[i].image_box, image_asset, font_asset, start_screen_buttons[i].handler);
    list_add(new->buttons, button);
  }
  return new;
}

skin_screen_t *skin_screen_init() {
  
  skin_screen_t *new = malloc(sizeof(skin_screen_t));
  assert(new);

  // background
  SDL_Rect bounding_box = sdl_get_bounds(SCREEN_MAX.y, SCREEN_MAX.x, VEC_ZERO.x, VEC_ZERO.y);
  new->background = asset_make_image(SKIN_SCREEN_PATH, bounding_box);

  // buttons
  TTF_Init();
  new->buttons = list_init(NUM_SKIN_SCREEN_BUTTONS, (free_func_t)asset_destroy);
  for (size_t i = 0; i < NUM_SKIN_SCREEN_BUTTONS; i++) {
    asset_t *font_asset = asset_make_text(skin_screen_buttons[i].font_path, skin_screen_buttons[i].image_box, "", skin_screen_buttons[i].text_color);
    asset_t *image_asset = asset_make_image(skin_screen_buttons[i].image_path, skin_screen_buttons[i].image_box);
    asset_t *button = asset_make_button(skin_screen_buttons[i].image_box, image_asset, font_asset, skin_screen_buttons[i].handler);
    list_add(new->buttons, button);
  }
  
  // display skin
  new->skin = ELVEN;
  new->skin_display = asset_make_image(skin_paths[0], SKIN_DISPLAY_BOX);
  return new;
}

void skin_screen_main(skin_screen_t *screen) {
  asset_render(screen->background);
  for (size_t i = 0; i < NUM_SKIN_SCREEN_BUTTONS; i++) {
    asset_render(list_get(screen->buttons, i));
  }
  asset_render(screen->skin_display);
}

void skin_screen_free(skin_screen_t *screen) {
  list_free(screen->buttons);
  asset_destroy(screen->background);
  free(screen);
}

void start_screen_main(start_screen_t *screen) {
  asset_render(screen->background);
  for (size_t i = 0; i < BUTTON_LIST_LENGTH; i++) {
    asset_render(list_get(screen->buttons, i));
  }
}

void start_screen_free(start_screen_t *screen) {
  list_free(screen->buttons);
  asset_destroy(screen->background);
  free(screen);
}

level_t *level_init(level_info_t level_info) {
  level_t *new = malloc(sizeof(level_t));
  assert(new);

  new->scene = scene_init();
  new->assets = list_init(ASSET_MEMORY, (free_func_t)asset_destroy);
  new->bullets = list_init(BULLET_MEMORY, (free_func_t)body_free);
  new->helper_dots = list_init(NUM_HELPER_DOTS * HELPER_DOT_COLORS, (free_func_t)body_free);
  new->screen_name = level_info.screen_name;
  new->use_ai = level_info.use_ai;
  new->char_platform_velocity = level_info.character_2_velocity;
  new->ai_countdown = INFINITY;
  new->ai_difficulty = level_info.ai_difficulty;
  new->turn = true;
  new->gravity = level_info.level_gravity;

  // background
  SDL_Rect bounding_box1 = sdl_get_bounds(SCREEN_MAX.y, SCREEN_MAX.x, VEC_ZERO.x, VEC_ZERO.y);
  asset_t *background_asset = asset_make_image(level_info.background_image_path, bounding_box1);
  list_add(new->assets, background_asset);

  // walls
  new->left_wall = body_init(sdl_make_rectangle(LEFT_WALL_X, SCREEN_MAX.y * WALL_HEIGHT_FACTOR, WALL_WIDTH_LEFT, SCREEN_MAX.y * WALL_HEIGHT_FACTOR), INFINITY, BLACK);
  list_add(new->assets, asset_make_body(new->left_wall));
  new->right_wall = body_init(sdl_make_rectangle(SCREEN_MAX.x - RIGHT_WALL_X_OFFSET, SCREEN_MAX.y * WALL_HEIGHT_FACTOR, WALL_WIDTH_RIGHT, SCREEN_MAX.y * WALL_HEIGHT_FACTOR), INFINITY, BLACK);
  list_add(new->assets, asset_make_body(new->right_wall));
  scene_add_body(new->scene, new->right_wall);
  new->ground = body_init(sdl_make_rectangle(0, GROUND_Y, SCREEN_MAX.x, GROUND_HEIGHT), INFINITY, BLACK);
  list_add(new->assets, asset_make_body(new->ground));

  // first character
  character_t *character = character_init(level_info.inital_character_one_pos, level_info.character_one_max_health, level_info.character_one_image_path, new->scene, CHARACTER_ONE_HEALTH_POSITION);
  new->character_one = character;
  list_add(new->assets, character_get_body_asset(character)); 
  list_add(new->assets, character_get_platform_asset(character));
  list_t *character_health_bar_assets = character_get_health_bar_assets(character);
  list_add(new->assets, list_get(character_health_bar_assets, HEALTH_BAR_BORDER_IDX));
  list_add(new->assets, list_get(character_health_bar_assets, HEALTH_BAR_HEALTH_IDX));

  // second character
  character_t *character_two = character_init(level_info.inital_character_two_pos, level_info.character_two_max_health, level_info.character_two_image_path, new->scene, CHARACTER_TWO_HEALTH_POSITION);
  new->character_two = character_two;
  character_set_velocity(new->character_two, new->char_platform_velocity);
  character_set_platform_velocity(new->character_two, new->char_platform_velocity);
  list_add(new->assets, character_get_body_asset(character_two));
  list_add(new->assets, character_get_platform_asset(character_two));
  list_t *character_two_health_bar_assets = character_get_health_bar_assets(character_two);
  list_add(new->assets, list_get(character_two_health_bar_assets, HEALTH_BAR_BORDER_IDX));
  list_add(new->assets, list_get(character_two_health_bar_assets, HEALTH_BAR_HEALTH_IDX));

  // game over assets
  new->game_over_assets = list_init(GAME_OVER_LIST_LENGTH, (free_func_t)asset_destroy);
  list_add(new->game_over_assets, asset_make_image(PLAYER_ONE_WIN_PATH, OVER_ONE_BOUNDING_BOX));
  list_add(new->game_over_assets, asset_make_image(PLAYER_TWO_WIN_PATH, OVER_TWO_BOUNDING_BOX));
  for (size_t i = 0; i < GAME_BUTTON_LENGTH; i++) {
    asset_t *image_asset = asset_make_image(game_over_buttons[i].image_path, game_over_buttons[i].image_box);
    asset_t *text_asset = asset_make_text(game_over_buttons[i].font_path, game_over_buttons[i].image_box, game_over_buttons[i].text, game_over_buttons[i].text_color);
    list_add(new->game_over_assets, asset_make_button(game_over_buttons[i].image_box, image_asset, text_asset, game_over_buttons[i].handler));
  }
  return new;
}

/** Make a circle-shaped body object.
 *
 * @param center a vector representing the center of the body.
 * @param radius the radius of the circle
 * @param mass the mass of the body
 * @param color the color of the circle
 * @return pointer to the circle-shaped body
 */
body_t *make_circle(vector_t center, double radius, double mass,
                    rgb_color_t color) {
  list_t *c = list_init(CIRC_NPOINTS, free);
  for (size_t i = 0; i < CIRC_NPOINTS; i++) {
    double angle = 2 * M_PI * i / CIRC_NPOINTS;

    vector_t *v = malloc(sizeof(*v));
    assert(v);

    vector_t unit = {cos(angle), sin(angle)};
    *v = vec_add(vec_multiply(radius, unit), center);

    list_add(c, v);
  }
  return body_init(c, mass, color);
}

/**
 * Returns the character whose turn it is. Option to return the character
 * that is not their turn.
 *
 * @param state the state of the game
 * @param opposite whether or not to return the character without the turn
 * @return pointer to character (or other character) of current turn
 */
character_t *get_character_turn(level_t *level, bool opposite) {
  character_t *character;
  if (level->turn) {
    if (opposite) {
      return level->character_two;
    }
    return level->character_one;
  } else {
    if (opposite) {
      return level->character_one;
    }
    return level->character_two;
  }
  return character; 
}

void level_set_shot_start(level_t *level, double x, double y) {
  character_t *character = get_character_turn(level, false);
  character_set_shot_start_point(character, (vector_t){x, y});
  character_set_shot_end_point(character, (vector_t){x, y});

  //add helper dots
  vector_t dots_start_point; 
  vector_t character_center = body_get_centroid(character_get_body(character));
  double character_half_width = character_get_size(level->character_one).x / 2;
  if (level->turn) {
    dots_start_point = (vector_t){character_center.x + character_half_width, character_center.y};
  }
  else {
    dots_start_point = (vector_t){character_center.x - character_half_width, character_center.y};
  }

  // Creating black dot border
  for (size_t i = 0; i < NUM_HELPER_DOTS; i++) {
    body_t *dot = make_circle(dots_start_point, DOT_RADIUS * 2, INFINITY, BLACK);
    asset_t *dot_asset = asset_make_body(dot);
    list_add(level->helper_dots, dot);
    list_add(level->assets, dot_asset);
    scene_add_body(level->scene, dot);
  }

  // Creating white dots inside
  for (size_t i = 0; i < NUM_HELPER_DOTS; i++) {
    body_t *dot = make_circle(dots_start_point, DOT_RADIUS, INFINITY, WHITE_DOT_COLOR);
    asset_t *dot_asset = asset_make_body(dot);
    list_add(level->helper_dots, dot);
    list_add(level->assets, dot_asset);
    scene_add_body(level->scene, dot);
  }
  sdl_play_sound_effect(DRAW_BOW);
}

void level_update_helper_dots(level_t *level) {
  character_t *character = get_character_turn(level, false);
  vector_t shot_start_point = character_get_shot_start_point(character);
  vector_t shot_end_point = character_get_shot_end_point(character);
  vector_t velocity_vec = character_shot_velocity(shot_start_point, shot_end_point, SHOT_MAX_SPEED);
  if (!vec_equals(shot_start_point, VEC_ZERO)) {

    // get first dot position
    body_t *start_dot = list_get(level->helper_dots, 0);
    vector_t dots_start_point = VEC_ZERO; 
    vector_t character_center = body_get_centroid(character_get_body(character));
    double character_half_width = character_get_size(level->character_one).x / 2;
    if (level->turn) {
      dots_start_point.x = character_center.x + character_half_width;
      dots_start_point.y = character_center.y;
    }
    else {
      dots_start_point.x = character_center.x - character_half_width;
      dots_start_point.y = character_center.y;
    }
    body_set_centroid(start_dot, dots_start_point);

    // spread out dots
    vector_t increment = vec_multiply(1.0 / NUM_HELPER_DOTS, vec_multiply(DOT_VELOCITY_SCALE_FACTOR, velocity_vec));
    for (size_t i = 0; i < NUM_HELPER_DOTS; i++) { 
      body_t *dot = list_get(level->helper_dots, i);
      body_set_centroid(dot, vec_add(vec_multiply(i, increment), dots_start_point));
    }
    for (size_t i =  NUM_HELPER_DOTS; i < 2 * NUM_HELPER_DOTS; i++) { 
      body_t *dot = list_get(level->helper_dots, i);
      body_set_centroid(dot, vec_add(vec_multiply(i - (NUM_HELPER_DOTS), increment), dots_start_point));
    }
  }
}

void level_shot_drag_update(level_t *level, double x, double y) {
  character_t *character = get_character_turn(level, false);
  vector_t shot_end_point = (vector_t){x, y};
  character_set_shot_end_point(character, shot_end_point);
}

void level_cycle_turns(level_t *level) {
  level->turn = !level->turn;
}

/**
 * The collision handler for collisions between bullets and objects.
 * If hitting a character, lowers health proportional to the incoming velocity.
 *
 * @param body1 the body for the bullet
 * @param body2 the body of the other object
 * @param axis the axis of collision
 * @param aux the game state
 * @param elasticity the elasticity of the collision
 */
void bullet_collision_handler(body_t *body1, body_t *body2, vector_t axis,
                                void *aux, double force_const) {
  level_t *level = aux;
  vector_t incoming_velocity = body_get_velocity(body1);
  double damage = vec_get_length(incoming_velocity);
  if (body2 == character_get_body(level->character_one)) {
    sdl_play_sound_effect(HIT);
    character_deduct_health(level->character_one, damage);
  }
  else if (body2 == character_get_body(level->character_two)) {
    sdl_play_sound_effect(HIT);
    character_deduct_health(level->character_two, damage);
  }
  body_remove(body1);
  if (level->use_ai && !level->turn && !level_game_over(level)) {
    level_start_ai_countdown(level, 1.5);
  }
  for (size_t i = 0; i < list_size(level->bullets); i++) {
    if (list_get(level->bullets, i) == body1) {
      list_remove(level->bullets, i);
    }
  }
  for (size_t i = 0; i < list_size(level->assets); i++) {
    if (asset_get_body(list_get(level->assets, i)) == body1) {
      list_remove(level->assets, i);
    }
  }
}

bool level_bullet_in_scene(level_t *level) {
  return list_size(level->bullets) > 0;
}

/**
 * Makes the bullet the bullet body for the character.
 *
 * @param level game level used to calculate center for the bullet
 * @param character character that is shooting the bullet
 * @param mass mass of the bullet
 * @param color color of the bullet
 * @return body of the bullet
 */

body_t *make_bullet(level_t *level, character_t *character, double mass,
                    rgb_color_t color) {
  vector_t bullet_center; 
  vector_t character_center = body_get_centroid(character_get_body(character));
  double character_half_width = character_get_size(level->character_one).x * HALF_SCALE_FACTOR;
  if (character == level->character_one) {
    bullet_center = (vector_t){character_center.x + character_half_width, character_center.y};
  }
  else {
    bullet_center = (vector_t){character_center.x - character_half_width, character_center.y};
  }
  list_t *bullet_shape = sdl_make_rectangle(bullet_center.x, bullet_center.y, BULLET_WIDTH, BULLET_HEIGHT);
  body_t *bullet = body_init(bullet_shape, mass, color);
  body_set_rotate_with_velocity(bullet, true);
  return bullet;
}

/**
 * Creates collisions between the bullet and the player that is being target,
 * the left and right walls, and the ground.
 * 
 * @param level pointer to the current level
 * @param bullet pointer to the body of the bullet
 * @param character_to_hit pointer to the body of the character that should be affected by this bullet
*/
void create_bullet_collision(level_t *level, body_t *bullet, body_t *character_to_hit) {
  create_collision(level->scene, bullet, character_to_hit, bullet_collision_handler, level, BULLET_ELASTICITY);
  create_collision(level->scene, bullet, level->left_wall, bullet_collision_handler, level, BULLET_ELASTICITY);
  create_collision(level->scene, bullet, level->right_wall, bullet_collision_handler, level, BULLET_ELASTICITY);
  create_collision(level->scene, bullet, level->ground, bullet_collision_handler, level, BULLET_ELASTICITY);
}

void level_ai_shoot(level_t *level) {
  body_t *player = character_get_body(level->character_one);
  vector_t player_center = body_get_centroid(player);
  body_t* ai = character_get_body(level->character_two);

  // get shot origin
  vector_t ai_center = body_get_centroid(ai);
  double ai_half_width = character_get_size(level->character_two).x * HALF_SCALE_FACTOR;
  vector_t shot_origin = (vector_t){ai_center.x - ai_half_width, ai_center.y};
  
  // make bullet
  vector_t init_velocity = character_ai_shot_velocity(shot_origin, player_center, level->ai_difficulty, level->gravity);
  body_t *bullet = make_bullet(level, level->character_two, BULLET_MASS, BLACK);
  body_set_velocity(bullet, init_velocity);
  create_bullet_collision(level, bullet, character_get_body(level->character_one));
  asset_t *bullet_asset = asset_make_image_with_body(BULLET_PATH, bullet);
  scene_add_body(level->scene, bullet);
  list_add(level->assets, bullet_asset);
  list_add(level->bullets, bullet);

  sdl_play_sound_effect(SHOOT);
  level_cycle_turns(level);
}

void level_start_ai_countdown(level_t *level, double countdown_time) {
  level->ai_countdown = countdown_time;
}

double level_update_ai_countdown(level_t *level, double dt) {
  if (level->ai_countdown != INFINITY) {
    level->ai_countdown -= dt;
  }
  return level->ai_countdown;
}

void level_shoot_shot(level_t *level, double x, double y) {
  character_t *character = get_character_turn(level, false);
  character_t *character_opposite = get_character_turn(level, true);
  vector_t shot_end_point = (vector_t){x, y};
  vector_t shot_start_point = character_get_shot_start_point(character);
  if (!vec_equals(shot_start_point, VEC_ZERO)) {

    // reset helper dots
    for (size_t i = 0; i < HELPER_DOT_COLORS * NUM_HELPER_DOTS; i++) {
      body_t* body_to_remove = list_remove(level->helper_dots, 0);
      body_remove(body_to_remove);
      list_remove(level->assets, list_size(level->assets) - 1);
    }

    // make bullet
    character_set_shot_end_point(character, shot_end_point);
    vector_t init_velocity = character_shot_velocity(shot_start_point, shot_end_point, SHOT_MAX_SPEED);
    body_t *bullet = make_bullet(level, character, BULLET_MASS, BLACK);
    asset_t *bullet_asset = asset_make_image_with_body(BULLET_PATH, bullet);
    body_set_velocity(bullet, init_velocity);
    create_bullet_collision(level, bullet, character_get_body(character_opposite));
    scene_add_body(level->scene, bullet);
    list_add(level->assets, bullet_asset);
    list_add(level->bullets, bullet);

    // reset shot parameters
    character_set_shot_start_point(character, VEC_ZERO);
    character_set_shot_end_point(character, VEC_ZERO);

    level_cycle_turns(level);
    sdl_play_sound_effect(SHOOT);
  }
}

screen_t level_get_screen_name(level_t *level) {
  return level->screen_name;
}

bool level_get_use_ai(level_t *level) {
  return level->use_ai;
}

bool level_get_turn(level_t *level) {
  return level->turn;
}

ssize_t level_get_start_button_index_clicked(double x, double y) {
  ssize_t index = -1;
  for (size_t i = 0; i < BUTTON_LIST_LENGTH; i++) {
    if (sdl_contained_in_box(x, y, start_screen_buttons[i].image_box)) {
      index = i;
    }
  }
  return index;
}

ssize_t level_get_skin_button_index_clicked(double x, double y) {
  ssize_t index = -1;
  for (size_t i = 0; i < NUM_SKIN_SCREEN_BUTTONS; i++) {
    if (sdl_contained_in_box(x, y, skin_screen_buttons[i].image_box)) {
      index = i;
    }
  }
  return index;
}

void level_main(level_t *level) {
  double dt = time_since_last_tick();

  // rendering assets
  sdl_render_scene(level->scene, NULL);
  for (size_t i = 0; i < list_size(level->assets); i++) {
    asset_render(list_get(level->assets, i));
  }

  // moving platform
  if (character_position_limit(level->character_two, SCREEN_MIN.y + BOTTOM_BUFFER, SCREEN_MAX.y - BUFFER)) {
    if (body_get_centroid(character_get_body(level->character_two)).y <= SCREEN_MIN.y + BOTTOM_BUFFER && level->char_platform_velocity.y < 0) {
      level->char_platform_velocity = vec_negate(level->char_platform_velocity);
    }
    else if (body_get_centroid(character_get_body(level->character_two)).y >= SCREEN_MAX.y - BUFFER && level->char_platform_velocity.y > 0) {
      level->char_platform_velocity = vec_negate(level->char_platform_velocity);
    }
    character_set_velocity(level->character_two, level->char_platform_velocity);
    character_set_platform_velocity(level->character_two, level->char_platform_velocity);
  }

  // update shots
  character_update_health_bar(level->character_one);
  character_update_health_bar(level->character_two);
  for (size_t i = 0; i < list_size(level->bullets); i++) {
    body_t *bullet = list_get(level->bullets, i);
    body_add_force(bullet, vec_multiply(body_get_mass(bullet), level->gravity));
  }
  level_update_helper_dots(level);
  if (level_update_ai_countdown(level, dt) <= 0) {
    level_ai_shoot(level);
    level->ai_countdown = INFINITY;
  }
  scene_tick(level->scene, dt);
}

bool level_game_over(level_t *level) {
  return character_get_health(level->character_one) <= 0 || character_get_health(level->character_two) <= 0;
}

ssize_t level_game_over_get_button_index(double x, double y) {
  ssize_t index = -1;
  for (size_t i = 0; i < GAME_BUTTON_LENGTH; i++) {
    if (sdl_contained_in_box(x, y, game_over_buttons[i].image_box)) {
      index = i;
    }
  }
  return index;
}

void level_set_skin_screen(skin_screen_t *screen, skin_t skin) {
  asset_set_image(list_get(screen->buttons, screen->skin + SKIN_BUTTON_INCREMENT), skin_screen_buttons[screen->skin + SKIN_BUTTON_INCREMENT].image_path);
  asset_set_image(list_get(screen->buttons, skin + SKIN_BUTTON_INCREMENT), skin_button_selected_paths[skin]);
  asset_set_image(screen->skin_display, skin_paths[skin]);
  screen->skin = skin;
}

void level_set_skin(level_t* level, skin_t skin, bool set_flipped) {
  asset_set_image(character_get_body_asset(level->character_one), skin_paths[skin]);
  if (set_flipped) {
    asset_set_image(character_get_body_asset(level->character_two), skin_flipped_paths[skin]);
  }
}

list_t *level_get_game_over_assets(level_t *level) {
  return (level->game_over_assets);
}

double level_get_character1_health(level_t *level) {
  return character_get_health(level->character_one);
}

double level_get_character2_health(level_t *level) {
  return character_get_health(level->character_two);
}

void level_free(level_t *level) {
  character_free(level->character_one);
  character_free(level->character_two);
  list_free(level->bullets);
  list_free(level->assets);
  scene_free(level->scene);
  free(level);
}

