#ifndef PET2D_MVP_A_HOME_OBSERVE_IMPORTED_H
#define PET2D_MVP_A_HOME_OBSERVE_IMPORTED_H

#include "pet_key.h"
#include "pet2d_mvp_a_renderer_contract.h"
#include "pet_platform.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PET2D_MVP_A_HOME_OBSERVE_IMPORTED_STAGE_W 160u
#define PET2D_MVP_A_HOME_OBSERVE_IMPORTED_STAGE_H 96u
#define PET2D_MVP_A_HOME_OBSERVE_IMPORTED_VIEWPORT_W 160u
#define PET2D_MVP_A_HOME_OBSERVE_IMPORTED_VIEWPORT_H 96u
#define PET2D_MVP_A_HOME_OBSERVE_IMPORTED_SPRITE_SIZE 32u
#define PET2D_MVP_A_HOME_OBSERVE_IMPORTED_INITIAL_X 64
#define PET2D_MVP_A_HOME_OBSERVE_IMPORTED_INITIAL_Y 32
#define PET2D_MVP_A_HOME_OBSERVE_IMPORTED_STEP_PIXELS 8u
#define PET2D_MVP_A_HOME_OBSERVE_IMPORTED_MOVE_MS 160u
#define PET2D_MVP_A_HOME_OBSERVE_IMPORTED_ACTION_MS 300u
#define PET2D_MVP_A_HOME_OBSERVE_IMPORTED_TIMEOUT_MS 4000u

typedef enum {
    PET2D_MVP_A_HOME_OBSERVE_IMPORTED_STATE_NONE = 0,
    PET2D_MVP_A_HOME_OBSERVE_IMPORTED_STATE_ENTER,
    PET2D_MVP_A_HOME_OBSERVE_IMPORTED_STATE_IDLE,
    PET2D_MVP_A_HOME_OBSERVE_IMPORTED_STATE_MOVE_LEFT,
    PET2D_MVP_A_HOME_OBSERVE_IMPORTED_STATE_MOVE_RIGHT,
    PET2D_MVP_A_HOME_OBSERVE_IMPORTED_STATE_ACTION,
    PET2D_MVP_A_HOME_OBSERVE_IMPORTED_STATE_EXITING,
    PET2D_MVP_A_HOME_OBSERVE_IMPORTED_STATE_DONE,
    PET2D_MVP_A_HOME_OBSERVE_IMPORTED_STATE_ERROR
} pet2d_mvp_a_home_observe_imported_state_t;

typedef enum {
    PET2D_MVP_A_HOME_OBSERVE_IMPORTED_EXIT_NONE = 0,
    PET2D_MVP_A_HOME_OBSERVE_IMPORTED_EXIT_CANCEL,
    PET2D_MVP_A_HOME_OBSERVE_IMPORTED_EXIT_TIMEOUT,
    PET2D_MVP_A_HOME_OBSERVE_IMPORTED_EXIT_END,
    PET2D_MVP_A_HOME_OBSERVE_IMPORTED_EXIT_ERROR
} pet2d_mvp_a_home_observe_imported_exit_reason_t;

typedef struct {
    pet2d_mvp_a_home_observe_imported_state_t state;
    pet_u8_t pose;
    pet_i16_t pet_x;
    pet_i16_t pet_y;
    pet_i16_t prev_x;
    pet_i16_t prev_y;
    pet_u32_t frame_index;
    pet_u32_t enter_ms;
    pet_u32_t timeout_ms;
    pet_u32_t action_started_ms;
    pet_u32_t action_duration_ms;
    pet2d_mvp_a_home_observe_imported_exit_reason_t exit_reason;
} pet2d_mvp_a_home_observe_imported_model_t;

typedef struct {
    pet_u32_t enter_count;
    pet_u32_t exit_count;
    pet_u32_t error_count;
    pet_u32_t tick_count;
    pet_u32_t key_event_count;
    pet_u32_t action_done_count;
    pet_u32_t render_count;
    pet_u32_t flush_success_count;
    pet_u32_t flush_fail_count;
    pet_u32_t flush_skipped_count;
    pet_u32_t last_enter_ms;
    pet_u32_t last_exit_ms;
    pet_u32_t last_duration_ms;
    pet_u16_t last_dirty_w;
    pet_u16_t last_dirty_h;
    pet_u16_t last_pet_x;
    pet_u16_t last_pet_y;
    pet_u8_t last_pose;
    pet_u8_t last_state;
    pet_u8_t last_exit_reason;
    pet_u8_t last_result;
} pet2d_mvp_a_home_observe_imported_stats_t;

const char *pet2d_mvp_a_home_observe_imported_state_name(
    pet2d_mvp_a_home_observe_imported_state_t state);
const char *pet2d_mvp_a_home_observe_imported_exit_name(
    pet2d_mvp_a_home_observe_imported_exit_reason_t reason);

pet_result_t pet2d_mvp_a_home_observe_imported_enter(void);
pet_result_t pet2d_mvp_a_home_observe_imported_tick(pet_u32_t now_ms);
pet_result_t pet2d_mvp_a_home_observe_imported_handle_key(
    const pet_key_event_t *event);
pet_result_t pet2d_mvp_a_home_observe_imported_exit(void);
pet_bool_t pet2d_mvp_a_home_observe_imported_is_active(void);
pet_result_t pet2d_mvp_a_home_observe_imported_get_model(
    pet2d_mvp_a_home_observe_imported_model_t *out_model);
pet_result_t pet2d_mvp_a_home_observe_imported_get_render_plan(
    pet2d_mvp_a_render_plan_t *out_plan);
pet_result_t pet2d_mvp_a_home_observe_imported_get_stats(
    pet2d_mvp_a_home_observe_imported_stats_t *out_stats);
pet_result_t pet2d_mvp_a_home_observe_imported_self_test(void);

#ifdef __cplusplus
}
#endif

#endif
