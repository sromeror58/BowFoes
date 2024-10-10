#ifndef __CHARACTER_H__
#define __CHARACTER_H__

#include "body.h"
#include "list.h"
#include "vector.h"
#include "asset.h"
#include <stdbool.h>

/**
 * A character/enemy and its associated health bar
 */
typedef struct character character_t;

/**
 * Initializes a character by making the body asset and health bar asset and
 * adds it to the scene
 *
 * @param init_pos initial position of the character
 * @param max_health maximum health
 * @param img_path path to image of character
 * @param scene scene to display character in 
 * @param health_pos which is the health bar position
 * @return a pointer to a character_t object
 */
character_t *character_init(vector_t init_pos, double max_health, const char *img_path, scene_t *scene, vector_t health_pos);

/**
 * Gets the size of the character
 *
 * @param character pointer to the character
 */
vector_t character_get_size(character_t *character);

/**
 * Gets the body asset of the character
 *
 * @param character pointer to the character
 */
asset_t *character_get_body_asset(character_t *character);

/**
 * Gets the health bar assets of the character
 *
 * @param character pointer to the character
 */
list_t *character_get_health_bar_assets(character_t *character);

/**
 * Gets the current health of the character passed through.
 * 
 * @param character pointer to the character
*/
double character_get_health(character_t *character);

/**
 * Gets the platform bar assets of the character
 *
 * @param character pointer to the character
 */
asset_t *character_get_platform_asset(character_t *character);

/**
 * Gets the platform bar body of the character
 *
 * @param character pointer to the character
 */
body_t *character_get_platform_body(character_t *character); 

/**
 * Gets the body of the character
 *
 * @param character pointer to the character
 */
body_t *character_get_body(character_t *character); 

/**
 * Gets the shot start point - the first coordinate to calculate the shooting
 * direction.
 *
 * @param character pointer to the character
 */
vector_t character_get_shot_start_point(character_t *character);

/**
 * Gets the shot end point - the second coordinate to calculate the shooting
 * direction.
 *
 * @param character pointer to the character
 */
vector_t character_get_shot_end_point(character_t *character);

/**
 * Sets the shot start point - the first coordinate to calculate the shooting
 * direction.
 *
 * @param character pointer to the character
 * @param shot_start_point being the start point of the shot
 */
void character_set_shot_start_point(character_t *character, vector_t shot_start_point);

/**
 * Sets the shot end point - the second coordinate used to calculate the shooting
 * direction.
 *
 * @param character pointer to the character
 * @param shot_end_point being the end point of the shot
 */
void character_set_shot_end_point(character_t *character, vector_t shot_end_point);

/**
 * Reduces the health of the character by damage.
 * 
 * @param character pointer to the character
 * @param damage amount of health to deduct
*/
void character_deduct_health(character_t *character, double damage);

/**
 * Updates the green part of the health bar based on the fraction of the current
 * health to the max health
 * 
 * @param character pointer to a character
*/
void character_update_health_bar(character_t *character);

/**
 * Gets the velocity vector for a shot based on the start and end points of 
 * where the mouse was dragged. First a vector with a magnitude that is
 * a percentage, representing a percentage of the max velocity magnitude possible,
 * is computed, and then scaled by the maximum possible initial speed.
 *
 * @param shot_start_point the first coordinate used to calculate direction of the shot
 * @param shot_end_point the second coordinate used to calculate direction of the shot
 * @param max_speed the max initial speed for the shot
 * @return the inital velocity of the shot
 */
vector_t character_shot_velocity(vector_t shot_start_point, vector_t shot_end_point, double max_speed);

/**
 * Gets the velocity vector for a shot given where the shot is starting from
 * and where it needs to hit. Randomly chooses an angle between (0, 90) and
 * calculates initial velocity to hit target.
 *
 * @param shot_origin where the shot is starting from
 * @param target where the shot needs to hit
 * @param difficulty scale from 0 to 100 of how accurate the ai is, 100 being absolutely accurate
 * @param gravity which is used as a force on the shot
 * @return the inital velocity of the shot
 */
vector_t character_ai_shot_velocity(vector_t shot_origin, vector_t target, double difficulty, vector_t gravity);

/**
 * Sets the character velocity.
 * 
 * @param character to update the position of
 * @param velocity applied on the character
*/
void character_set_velocity(character_t *character, vector_t velocity);

/**
 * Sets the platform velocity.
 * 
 * @param character to set the velocity of
 * @param velocity applied on the body
*/
void character_set_platform_velocity(character_t *character, vector_t velocity);

/**
 * Ensures the vertical back and forth movement of character through returning 
 * true if limit is exceeded otherwise returns false.
 * 
 * @param character that is being checked
 * @param y_min minimum y-value of the character position
 * @param y_max maximum y-value of the character position
*/
bool character_position_limit(character_t *character, double y_min, double y_max);

/**
 * Free memory allocated for a character.
 *
 * @param character pointer to the character to free
 */
void character_free(character_t *character);

#endif // #ifndef __CHARACTER_H__
