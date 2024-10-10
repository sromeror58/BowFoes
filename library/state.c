#include <assert.h>

#include "character.h"
#include "scene.h"
#include "asset.h"
#include "asset_cache.h"
#include "body.h"
#include "vector.h"
#include "sdl_wrapper.h"
#include "level.h"
#include "state.h"

const size_t REPLAY_BTN_IDX = 0;
const size_t HOME_BTN_IDX = 1;
const size_t NEXT_BTN_IDX = 2;
const size_t GAME_OVER_ASSETS_START = 2;
const size_t GAME_OVER_ASSETS_END = 5;
const size_t PLAYER_TWO_WIN = 1;
const size_t PLAYER_ONE_WIN = 0;
const size_t LEVEL_VOLUME = 40;
const ssize_t BACK_BUTTON_INCREMENT = -1;
const ssize_t TWO_PLAYER_LEVEL_INCREMENT = -1;

typedef struct state {
    start_screen_t *start_screen;
    skin_screen_t *skin_screen;
    level_t **levels;
    screen_t curr_screen;
    skin_t skin;
    size_t num_levels;
    level_info_t levels_info[];
} state_t;

state_t *state_init(const level_info_t levels_info[], size_t num_levels) {
  state_t *state = malloc(sizeof(state_t) + num_levels * sizeof(level_info_t));
  assert(state);

  // levels
  state->levels = malloc(num_levels * sizeof(level_t *));
  assert(state->levels);
  for (size_t i = 0; i < num_levels; i++) {
    state->levels_info[i] = levels_info[i];
  }
  for (size_t i = 0; i < num_levels; i++) {
    state->levels[i] = level_init(levels_info[i]);
  }
  state->start_screen = start_screen_init();
  state->skin_screen = skin_screen_init();
  state->curr_screen = START_SCENE;
  state->num_levels = num_levels;

  // skin
  state->skin = ELVEN;
  level_set_skin_screen(state->skin_screen, ELVEN);
  return state;
}

screen_t state_get_screen(state_t *state) {
    return state->curr_screen;
}

level_t *state_current_level(state_t *state) {
    if (state->curr_screen > SKIN_SCREEN) {
        return state->levels[state->curr_screen - LEVEL_ONE];
    }
    return NULL;
}

void state_game_over_handler(state_t *state, double x, double y) {
  ssize_t index = level_game_over_get_button_index(x, y);
  if (index == -1) {
    return;
  }
  level_t *removed = state->levels[state->curr_screen - LEVEL_ONE];
  state->levels[state->curr_screen - LEVEL_ONE] = level_init(state->levels_info[state->curr_screen - LEVEL_ONE]);
  state_set_skin(state);
  free(removed);

  // replay
  if (index == REPLAY_BTN_IDX) {
    return;
  }

  // quit
  else if (index == HOME_BTN_IDX) {
    state->curr_screen = START_SCENE;
    sdl_set_music_volume(MIX_MAX_VOLUME);
  }  

  //next
  else if (index == NEXT_BTN_IDX) {
    if (state->curr_screen >= LEVEL_FIVE) {
      state->curr_screen = START_SCENE;
      sdl_set_music_volume(MIX_MAX_VOLUME);
    }
    else {
      state->curr_screen = state_get_screen(state) + 1; // go to next level
    }
  }
}

void state_current_main(state_t *state) {
  if (state->curr_screen > SKIN_SCREEN) {
      level_t *curr = state_current_level(state);
      level_main(curr);
      if (level_game_over(curr)) {
        for (size_t i = GAME_OVER_ASSETS_START; i < GAME_OVER_ASSETS_END; i++) {
          asset_render(list_get(level_get_game_over_assets(curr), i));
        }
        if (level_get_character1_health(curr) <= 0) {
            asset_render(list_get(level_get_game_over_assets(curr), PLAYER_TWO_WIN));
        }
        else if (level_get_character2_health(curr) <= 0) {
            asset_render(list_get(level_get_game_over_assets(curr), PLAYER_ONE_WIN));
        }
      }
  }
  else if (state->curr_screen == SKIN_SCREEN) {
    skin_screen_main(state->skin_screen);
  }
  else {
    start_screen_main(state->start_screen);
  }
}

void state_start_screen_handler(state_t *state, double x, double y) {
  state->curr_screen = (size_t)(level_get_start_button_index_clicked(x, y) + SKIN_SCREEN);
  if (state->curr_screen > SKIN_SCREEN) {
    sdl_set_music_volume(LEVEL_VOLUME);
  }
}

void state_skin_screen_handler(state_t *state, double x, double y) {
  size_t button_clicked = (size_t)(level_get_skin_button_index_clicked(x, y));
  switch (button_clicked + BACK_BUTTON_INCREMENT) {
    case ELVEN: {
      state->skin = ELVEN;
      level_set_skin_screen(state->skin_screen, ELVEN);
      state_set_skin(state);
      break;
    }
    case ARCHER: {
      state->skin = ARCHER;
      level_set_skin_screen(state->skin_screen, ARCHER);
      state_set_skin(state);
      break;
    }
    case GOBLIN: {
      state->skin = GOBLIN;
      level_set_skin_screen(state->skin_screen, GOBLIN);
      state_set_skin(state);
      break;
    }
    case KNIGHT: {
      state->skin = KNIGHT;
      level_set_skin_screen(state->skin_screen, KNIGHT);
      state_set_skin(state);
      break;
    }
    case BACK_BUTTON_INCREMENT: {
      state->curr_screen = START_SCENE;
    }
  }
}

void state_set_skin(state_t *state) {
  for (size_t i = 0; i < state->num_levels + TWO_PLAYER_LEVEL_INCREMENT; i++) {
    level_set_skin(state->levels[i], state->skin, false);
  }
  level_set_skin(state->levels[state->num_levels + TWO_PLAYER_LEVEL_INCREMENT], state->skin, true);
}

void state_free(state_t *state, size_t num_levels) {
  start_screen_free(state->start_screen);
  skin_screen_free(state->skin_screen);
  for (size_t i = 0; i < num_levels; i++) {
    level_free(state->levels[i]);
  }
  free(state->levels);
  asset_cache_destroy();
  free(state);
}