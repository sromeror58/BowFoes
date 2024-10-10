#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "asset_cache.h"
#include "collision.h"
#include "forces.h"
#include "list.h"
#include "sdl_wrapper.h"
#include "character.h"
#include "level.h"
#include "state.h"

const vector_t WINDOW_MIN = {0, 0};
const vector_t WINDOW_MAX = {1000, 500};
const size_t NUM_LEVELS = 6;
const vector_t CHARACTER1_POS = {70, 50};
const vector_t CHARACTER2_POS = {850, 50};
const vector_t LEVEL_TWO_CHARACTER2_POS = {850, 250};

const level_info_t levels_info[] = {
  { .screen_name = LEVEL_ONE,
    .background_image_path = "assets/level_one_background.png",
    .inital_character_one_pos = CHARACTER1_POS,
    .character_one_max_health = 5000.0,
    .character_one_image_path = "assets/character.png",
    .inital_character_two_pos = CHARACTER2_POS,
    .character_two_max_health = 1500.0,
    .character_two_image_path = "assets/enemy_flipped.png",
    .use_ai = true,
    .ai_difficulty = 10,
    .character_2_velocity = (vector_t){0, 0},
    .level_gravity = (vector_t){0, -250}},

  { .screen_name = LEVEL_TWO,
    .background_image_path = "assets/level_two_background.png",
    .inital_character_one_pos = CHARACTER1_POS,
    .character_one_max_health = 5000.0,
    .character_one_image_path = "assets/character.png",
    .inital_character_two_pos = LEVEL_TWO_CHARACTER2_POS,
    .character_two_max_health = 2500.0,
    .character_two_image_path = "assets/enemy_flipped.png",
    .use_ai = true,
    .ai_difficulty = 30,
    .character_2_velocity = (vector_t){0, 0},
    .level_gravity = (vector_t){0, -250}},

  { .screen_name = LEVEL_THREE,
    .background_image_path = "assets/level_three_background.png",
    .inital_character_one_pos = CHARACTER1_POS,
    .character_one_max_health = 5000.0,
    .character_one_image_path = "assets/character.png",
    .inital_character_two_pos = CHARACTER2_POS,
    .character_two_max_health = 3500.0,
    .character_two_image_path = "assets/enemy_flipped.png",
    .use_ai = true,
    .ai_difficulty = 60,
    .character_2_velocity = (vector_t){0, 50},
    .level_gravity = (vector_t){0, -250}},

  { .screen_name = LEVEL_FOUR,
    .background_image_path = "assets/level_four_background.png",
    .inital_character_one_pos = CHARACTER1_POS,
    .character_one_max_health = 4500.0,
    .character_one_image_path = "assets/character.png",
    .inital_character_two_pos = CHARACTER2_POS,
    .character_two_max_health = 4500.0,
    .character_two_image_path = "assets/enemy_flipped.png",
    .use_ai = true,
    .ai_difficulty = 75,
    .character_2_velocity = (vector_t){0, 100},
    .level_gravity = (vector_t){0, -100}},

  { .screen_name = LEVEL_FIVE,
    .background_image_path = "assets/level_five_background.png",
    .inital_character_one_pos = CHARACTER1_POS,
    .character_one_max_health = 6200.0,
    .character_one_image_path = "assets/character.png",
    .inital_character_two_pos = CHARACTER2_POS,
    .character_two_max_health = 6500.0,
    .character_two_image_path = "assets/enemy_flipped.png",
    .use_ai = true,
    .ai_difficulty = 90,
    .character_2_velocity = (vector_t){0, 150},
    .level_gravity = (vector_t){0, -400}},

    { .screen_name = TWO_PLAYER,
    .background_image_path = "assets/level_one_background.png",
    .inital_character_one_pos = CHARACTER1_POS,
    .character_one_max_health = 3000.0,
    .character_one_image_path = "assets/character.png",
    .inital_character_two_pos = CHARACTER2_POS,
    .character_two_max_health = 3000.0,
    .character_two_image_path = "assets/character2.png",
    .use_ai = false,
    .ai_difficulty = 95,
    .character_2_velocity = (vector_t){0, 50},
    .level_gravity = (vector_t){0, -250}}
};

state_t *emscripten_init() {
  asset_cache_init();
  sdl_init(WINDOW_MIN, WINDOW_MAX);
  state_t *state = state_init(levels_info, NUM_LEVELS);
  sdl_set_mouse_handlers(state_start_screen_handler, state_skin_screen_handler, level_set_shot_start, level_shot_drag_update, level_shoot_shot, state_game_over_handler);
  return state;
}

bool emscripten_main(state_t *state) {
  sdl_clear();
  state_current_main(state);
  sdl_show();
  return false;
}

void emscripten_free(state_t *state) {
  state_free(state, NUM_LEVELS);
}