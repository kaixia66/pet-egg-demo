#ifndef PET2D_MVP_A_SCENE_SKELETON_H
#define PET2D_MVP_A_SCENE_SKELETON_H

#include "pet_key.h"
#include "pet_platform.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PET2D_MVP_A_SCENE_TIMEOUT_MS 4000u
#define PET2D_MVP_A_SCENE_RENDER_INTERVAL_MS 250u
#define PET2D_MVP_A_SCENE_SPRITE_SIZE 32u
#define PET2D_MVP_A_SCENE_STAGE_W 96u
#define PET2D_MVP_A_SCENE_STAGE_H 64u
#define PET2D_MVP_A_SCENE_STEP_PIXELS 8u
#define PET2D_MVP_A_SCENE_MOVE_ACTION_MS 160u
#define PET2D_MVP_A_SCENE_POSE_ACTION_MS 300u

typedef enum {
    PET2D_MVP_A_SCENE_STATE_NONE = 0,
    PET2D_MVP_A_SCENE_STATE_ENTER,
    PET2D_MVP_A_SCENE_STATE_IDLE,
    PET2D_MVP_A_SCENE_STATE_MOVE_LEFT,
    PET2D_MVP_A_SCENE_STATE_MOVE_RIGHT,
    PET2D_MVP_A_SCENE_STATE_ACTION,
    PET2D_MVP_A_SCENE_STATE_EXITING,
    PET2D_MVP_A_SCENE_STATE_DONE,
    PET2D_MVP_A_SCENE_STATE_ERROR
} pet2d_mvp_a_scene_state_t;

typedef enum {
    PET2D_MVP_A_SCENE_EXIT_NONE = 0,
    PET2D_MVP_A_SCENE_EXIT_CANCEL,
    PET2D_MVP_A_SCENE_EXIT_TIMEOUT,
    PET2D_MVP_A_SCENE_EXIT_ERROR
} pet2d_mvp_a_scene_exit_reason_t;

typedef enum {
    PET2D_MVP_A_SCENE_POSE_IDLE = 0,
    PET2D_MVP_A_SCENE_POSE_HAPPY,
    PET2D_MVP_A_SCENE_POSE_BLINK,
    PET2D_MVP_A_SCENE_POSE_STEP,
    PET2D_MVP_A_SCENE_POSE_MAX
} pet2d_mvp_a_scene_pose_t;

typedef struct {
    pet_i16_t x;
    pet_i16_t y;
    pet_u16_t w;
    pet_u16_t h;
    pet_u8_t pose;
    pet_u8_t pattern_id;
    pet_u8_t flags;
    pet_u8_t reserved;
} pet2d_mvp_a_scene_draw_cmd_t;

typedef struct {
    pet2d_mvp_a_scene_state_t state;
    pet2d_mvp_a_scene_pose_t pose;
    pet_i16_t pet_x;
    pet_i16_t pet_y;
    pet_i16_t prev_x;
    pet_i16_t prev_y;
    pet_u32_t frame_index;
    pet_u32_t action_started_ms;
    pet_u32_t action_duration_ms;
    pet_u32_t enter_ms;
    pet_u32_t timeout_ms;
    pet2d_mvp_a_scene_exit_reason_t exit_reason;
} pet2d_mvp_a_scene_model_t;

typedef struct {
    pet_u32_t enter_count;
    pet_u32_t exit_count;
    pet_u32_t error_count;
    pet_u32_t tick_count;
    pet_u32_t key_event_count;
    pet_u32_t action_toggle_count;
    pet_u32_t action_done_count;
    pet_u32_t frame_count;
    pet_u32_t render_count;
    pet_u32_t flush_success_count;
    pet_u32_t flush_fail_count;
    pet_u32_t flush_skipped_count;

    pet_u32_t logic_total_ms;
    pet_u32_t render_total_ms;
    pet_u32_t flush_total_ms;
    pet_u32_t frame_total_ms;
    pet_u32_t logic_max_ms;
    pet_u32_t render_max_ms;
    pet_u32_t flush_max_ms;
    pet_u32_t frame_max_ms;

    pet_u32_t last_enter_ms;
    pet_u32_t last_exit_ms;
    pet_u32_t last_duration_ms;
    pet_u32_t max_duration_ms;
    pet_u16_t last_dirty_x;
    pet_u16_t last_dirty_y;
    pet_u16_t last_dirty_w;
    pet_u16_t last_dirty_h;
    pet_u16_t last_pet_x;
    pet_u16_t last_pet_y;
    pet_u8_t last_pose;
    pet_u8_t last_exit_reason;
    pet_u8_t last_state;
    pet_u8_t last_result;
} pet2d_mvp_a_scene_stats_t;

pet_result_t pet2d_mvp_a_scene_skeleton_enter(void);
pet_result_t pet2d_mvp_a_scene_skeleton_tick(pet_u32_t now_ms);
pet_result_t pet2d_mvp_a_scene_skeleton_handle_key(const pet_key_event_t *event);
pet_result_t pet2d_mvp_a_scene_skeleton_exit(void);
pet_bool_t pet2d_mvp_a_scene_skeleton_is_active(void);
pet_result_t pet2d_mvp_a_scene_skeleton_get_state(pet2d_mvp_a_scene_state_t *out_state);
pet_result_t pet2d_mvp_a_scene_skeleton_get_model(pet2d_mvp_a_scene_model_t *out_model);
pet_result_t pet2d_mvp_a_scene_skeleton_get_draw_cmd(
    pet2d_mvp_a_scene_draw_cmd_t *out_cmd);
pet_result_t pet2d_mvp_a_scene_skeleton_get_stats(pet2d_mvp_a_scene_stats_t *out_stats);
pet_result_t pet2d_mvp_a_scene_skeleton_reset_stats(void);
pet_result_t pet2d_mvp_a_scene_action_loop_self_test(void);
pet_result_t pet2d_mvp_a_scene_skeleton_self_test(void);

#ifdef __cplusplus
}
#endif

#endif
