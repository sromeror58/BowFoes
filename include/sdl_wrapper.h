#ifndef __SDL_WRAPPER_H__
#define __SDL_WRAPPER_H__

#include "color.h"
#include "list.h"
#include "polygon.h"
#include "scene.h"
#include "state.h"
#include "vector.h"
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <stdbool.h>

// Values passed to a key handler when the given arrow key is pressed
typedef enum {
  LEFT_ARROW = 1,
  UP_ARROW = 2,
  RIGHT_ARROW = 3,
  DOWN_ARROW = 4,
  SPACE_BAR = 5,
} arrow_key_t;

// Values passed to a key handler when the given arrow key is pressed
typedef enum {
  DRAW_BOW = 0,
  SHOOT = 1,
  HIT = 2
} sound_effect_t;

/**
 * The possible types of key events.
 * Enum types in C are much more primitive than in Java; this is equivalent to:
 * typedef unsigned int KeyEventType;
 * #define KEY_PRESSED 0
 * #define KEY_RELEASED 1
 */
typedef enum { KEY_PRESSED, KEY_RELEASED } key_event_type_t;

/**
 * A keypress handler.
 * When a key is pressed or released, the handler is passed its char value.
 * Most keys are passed as their char value, e.g. 'a', '1', or '\r'.
 * Arrow keys have the special values listed above.
 *
 * @param key a character indicating which key was pressed
 * @param type the type of key event (KEY_PRESSED or KEY_RELEASED)
 * @param held_time if a press event, the time the key has been held in seconds
 */
typedef void (*key_handler_t)(char key, key_event_type_t type, double held_time,
                              void *state);

/**
 * The possible types of mouse events.
 * Enum types in C are much more primitive than in Java; this is equivalent to:
 * typedef unsigned int MouseEventType;
 * #define MOUSE_PRESSED 0
 * #define MOUSE_RELEASED 1
 */
typedef enum { MOUSE_PRESSED, MOUSE_RELEASED } mouse_event_type_t;

/**
 * A mousepress handler.
 * When a mouse is pressed, the handler is passed.
 *
 * @param state the current state the game is at.
 * @param x_loc the x-location that it was pressed at.
 * @param y_loc the y-location that it was pressed at.
 */
typedef void (*state_mouse_handler_t)(state_t *state, double x_loc, double y_loc);

/**
 * A mousepress handler.
 * When a mouse is pressed, the handler is passed.
 *
 * @param level the current level the game is at.
 * @param x_loc the x-location that it was pressed at.
 * @param y_loc the y-location that it was pressed at.
 */
typedef void (*level_mouse_handler_t)(level_t *level, double x_loc, double y_loc);

/**
 * All the mouse handlers used in the game.
*/
typedef struct sdl_mouse_handlers sdl_mouse_handlers_t;

/**
 * Initializes the SDL window and renderer.
 * Must be called once before any of the other SDL functions.
 *
 * @param min the x and y coordinates of the bottom left of the scene
 * @param max the x and y coordinates of the top right of the scene
 */
void sdl_init(vector_t min, vector_t max);

/**
 * Initializes the mouse handlers struct with the provided handler functions.
 * 
 * @param screen_switch The handler for switching scenes.
 * @param skin_screen The handler for the skin screen.
 * @param start The handler for the start of a shot event.
 * @param drag The handler for dragging during a shot event.
 * @param end The handler for the end of a shot event.
 */
void sdl_set_mouse_handlers(state_mouse_handler_t screen_switch, state_mouse_handler_t skin_screen, level_mouse_handler_t start, level_mouse_handler_t drag, level_mouse_handler_t end, state_mouse_handler_t game_over);

/**
 * Processes all SDL events and returns whether the window has been closed.
 * This function must be called in order to handle inputs.
 *
 * @return true if the window was closed, false otherwise
 */
bool sdl_is_done(void *state);

/**
 * Clears the screen. Should be called before drawing polygons in each frame.
 */
void sdl_clear(void);

/**
 * Draws a polygon from the given list of vertices and a color.
 *
 * @param poly a struct representing the polygon
 * @param color the color used to fill in the polygon
 */
void sdl_draw_polygon(polygon_t *poly, rgb_color_t color);

/**
 * Displays the rendered frame on the SDL window.
 * Must be called after drawing the polygons in order to show them.
 */
void sdl_show(void);

/**
 * Draws all bodies in a scene.
 * This internally calls sdl_clear(), sdl_draw_polygon(), and sdl_show(),
 * so those functions should not be called directly.
 *
 * @param scene the scene to draw
 * @param aux an additional body to draw (can be NULL if no additional bodies)
 */
void sdl_render_scene(scene_t *scene, void *aux);

/**
 * Registers a function to be called every time a key is pressed.
 * Overwrites any existing handler.
 *
 * Example:
 * ```
 * void on_key(char key, key_event_type_t type, double held_time) {
 *     if (type == KEY_PRESSED) {
 *         switch (key) {
 *             case 'a':
 *                 printf("A pressed\n");
 *                 break;
 *             case UP_ARROW:
 *                 printf("UP pressed\n");
 *                 break;
 *         }
 *     }
 * }
 * int main(void) {
 *     sdl_on_key(on_key);
 *     while (!sdl_is_done());
 * }
 * ```
 *
 * @param handler the function to call with each key press
 */
void sdl_on_key(key_handler_t handler);

/**
 * Gets the amount of time that has passed since the last time
 * this function was called, in seconds.
 *
 * @return the number of seconds that have elapsed
 */
double time_since_last_tick(void);

/**
 * This method will verify if the renderer is not NULL and then use the
 * IMG_LoadTexture() method to load our image. Free/Deleting the image would be
 * required by the caller.
 *
 * @param image_path path to image to load
 * @return the SDL_Texture of an image
 */
SDL_Texture *load_image(const char *image_path);

/**
 * This method clears the renderer and then positions the image on the window
 * using the bounds parameter. You will need to clear the window before running
 * this methods and also show the window after calling this method.
 *
 * @param img pointer to the texture that is drawn
 * @param bounds the dimensions and parameters of the image
 */
void sdl_draw_image(SDL_Texture *img, SDL_Rect bounds);

/**
 * This method clears the renderer and then positions the image on the window
 * using the bounds parameter. Also rotates the image by a given angle.
 *
 * @param img pointer to the texture that is drawn
 * @param bounds the dimensions and parameters of the image
 * @param rot angle to rotate the image
 */
void sdl_draw_image_with_angle(SDL_Texture *img, SDL_Rect bounds, double rot);

/**
 * Opens a font style with a certain font size.
 *
 * @param font_style_path Path to the .ttf font style file
 * @param font_size Size of the font
 */
TTF_Font *load_font(const char *font_style_path, size_t font_size);

/**
 * Creates surfaces and textures needed for the given text and then
 * renders the text texture with given a font style.
 *
 * @param text text to draw
 * @param font font style of the text
 * @param color the color for the text
 * @param bounds the dimensions and parameters of the text
 */
void sdl_draw_text(const char *text, TTF_Font *font, rgb_color_t color,
                   SDL_Rect bounds);

/**
 * @brief Checks if a mouse click event has occurred.
 *
 * This function returns true if a mouse click event has occurred since the last
 * call to this function, and false otherwise.
 *
 * @return true if a mouse click event has occurred, false otherwise.
 */
bool sdl_is_mouse_click(void);

/**
 * Plays the given sound effect using SDL mixer.
 *
 * @param sound_effect which sound effect to play.
 */
void sdl_play_sound_effect(sound_effect_t sound_effect);

/**
 * Changes the volume of the music. Volume can be between 0 and 128.
 *
 * @param volume the volume to set the music
 */
void sdl_set_music_volume(size_t volume);

/**
 * Takes in the dimensions and location of the object that we want to puton the
 * screen and returns a SDL_Rect object.
 *
 * @param h the height of the object
 * @param w the width of the object
 * @param x the x position of the top left corner of the object
 * @param y the y position of the top left corner of the object
 *
 * @return an SDL_Rect object of the dimensions and location of the object.

*/
SDL_Rect sdl_get_bounds(size_t h, size_t w, size_t x, size_t y);

/**
 * Checks if a location on the screen, given by inputs x and y, is contained
 * within a bounding box
 *
 * @param body a pointer to a body returned from body_init()
 * @return an SDL_Rect object with the dimensions for the bounding box
 */
SDL_Rect bounding_box(body_t *body);

/**
 * Checks if a location on the screen, given by inputs x and y, is contained
 * within a bounding box
 *
 * @param x the x location
 * @param y the y location
 * @param bounding_box bounding box to check for containment
 * @return boolean for whether or not the location is in the bounding box
 */
bool sdl_contained_in_box(double x, double y, SDL_Rect bounding_box);

/**
 * Return a list of coordinates corresponding to a rectangle
 * 
 * @param x the x coordinate of the top left
 * @param y the y coordinate of the top left
 * @param w the width of the rectangle
 * @param h the height of the rectangle
 * @return list of vectors used to define the rectangle
*/ 
list_t *sdl_make_rectangle(double x, double y, double w, double h);

#endif // #ifndef __SDL_WRAPPER_H__
