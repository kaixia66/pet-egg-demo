#ifndef SIM_CONSISTENCY_REPLAY_H
#define SIM_CONSISTENCY_REPLAY_H

#include <stddef.h>

#include "pet2d_mvp_a_scene_skeleton.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SIM_CONSISTENCY_STAGE_W PET2D_MVP_A_SCENE_STAGE_W
#define SIM_CONSISTENCY_STAGE_H PET2D_MVP_A_SCENE_STAGE_H
#define SIM_CONSISTENCY_PET_SIZE PET2D_MVP_A_SCENE_SPRITE_SIZE
#define SIM_CONSISTENCY_STEP_PIXELS PET2D_MVP_A_SCENE_STEP_PIXELS
#define SIM_CONSISTENCY_REPLAY_LOG_CAP 2048u

typedef enum {
    SIM_CONSISTENCY_STEP_ENTER = 0,
    SIM_CONSISTENCY_STEP_TICK,
    SIM_CONSISTENCY_STEP_KEY
} sim_consistency_step_type_t;

typedef struct {
    sim_consistency_step_type_t type;
    pet_u32_t at_ms;
    PetProductKey key;
} sim_consistency_replay_step_t;

typedef struct {
    pet2d_mvp_a_scene_model_t model;
    pet2d_mvp_a_render_plan_t plan;
    pet_u32_t skipped_flush_count;
    pet_u8_t pose_cycle;
} sim_consistency_scene_context_t;

pet_result_t sim_consistency_run_scene_replay(char *out_log, size_t out_capacity);
pet_result_t sim_consistency_run_timeout_replay(char *out_log, size_t out_capacity);
pet_result_t sim_consistency_check_screen_profile_fixture(void);
pet_result_t sim_consistency_check_key_replay_fixture(void);
pet_result_t sim_consistency_check_save_slot_fixture(void);
pet_result_t sim_consistency_check_packet_fixture(void);
pet_result_t sim_consistency_run_all(char *out_log, size_t out_capacity);

#ifdef __cplusplus
}
#endif

#endif
