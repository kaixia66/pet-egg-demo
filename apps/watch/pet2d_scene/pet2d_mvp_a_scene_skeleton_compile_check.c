#include "pet2d_mvp_a_scene_skeleton.h"

PET_STATIC_ASSERT(pet2d_mvp_a_scene_scratch_bound,
                  PET2D_MVP_A_SCENE_STAGE_W <= 96u);
PET_STATIC_ASSERT(pet2d_mvp_a_scene_stats_frame_width,
                  sizeof(((pet2d_mvp_a_scene_stats_t *)0)->frame_count) ==
                      sizeof(pet_u32_t));

pet_result_t pet2d_mvp_a_scene_skeleton_compile_check_self_test(void)
{
    return pet2d_mvp_a_scene_skeleton_self_test();
}

pet_result_t pet2d_mvp_a_scene_skeleton_compile_check_stats(void)
{
    pet2d_mvp_a_scene_stats_t stats;
    pet2d_mvp_a_scene_state_t state;

    if (pet2d_mvp_a_scene_skeleton_get_stats(&stats) != PET_RESULT_OK) {
        return PET_RESULT_ERROR;
    }
    if (pet2d_mvp_a_scene_skeleton_get_state(&state) != PET_RESULT_OK) {
        return PET_RESULT_ERROR;
    }
    if ((state != PET2D_MVP_A_SCENE_STATE_IDLE) &&
        (state != PET2D_MVP_A_SCENE_STATE_ENTERING) &&
        (state != PET2D_MVP_A_SCENE_STATE_RUNNING) &&
        (state != PET2D_MVP_A_SCENE_STATE_EXITING) &&
        (state != PET2D_MVP_A_SCENE_STATE_DONE) &&
        (state != PET2D_MVP_A_SCENE_STATE_ERROR)) {
        return PET_RESULT_ERROR;
    }
    (void)stats;
    return PET_RESULT_OK;
}
