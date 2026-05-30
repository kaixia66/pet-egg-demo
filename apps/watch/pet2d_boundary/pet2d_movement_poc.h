#ifndef PET2D_MOVEMENT_POC_H
#define PET2D_MOVEMENT_POC_H

#include "pet2d_minimal_visual.h"
#include "pet_platform.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PET2D_MOVEMENT_POC_SURFACE_SIZE 32u
#define PET2D_MOVEMENT_POC_STEP 16u

typedef struct {
    pet_i16_t x;
    pet_i16_t y;
    pet_i16_t last_x;
    pet_i16_t last_y;
    pet_u16_t surface_w;
    pet_u16_t surface_h;
    pet_u16_t step;
    pet_u32_t frame_count;
    pet_u8_t pattern_toggle;
    pet_u8_t exit_requested;
} pet2d_movement_poc_state_t;

typedef struct {
    pet_u32_t movement_probe_attempt_count;
    pet_u32_t movement_probe_success_count;
    pet_u32_t movement_probe_fail_count;
    pet_key_t last_key;
    pet_key_action_t last_event;
    pet_i16_t last_x;
    pet_i16_t last_y;
    pet_i16_t last_dirty_x;
    pet_i16_t last_dirty_y;
    pet_u16_t last_dirty_w;
    pet_u16_t last_dirty_h;
    pet_result_t last_result;
} pet2d_movement_poc_stats_t;

pet_result_t pet2d_movement_poc_init(void);
pet_result_t pet2d_movement_poc_handle_key(const pet_key_event_t *event);
pet_result_t pet2d_movement_poc_render_once(void);
pet_result_t pet2d_boundary_movement_probe_step(pet_key_t key);
pet_result_t pet2d_movement_poc_get_state(pet2d_movement_poc_state_t *out_state);
pet_result_t pet2d_movement_poc_get_stats(pet2d_movement_poc_stats_t *out_stats);
pet_result_t pet2d_movement_poc_reset_stats(void);
pet_result_t pet2d_movement_poc_self_test(void);

#ifdef __cplusplus
}
#endif

#endif
