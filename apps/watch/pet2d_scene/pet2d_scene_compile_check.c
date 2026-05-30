#include "pet2d_scene.h"

PET_STATIC_ASSERT(pet2d_scene_stats_has_expected_width,
                  sizeof(((pet2d_scene_stats_t *)0)->enter_count) == sizeof(pet_u32_t));

pet_result_t pet2d_scene_compile_check_state(void)
{
    pet2d_scene_state_t state;

    if (pet2d_scene_get_state(&state) != PET_RESULT_OK) {
        return PET_RESULT_ERROR;
    }
    if ((state != PET2D_SCENE_STATE_IDLE) &&
        (state != PET2D_SCENE_STATE_ENTERING) &&
        (state != PET2D_SCENE_STATE_RUNNING) &&
        (state != PET2D_SCENE_STATE_EXITING) &&
        (state != PET2D_SCENE_STATE_DONE) &&
        (state != PET2D_SCENE_STATE_ERROR)) {
        return PET_RESULT_ERROR;
    }
    return PET_RESULT_OK;
}

pet_result_t pet2d_scene_compile_check_self_test(void)
{
    return pet2d_scene_self_test();
}
