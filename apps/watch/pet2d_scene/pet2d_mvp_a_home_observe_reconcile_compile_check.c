#include "pet2d_mvp_a_home_observe_reconcile.h"

#include "pet_platform.h"

PET_STATIC_ASSERT(p38_reconcile_exact_count,
                  PET2D_MVP_A_HOME_OBSERVE_RECONCILE_EXACT_MATCH_COUNT == 20u);
PET_STATIC_ASSERT(p38_reconcile_semantic_count,
                  PET2D_MVP_A_HOME_OBSERVE_RECONCILE_SEMANTIC_MATCH_COUNT == 5u);
PET_STATIC_ASSERT(p38_reconcile_non_match_count,
                  PET2D_MVP_A_HOME_OBSERVE_RECONCILE_NON_MATCH_COUNT == 7u);
PET_STATIC_ASSERT(p38_reconcile_stage_w,
                  PET2D_MVP_A_HOME_OBSERVE_RECONCILE_SIM_STAGE_W ==
                  PET2D_MVP_A_HOME_OBSERVE_IMPORTED_STAGE_W);
PET_STATIC_ASSERT(p38_reconcile_timeout,
                  PET2D_MVP_A_HOME_OBSERVE_RECONCILE_SIM_TIMEOUT_MS ==
                  PET2D_MVP_A_HOME_OBSERVE_IMPORTED_TIMEOUT_MS);

pet_result_t pet2d_mvp_a_home_observe_reconcile_compile_check(void)
{
    pet2d_mvp_a_home_observe_reconcile_summary_t summary;
    pet_result_t ret;

    ret = pet2d_mvp_a_home_observe_reconcile_self_test();
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    ret = pet2d_mvp_a_home_observe_reconcile_get_summary(&summary);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    if ((summary.p37_manual_real_board_smoke_verified == 0u) ||
        (summary.home_observe_enabled != 0u) ||
        (summary.full_pet2d_runtime_enabled != 0u) ||
        (summary.pet2d_runtime_enabled != 0u)) {
        return PET_RESULT_ERROR;
    }
    (void)pet2d_mvp_a_home_observe_reconcile_scene_id();
    (void)pet2d_mvp_a_home_observe_reconcile_scene_status();
    return PET_RESULT_OK;
}
