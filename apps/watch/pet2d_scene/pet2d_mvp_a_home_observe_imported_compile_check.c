#include "pet2d_mvp_a_home_observe_imported.h"

#include "pet_platform.h"

PET_STATIC_ASSERT(p36_imported_stage_w,
                  PET2D_MVP_A_HOME_OBSERVE_IMPORTED_STAGE_W == 160u);
PET_STATIC_ASSERT(p36_imported_stage_h,
                  PET2D_MVP_A_HOME_OBSERVE_IMPORTED_STAGE_H == 96u);
PET_STATIC_ASSERT(p36_imported_pet_size,
                  PET2D_MVP_A_HOME_OBSERVE_IMPORTED_SPRITE_SIZE == 32u);
PET_STATIC_ASSERT(p36_imported_timeout,
                  PET2D_MVP_A_HOME_OBSERVE_IMPORTED_TIMEOUT_MS == 4000u);

pet_result_t pet2d_mvp_a_home_observe_imported_compile_check(void)
{
    pet2d_mvp_a_home_observe_imported_model_t model;
    pet2d_mvp_a_render_plan_t plan;
    pet2d_mvp_a_home_observe_imported_stats_t stats;
    pet_result_t ret;

    ret = pet2d_mvp_a_home_observe_imported_self_test();
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    (void)pet2d_mvp_a_home_observe_imported_state_name(
        PET2D_MVP_A_HOME_OBSERVE_IMPORTED_STATE_IDLE);
    (void)pet2d_mvp_a_home_observe_imported_exit_name(
        PET2D_MVP_A_HOME_OBSERVE_IMPORTED_EXIT_CANCEL);
    (void)pet2d_mvp_a_home_observe_imported_is_active();
    (void)pet2d_mvp_a_home_observe_imported_get_model(&model);
    (void)pet2d_mvp_a_home_observe_imported_get_render_plan(&plan);
    (void)pet2d_mvp_a_home_observe_imported_get_stats(&stats);
    return PET_RESULT_OK;
}
