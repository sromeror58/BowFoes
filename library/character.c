#include <assert.h>
#include <math.h>

#include "character.h"
#include "scene.h"
#include "asset.h"
#include "body.h"
#include "vector.h"
#include "sdl_wrapper.h"

const vector_t CHARACTER_SIZE = {120, 120};
const rgb_color_t WHITE = (rgb_color_t){1, 1, 1};
const rgb_color_t GREEN = (rgb_color_t){0, 0.5, 0};;
const rgb_color_t RED = (rgb_color_t){1, 0, 0};
const double HEALTH_BAR_Y_OFFSET = 90.0;
const vector_t HEALTH_BAR_SIZE = {170, 15};
const double HALF_SIZE_SCALE_FACTOR = 0.5;
const vector_t PLATFORM_DIMENSIONS = {122, 32};
const char *PLATFORM_PATH = "assets/platform.png";
const double PLATFORM_BAR_X_OFFSET = -10.0;
const double PLATFORM_BAR_Y_OFFSET = 10.0;
const double MAX_SHOT_MAGNITUDE = 60;
const size_t HEALTH_ASSET_IDX = 1;
const size_t SHAPE_TOP_LEFT_IDX = 0;
const vector_t GRAVITY_VEC = {0, -150};
const size_t MAX_DIFFICULTY = 100;
const double DIFFICULTY_SCALE = 2.0;
const rgb_color_t PLATFORM_COLOR = (rgb_color_t) {0.59, 0.29, 0};
const size_t HEALTH_BAR_ASSET_SIZE = 2;
const double AI_SHOT_ANGLE = (45 * M_PI) / 180.0;


struct character {
    body_t *character_body;
    vector_t size;
    asset_t *body_asset;
    list_t *health_bar_assets;
    double max_health;
    double current_health;
    asset_t *platform_assets;
    body_t *platform_body;
    vector_t shot_start_point;
    vector_t shot_end_point;
};

/**
 * Makes the body for the character
 * 
 * @param pos position of the body
 * @param size a vector of the width and height of the body
 * @return a body of the specified size at the specified position
*/
body_t *make_character_body(vector_t pos, vector_t size) {
    list_t *shape = sdl_make_rectangle(pos.x, pos.y, size.x, size.y);
    body_t *character = body_init(shape, INFINITY, WHITE);
    body_set_centroid(character, vec_add(pos, vec_multiply(HALF_SIZE_SCALE_FACTOR, size)));
    return character;
}

/**
 * Makes two assets: 1) a red bar representing the total health of the character
 * 2) a green bar representing the current health of the character
 * The green bar overlaps the red bar. Also adds them to the scene.
 * 
 * @param current_pos the current position of the character
 * @param max_health the maximum health of the character
 * @param scene to add to
 * @return a list of the two assets that make up the health bar
*/
list_t *make_health_bar(vector_t current_pos, double max_health, scene_t *scene) {
    list_t *health_bar_assets = list_init(HEALTH_BAR_ASSET_SIZE, (free_func_t) asset_destroy);
    vector_t health_bar_pos = (vector_t){current_pos.x, current_pos.y + HEALTH_BAR_Y_OFFSET};
    
    // make red health bar
    list_t *border_shape = sdl_make_rectangle(health_bar_pos.x, health_bar_pos.y, HEALTH_BAR_SIZE.x, HEALTH_BAR_SIZE.y);
    body_t *health_bar_border = body_init(border_shape, INFINITY, RED);
    body_set_centroid(health_bar_border, vec_add(health_bar_pos, vec_multiply(HALF_SIZE_SCALE_FACTOR, HEALTH_BAR_SIZE)));
    scene_add_body(scene, health_bar_border);
    body_get_shape(health_bar_border);
    asset_t *border_asset = asset_make_body(health_bar_border);
    list_add(health_bar_assets, border_asset);

    // make green health bar representing current health
    list_t *health_shape = sdl_make_rectangle(health_bar_pos.x, health_bar_pos.y, HEALTH_BAR_SIZE.x, HEALTH_BAR_SIZE.y);
    body_t *health_bar_health = body_init(health_shape, INFINITY, GREEN);
    body_set_centroid(health_bar_health, vec_add(health_bar_pos, vec_multiply(HALF_SIZE_SCALE_FACTOR, HEALTH_BAR_SIZE)));
    scene_add_body(scene, health_bar_health);
    asset_t *health_asset = asset_make_body(health_bar_health);
    list_add(health_bar_assets, health_asset);
    
    return health_bar_assets;
}

void character_update_health_bar(character_t *character) {
    asset_t *health_bar_asset = list_get(character->health_bar_assets, HEALTH_ASSET_IDX);
    body_t *health_bar = asset_get_body(health_bar_asset);
    list_t *cur_shape = body_get_shape(health_bar);
    vector_t *top_left = list_get(cur_shape, SHAPE_TOP_LEFT_IDX);
    list_t *new_health_shape = sdl_make_rectangle((*top_left).x, (*top_left).y, HEALTH_BAR_SIZE.x * (character->current_health / character->max_health), HEALTH_BAR_SIZE.y);
    body_set_shape(health_bar, new_health_shape);
}

/**
 * Makes the platform asset for the character to be on top of
 * 
 * @param current_pos the current position of the character
 * @param scene the scene to add the body to
 * @return an asset that represents the platform
*/
body_t *make_platform_bar(vector_t current_pos, scene_t *scene) {
    vector_t platform_position = (vector_t){current_pos.x - PLATFORM_BAR_X_OFFSET, current_pos.y + PLATFORM_BAR_Y_OFFSET};
    list_t *platform_shape = sdl_make_rectangle(platform_position.x, platform_position.y, PLATFORM_DIMENSIONS.x, PLATFORM_DIMENSIONS.y);
    body_t *platform_shape_body = body_init(platform_shape, INFINITY, PLATFORM_COLOR);
    return platform_shape_body;
}

/**
 * Makes the platform body for the character to be on top of
 * 
 * @param platform_body which is the body of the platform
 * @param scene to add the body to
 * @param current_pos the current position of the character
 * @return an asset that represents the platform
*/
asset_t *make_platform_asset(body_t *platform_body, scene_t *scene, vector_t current_pos) {
    scene_add_body(scene, platform_body);
    asset_t *platform_asset = asset_make_image_with_body(PLATFORM_PATH, platform_body);
    return platform_asset;
}

character_t *character_init(vector_t init_pos, double max_health, const char *img_path, scene_t *scene, vector_t health_pos) {
  character_t *new_character = malloc(sizeof(character_t));
  assert(new_character);

  // body
  body_t *character = make_character_body(init_pos, CHARACTER_SIZE);
  new_character->character_body = character;
  scene_add_body(scene, character);
  asset_t *character_asset = asset_make_image_with_body(img_path, character);
  new_character->body_asset = character_asset;
  new_character->size = CHARACTER_SIZE;

  // platform
  new_character->platform_body = make_platform_bar(init_pos, scene);
  new_character->platform_assets = make_platform_asset(new_character->platform_body, scene, init_pos);
  
  // health
  new_character->max_health = max_health;
  new_character->current_health = max_health;
  list_t *health_bar_assets = make_health_bar(health_pos, max_health, scene);
  new_character->health_bar_assets = health_bar_assets;

  // shot parameters
  new_character->shot_start_point = VEC_ZERO;
  new_character->shot_end_point = VEC_ZERO;

  return new_character;
}

vector_t character_get_size(character_t *character) {
    return character->size;
}

asset_t *character_get_body_asset(character_t *character) {
    return character->body_asset;
}

list_t *character_get_health_bar_assets(character_t *character) {
    return character->health_bar_assets;
}

double character_get_health(character_t *character) {
    return character->current_health;
}

asset_t *character_get_platform_asset(character_t *character) {
    return character->platform_assets;
}

body_t *character_get_platform_body(character_t *character) {
    return character->platform_body;
}

body_t *character_get_body(character_t *character) {
    return character->character_body;
}

vector_t character_get_shot_start_point(character_t *character) {
    return character->shot_start_point;
}

vector_t character_get_shot_end_point(character_t *character) {
    return character->shot_end_point;
}

void character_set_shot_end_point(character_t *character, vector_t shot_end_point) {
    character->shot_end_point = shot_end_point;
}

void character_set_shot_start_point(character_t *character, vector_t shot_start_point) {
    character->shot_start_point = shot_start_point;
}

void character_deduct_health(character_t *character, double damage) {
    if (damage < character->current_health) {
        character->current_health -= damage;
    }
    else {
        character->current_health = 0;
    }
}

vector_t character_shot_velocity(vector_t shot_start_point, vector_t shot_end_point, double max_speed) {
    if (vec_equals(shot_start_point, shot_end_point)) {
        return VEC_ZERO;
    }
    vector_t direction = vec_subtract(shot_end_point, shot_start_point);
    direction = (vector_t){-1.0 * direction.x, direction.y};
    double vec_magnitude = vec_get_length(direction);
    vector_t shot_unit = vec_multiply(1.0 / vec_magnitude, direction);
    if (vec_magnitude > MAX_SHOT_MAGNITUDE) {
        vec_magnitude = MAX_SHOT_MAGNITUDE;
    }
    double magnitude_percentage = vec_magnitude / MAX_SHOT_MAGNITUDE;
    vector_t shot_velocity = vec_multiply(max_speed*magnitude_percentage, shot_unit);
    return shot_velocity;
}

vector_t character_ai_shot_velocity(vector_t shot_origin, vector_t target, double difficulty, vector_t gravity) {
    double shot_angle = AI_SHOT_ANGLE;
    double x_diff = target.x - shot_origin.x;
    double y_diff = target.y - shot_origin.y;
    double numerator = gravity.y * x_diff * x_diff;
    double denominator = x_diff + y_diff;
    double increase_in_difficulty = (double) rand() / (double)RAND_MAX;
    double init_vel_mag = sqrt(fabs(numerator / denominator)) + (increase_in_difficulty * DIFFICULTY_SCALE * (MAX_DIFFICULTY - difficulty));
    vector_t velocity = {-1.0 * init_vel_mag * cos(shot_angle), init_vel_mag * sin(shot_angle)}; // x coord negated so arrow goes left
    return velocity;
}

void character_set_velocity(character_t *character, vector_t velocity) {
    body_t *char_body = character_get_body(character);
    body_set_velocity(char_body, velocity);
}

void character_set_platform_velocity(character_t *character, vector_t velocity) {
    body_t *platform_body = character_get_platform_body(character);
    body_set_velocity(platform_body, velocity);
}

bool character_position_limit(character_t *character, double y_min, double y_max) {
    body_t *char_body = character_get_body(character);
    vector_t centroid = body_get_centroid(char_body);
    return centroid.y <= y_min || centroid.y >= y_max;
}

void character_free(character_t *character) {
    asset_destroy(character->body_asset);
    asset_destroy(character->platform_assets);
    list_free(character->health_bar_assets);
    free(character);
}
