#ifndef __LEVEL_H__
#define __LEVEL_H__

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "scene.h"
#include "body.h"
#include "vector.h"


/**
 * The start screen struct that will allow us to contain all the buttons that 
 * can lead to all the levels in the game.
 */
typedef struct start_screen start_screen_t;

/**
 * The skin screen allows the player to choose a skin.
 */
typedef struct skin_screen skin_screen_t;

/**
 * The generalized level struct that will allow us to create multiple levels
 * later on.
 */
typedef struct level level_t;

/**
 * Button info struct that stores all the assets and paths and handlers
 * to create the buttons on the start screen and levels.
 */
typedef struct button_info button_info_t;

/**
 * Enumeration of different possible screens in the game including the
 * start screen and the different levels.
*/
typedef enum {
  START_SCENE,
  SKIN_SCREEN,
  LEVEL_ONE,
  LEVEL_TWO,
  LEVEL_THREE,
  LEVEL_FOUR,
  LEVEL_FIVE,
  TWO_PLAYER,
} screen_t;

/**
 * Enumeration of different possible skins
*/
typedef enum {
  ELVEN,
  ARCHER,
  GOBLIN,
  KNIGHT
} skin_t;

/**
 * Struct for storing all the information to make a level
 */
typedef struct level_info {
    screen_t screen_name;
    char *background_image_path;
    vector_t inital_character_one_pos;
    double character_one_max_health;
    char *character_one_image_path;
    vector_t inital_character_two_pos;
    double character_two_max_health;
    char *character_two_image_path;
    bool use_ai;
    size_t ai_difficulty;
    vector_t character_2_velocity;
    vector_t level_gravity;
} level_info_t;

/**
 * The start_screen_t object initializer initializing all the buttons on the 
 * start screen. The caller will still need to call sdl_init, asset_cache_init,
 * and set the mouse handlers.
 * 
 * @return a start_screen_t struct
*/
start_screen_t *start_screen_init();

/**
 * The start_screen_t object initializer initializing all the buttons on the 
 * start screen.
*/
skin_screen_t *skin_screen_init();

/**
 * This function will be used in emsripten_main in game.c to render the whole 
 * start screen. Caller should call sdl_clear and sdl_show before and after usage 
 * respectively.
 * 
 * @param screen object that contains all the elements that need to be rendered
 * on the window.
*/
void start_screen_main(start_screen_t *screen);

/**
 * This function will be used in emsripten_main in game.c to render the whole 
 * start screen. Caller should call sdl_clear and sdl_show before and after usage 
 * respectively.
 * 
 * @param screen object that contains all the elements that need to be rendered
 * on the window.
*/
void skin_screen_main(skin_screen_t *screen);

/**
 * This function will free all the elements from screen that were initialized in
 * the init function.
 * 
 * @param screen object that we want to free
*/
void start_screen_free(start_screen_t *screen);

/**
 * This function will free all the elements from screen that were initialized in
 * the init function.
 * 
 * @param screen object that we want to free
*/
void skin_screen_free(skin_screen_t *screen);

/**
 * Initializes a level_t object with all the proper assets required for functionality.
 * The caller will still need to call sdl_init, asset_cache_init, and set the 
 * mouse handlers.
 * 
 * @param level_info which includes all the information for the level.
 * @return a level_t struct
*/
level_t *level_init(level_info_t level_info);

/**
 * Renders all the assets in the level_t object and also runs the functionality of
 * game. Caller should call sdl_clear and sdl_show before and after usage 
 * respectively.
 * 
 * @param level that will be rendered on the screen
*/
void level_main(level_t *level);

/**
 * Sets the start of the drag direction for shooting, corresponding to the current
 * turn. Also makes the helper dots for shooting guidance.
 *
 * @param level the current level
 * @param x x position of the click
 * @param y y position of the click
 */
void level_set_shot_start(level_t *level, double x, double y);

/**
 * Update the helper dots to show the direction in which the shot will be fired.
 * The spacing between the dots is proportional to the initial velocity of the shot.
 * 
 * @param level the current level
*/
void level_update_helper_dots(level_t *level);

/**
 * Starts the countdown until the ai shoots
 * 
 * @param level the current level
 * @param countdown_time the amount of time till the ai shoots
*/
void level_start_ai_countdown(level_t *level, double countdown_time);

/**
 * Decreases the countdown by dt milliseconds until the ai shoots
 * 
 * @param level the current level
 * @param dt time since last tick
 * @return the countdown timer after it is decreased
*/
double level_update_ai_countdown(level_t *level, double dt);

/**
 * Makes the second character shoot the first character automatically.
 * The shot will hit the first character based on some probability.
 * 
 * @param level the current level
*/
void level_ai_shoot(level_t *level);

/**
 * Shoots a bullet based on where the mouse was released after being dragged.
 *
 * @param level the current level
 * @param x x position of the click
 * @param y y position of the click
 */
void level_shoot_shot(level_t *level, double x, double y);

/**
 * Checks if mouse is currently being dragged, meaning that the shot start point
 * has been set. Then shows indication of current shot direction and power.
 *
 * @param level the current level
 * @param x x position of the click
 * @param y y position of the click
 */
void level_shot_drag_update(level_t *level, double x, double y);

/**
 * Switch what character is shooting
 *
 * @param level the current level
 */
void level_cycle_turns(level_t *level);

/**
 * Returns whether the ai is on for this level and its the players turn
 *
 * @param level the current level
 * @return whether or not ai is on and its the players turn currently
 */
bool level_is_player_turn_not_ai(level_t *level);

/**
 * Returns the index of the buttons array that corresponds to where the mouse
 * was clicked.
 *
 * @param x x coordinate of the click
 * @param y y coordinate of the click
 * @return index of the buttons array that was clicked
 */
ssize_t level_get_start_button_index_clicked(double x, double y);

/**
 * Returns the index of the buttons array that corresponds to where the mouse
 * was clicked.
 *
 * @param x x coordinate of the click
 * @param y y coordinate of the click
 * @return index of the buttons array that was clicked
 */
ssize_t level_get_skin_button_index_clicked(double x, double y);

/**
 * Returns the screen name of the level
 *
 * @param level pointer to the current level
 * @return enumeration of screen name
 */
screen_t level_get_screen_name(level_t *level);

/**
 * Returns whether or not the ai is on for this level
 *
 * @param level pointer to the current level
 * @return whether or not the ai is on for this level
 */
bool level_get_use_ai(level_t *level);

/**
 * Returns true if its the left player's turn or false if right player's turn.
 *
 * @param level pointer to the current level
 * @return whether or not it's the first player's turn.
 */
bool level_get_turn(level_t *level);

/**
 * Returns boolean value true if either one of the character's health
 * goes below zero during the game. Return false otherwise
 * 
 * @param level the current level
 * @return true or false
 */
bool level_game_over(level_t *level); 

/**
 * Returns the index of which button that was clicked.
 * 
 * @param x coordinate
 * @param y coordinate
 * @return the index of game over button
 */
ssize_t level_game_over_get_button_index(double x, double y);

/**
 * Returns the list of game over text assets for the current level, so 
 * that they can be displayed at the end of each level
 * 
 * @param level the current level
 * @return list_t of the game over text assets
 */
list_t *level_get_game_over_assets(level_t *level);

/**
 * Returns whether or not there is currently a bullet that has been shot;
 * 
 * @param level the current level
 * @return if there is a bullet that has been shot
 */
bool level_bullet_in_scene(level_t *level);

/**
 * Return the health of the first character in a given level
 * 
 * @param level the current level
 * @return double value of character 1 health
 */
double level_get_character1_health(level_t *level);

/**
 * Return the health of the second character in a given level
 * 
 * @param level the current level
 * @return double value of character 2 health
 */
double level_get_character2_health(level_t *level);

/**
 * Sets the skin display to the corresponding image.
 * 
 * @param screen pointer to the skin screen
 * @param skin the skin to switch the display to
*/
void level_set_skin_screen(skin_screen_t *screen, skin_t skin);

/**
 * Set the skin of the non-ai character to the skin that is given.
 * In the case that it is the two-player level, set skin for both characters.
 * 
 * @param level pointer to the level
 * @param skin skin to switch to
 * @param set_flipped whether to set the second character
*/
void level_set_skin(level_t* level, skin_t skin, bool set_flipped);

/**
 * Frees all the assets and elements in the level inputted. 
*/
void level_free(level_t *level);

#endif // #ifndef __LEVEL_H__