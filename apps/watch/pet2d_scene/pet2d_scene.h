#ifndef PET2D_SCENE_H
#define PET2D_SCENE_H

#include "pet_key.h"
#include "pet_platform.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PET2D_SCENE_TEST_TIMEOUT_MS 4000u
#define PET2D_SCENE_RENDER_INTERVAL_MS 250u

typedef enum {
    PET2D_SCENE_STATE_IDLE = 0,
    PET2D_SCENE_STATE_ENTERING,
    PET2D_SCENE_STATE_RUNNING,
    PET2D_SCENE_STATE_EXITING,
    PET2D_SCENE_STATE_DONE,
    PET2D_SCENE_STATE_ERROR
} pet2d_scene_state_t;

typedef enum {
    PET2D_SCENE_EXIT_NONE = 0,
    PET2D_SCENE_EXIT_CANCEL,
    PET2D_SCENE_EXIT_TIMEOUT,
    PET2D_SCENE_EXIT_ERROR
} pet2d_scene_exit_reason_t;

typedef struct {
    pet_u32_t enter_count;
    pet_u32_t exit_count;
    pet_u32_t error_count;
    pet_u32_t tick_count;
    pet_u32_t key_count;
    pet_u32_t render_count;
    pet_u32_t flush_success_count;
    pet_u32_t flush_fail_count;
    pet_u32_t last_enter_ms;
    pet_u32_t last_exit_ms;
    pet_u32_t last_duration_ms;
    pet_u32_t max_duration_ms;
    pet_u32_t last_key_to_flush_ms;
    pet_u8_t last_exit_reason;
    pet_u8_t last_state;
} pet2d_scene_stats_t;

pet_result_t pet2d_scene_enter_test(void);
pet_result_t pet2d_scene_tick(pet_u32_t now_ms);
pet_result_t pet2d_scene_handle_key(const pet_key_event_t *event);
pet_result_t pet2d_scene_exit(void);
pet_bool_t pet2d_scene_is_active(void);
pet_result_t pet2d_scene_get_state(pet2d_scene_state_t *out_state);
pet_result_t pet2d_scene_get_stats(pet2d_scene_stats_t *out_stats);
pet_result_t pet2d_scene_reset_stats(void);
pet_result_t pet2d_scene_self_test(void);

#ifdef __cplusplus
}
#endif

#endif
