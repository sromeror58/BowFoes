#ifndef __STATE_H__
#define __STATE_H__

#include "math.h"
#include "level.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * Stores the demo state
 * Use this to store any variable needed every 'tick' of your demo
 */
typedef struct state state_t;

/**
 * Mallocs a state with the corresponding level infos and returns a pointer
 * to the state.
 * 
 * @return pointer to the state
 */
state_t *state_init(const level_info_t levels_info[], size_t num_levels);

/**
 * Returns the current screen that is being displayed
 * 
 * @param state pointer to a state
 * @return the current screen
*/
screen_t state_get_screen(state_t *state);

/** 
 * Returns the current level.
 * 
 * @param state pointer to state
 * @return pointer to current level
*/
level_t *state_current_level(state_t *state);

/**
 * Handles clicks after a level has finished. Allows going back to the start
 * screen, trying the level again, or going to the next level.
 * Also reinitializes the current level.
 * 
 * @param state pointer to the state
 * @param x x coordinate of the mouse click
 * @param y y coordinate of the mouse click
*/
void state_game_over_handler(state_t *state, double x, double y);

/**
 * Calls the main method corresponding to the current scene.
 * 
 * @param state pointer to the state
*/
void state_current_main(state_t *state);

/**
 * Mouse handler for clicking buttons on the start screen.
 * Clicking these buttons should go to the corresponding level.
 * Should follow type mouse_handler_t.
 * 
 * @param state pointer to the state
 * @param x x coordinate of click
 * @param y y coordinate of click
*/
void state_start_screen_handler(state_t *state, double x, double y);

/**
 * Mouse handler for clicking on the skin screen.
 * Should follow type mouse_handler_t.
 * 
 * @param state pointer to the state
 * @param x x coordinate of click
 * @param y y coordinate of click
*/
void state_skin_screen_handler(state_t *state, double x, double y);

/**
 * Frees the state and all the levels inside. Also destroys the asset_cache
 * 
 * @param state a pointer to the state
*/
void state_free(state_t *state, size_t num_levels);

/**
 * Loops through all levels and sets the skin to the current selected skin.
 *
 * @param state pointer to the state
*/
void state_set_skin(state_t *state);

/**
 * Initializes sdl as well as the variables needed
 * Creates and stores all necessary variables for the demo in a created state
 * variable Returns the pointer to this state (This is the state emscripten_main
 * and emscripten_free work with)
 */
state_t *emscripten_init();

/**
 * Called on each tick of the program
 * Updates the state variables and display as necessary, depending on the time
 * that has passed.
 *
 * @param state pointer to a state object with info about demo
 * @return a boolean representing whether the game/demo is over
 */
bool emscripten_main(state_t *state);

/**
 * Frees anything allocated in the demo
 * Should free everything in state as well as state itself.
 *
 * @param state pointer to a state object with info about demo
 */
void emscripten_free(state_t *state);

#endif // #ifndef __STATE_H__