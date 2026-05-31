#include "pet2d_mvp_a_scene_skeleton.h"

PET_STATIC_ASSERT(pet2d_mvp_a_scene_scratch_bound,
                  PET2D_MVP_A_SCENE_STAGE_W <= 96u);
PET_STATIC_ASSERT(pet2d_mvp_a_scene_stats_frame_width,
                  sizeof(((pet2d_mvp_a_scene_stats_t *)0)->frame_count) ==
                      sizeof(pet_u32_t));
PET_STATIC_ASSERT(pet2d_mvp_a_scene_draw_cmd_small,
                  sizeof(pet2d_mvp_a_scene_draw_cmd_t) <= 12u);
PET_STATIC_ASSERT(pet2d_mvp_a_scene_model_has_counter,
                  sizeof(((pet2d_mvp_a_scene_model_t *)0)->frame_index) ==
                      sizeof(pet_u32_t));

pet_result_t pet2d_mvp_a_scene_skeleton_compile_check_self_test(void)
{
    return pet2d_mvp_a_scene_skeleton_self_test();
}

pet_result_t pet2d_mvp_a_scene_skeleton_compile_check_stats(void)
{
    pet2d_mvp_a_scene_stats_t stats;
    pet2d_mvp_a_scene_state_t state;
    pet2d_mvp_a_scene_model_t model;
    pet2d_mvp_a_scene_draw_cmd_t cmd;

    if (pet2d_mvp_a_scene_skeleton_get_stats(&stats) != PET_RESULT_OK) {
        return PET_RESULT_ERROR;
    }
    if (pet2d_mvp_a_scene_skeleton_get_state(&state) != PET_RESULT_OK) {
        return PET_RESULT_ERROR;
    }
    if (pet2d_mvp_a_scene_skeleton_get_model(&model) != PET_RESULT_OK) {
        return PET_RESULT_ERROR;
    }
    if (pet2d_mvp_a_scene_skeleton_get_draw_cmd(&cmd) != PET_RESULT_OK) {
        return PET_RESULT_ERROR;
    }
    if ((state != PET2D_MVP_A_SCENE_STATE_NONE) &&
        (state != PET2D_MVP_A_SCENE_STATE_ENTER) &&
        (state != PET2D_MVP_A_SCENE_STATE_IDLE) &&
        (state != PET2D_MVP_A_SCENE_STATE_MOVE_LEFT) &&
        (state != PET2D_MVP_A_SCENE_STATE_MOVE_RIGHT) &&
        (state != PET2D_MVP_A_SCENE_STATE_ACTION) &&
        (state != PET2D_MVP_A_SCENE_STATE_EXITING) &&
        (state != PET2D_MVP_A_SCENE_STATE_DONE) &&
        (state != PET2D_MVP_A_SCENE_STATE_ERROR)) {
        return PET_RESULT_ERROR;
    }
    (void)stats;
    (void)model;
    (void)cmd;
    return PET_RESULT_OK;
}
