#include "pet2d_mvp_a_scene_skeleton.h"

PET_STATIC_ASSERT(pet2d_mvp_a_scene_action_ms_order,
                  PET2D_MVP_A_SCENE_POSE_ACTION_MS >=
                      PET2D_MVP_A_SCENE_MOVE_ACTION_MS);
PET_STATIC_ASSERT(pet2d_mvp_a_scene_has_step_pose,
                  PET2D_MVP_A_SCENE_POSE_STEP < PET2D_MVP_A_SCENE_POSE_MAX);

pet_result_t pet2d_mvp_a_scene_action_loop_compile_check_self_test(void)
{
    return pet2d_mvp_a_scene_action_loop_self_test();
}

pet_result_t pet2d_mvp_a_scene_action_loop_compile_check_contract(void)
{
    pet2d_mvp_a_scene_model_t model;
    pet2d_mvp_a_scene_draw_cmd_t cmd;

    if (pet2d_mvp_a_scene_skeleton_get_model(&model) != PET_RESULT_OK) {
        return PET_RESULT_ERROR;
    }
    if (pet2d_mvp_a_scene_skeleton_get_draw_cmd(&cmd) != PET_RESULT_OK) {
        return PET_RESULT_ERROR;
    }
    if ((cmd.w > PET2D_MVP_A_SCENE_SPRITE_SIZE) ||
        (cmd.h > PET2D_MVP_A_SCENE_SPRITE_SIZE)) {
        return PET_RESULT_ERROR;
    }
    if (model.timeout_ms < model.enter_ms) {
        return PET_RESULT_ERROR;
    }
    return PET_RESULT_OK;
}
