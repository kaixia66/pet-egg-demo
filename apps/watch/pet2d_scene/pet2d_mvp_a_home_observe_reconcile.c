#include "pet2d_mvp_a_home_observe_reconcile.h"

static pet_result_t reconcile_check_imported_constants(void)
{
    if ((PET2D_MVP_A_HOME_OBSERVE_IMPORTED_STAGE_W !=
         PET2D_MVP_A_HOME_OBSERVE_RECONCILE_SIM_STAGE_W) ||
        (PET2D_MVP_A_HOME_OBSERVE_IMPORTED_STAGE_H !=
         PET2D_MVP_A_HOME_OBSERVE_RECONCILE_SIM_STAGE_H) ||
        (PET2D_MVP_A_HOME_OBSERVE_IMPORTED_VIEWPORT_W !=
         PET2D_MVP_A_HOME_OBSERVE_RECONCILE_SIM_VIEWPORT_W) ||
        (PET2D_MVP_A_HOME_OBSERVE_IMPORTED_VIEWPORT_H !=
         PET2D_MVP_A_HOME_OBSERVE_RECONCILE_SIM_VIEWPORT_H) ||
        (PET2D_MVP_A_HOME_OBSERVE_IMPORTED_SPRITE_SIZE !=
         PET2D_MVP_A_HOME_OBSERVE_RECONCILE_SIM_PET_W) ||
        (PET2D_MVP_A_HOME_OBSERVE_IMPORTED_SPRITE_SIZE !=
         PET2D_MVP_A_HOME_OBSERVE_RECONCILE_SIM_PET_H) ||
        (PET2D_MVP_A_HOME_OBSERVE_IMPORTED_INITIAL_X !=
         PET2D_MVP_A_HOME_OBSERVE_RECONCILE_SIM_INITIAL_X) ||
        (PET2D_MVP_A_HOME_OBSERVE_IMPORTED_INITIAL_Y !=
         PET2D_MVP_A_HOME_OBSERVE_RECONCILE_SIM_INITIAL_Y) ||
        (PET2D_MVP_A_HOME_OBSERVE_IMPORTED_STEP_PIXELS !=
         PET2D_MVP_A_HOME_OBSERVE_RECONCILE_SIM_MOVE_STEP) ||
        (PET2D_MVP_A_HOME_OBSERVE_IMPORTED_MOVE_MS !=
         PET2D_MVP_A_HOME_OBSERVE_RECONCILE_SIM_MOVE_MS) ||
        (PET2D_MVP_A_HOME_OBSERVE_IMPORTED_ACTION_MS !=
         PET2D_MVP_A_HOME_OBSERVE_RECONCILE_SIM_ACTION_MS) ||
        (PET2D_MVP_A_HOME_OBSERVE_IMPORTED_TIMEOUT_MS !=
         PET2D_MVP_A_HOME_OBSERVE_RECONCILE_SIM_TIMEOUT_MS)) {
        return PET_RESULT_ERROR;
    }

    return PET_RESULT_OK;
}

static pet_result_t reconcile_check_dirty_contract(void)
{
    pet2d_mvp_a_rect_t stage = {
        0,
        0,
        PET2D_MVP_A_HOME_OBSERVE_IMPORTED_STAGE_W,
        PET2D_MVP_A_HOME_OBSERVE_IMPORTED_STAGE_H
    };
    pet2d_mvp_a_rect_t pet = {
        PET2D_MVP_A_HOME_OBSERVE_IMPORTED_INITIAL_X,
        PET2D_MVP_A_HOME_OBSERVE_IMPORTED_INITIAL_Y,
        PET2D_MVP_A_HOME_OBSERVE_IMPORTED_SPRITE_SIZE,
        PET2D_MVP_A_HOME_OBSERVE_IMPORTED_SPRITE_SIZE
    };
    pet2d_mvp_a_rect_t left_pet = {
        (pet_i16_t)(PET2D_MVP_A_HOME_OBSERVE_IMPORTED_INITIAL_X -
                    (pet_i16_t)PET2D_MVP_A_HOME_OBSERVE_IMPORTED_STEP_PIXELS),
        PET2D_MVP_A_HOME_OBSERVE_IMPORTED_INITIAL_Y,
        PET2D_MVP_A_HOME_OBSERVE_IMPORTED_SPRITE_SIZE,
        PET2D_MVP_A_HOME_OBSERVE_IMPORTED_SPRITE_SIZE
    };
    pet2d_mvp_a_render_plan_t plan;
    pet_result_t ret;

    ret = pet2d_mvp_a_renderer_build_initial_plan(&stage, &pet, 0u, &plan);
    if ((ret != PET_RESULT_OK) ||
        (plan.dirty_rect.w != PET2D_MVP_A_HOME_OBSERVE_RECONCILE_DIRTY_ENTER_W) ||
        (plan.dirty_rect.h != PET2D_MVP_A_HOME_OBSERVE_RECONCILE_DIRTY_ENTER_H) ||
        (plan.cmd_count != 2u)) {
        return PET_RESULT_ERROR;
    }

    ret = pet2d_mvp_a_renderer_build_pet_change_plan(&stage,
                                                     &pet,
                                                     &left_pet,
                                                     3u,
                                                     PET_TRUE,
                                                     &plan);
    if ((ret != PET_RESULT_OK) ||
        (plan.dirty_rect.w != PET2D_MVP_A_HOME_OBSERVE_RECONCILE_DIRTY_MOVE_W) ||
        (plan.dirty_rect.h != PET2D_MVP_A_HOME_OBSERVE_RECONCILE_DIRTY_MOVE_H) ||
        (plan.cmd_count != 2u)) {
        return PET_RESULT_ERROR;
    }

    ret = pet2d_mvp_a_renderer_build_pet_change_plan(&stage,
                                                     &pet,
                                                     &pet,
                                                     1u,
                                                     PET_FALSE,
                                                     &plan);
    if ((ret != PET_RESULT_OK) ||
        (plan.dirty_rect.w != PET2D_MVP_A_HOME_OBSERVE_RECONCILE_DIRTY_ACTION_W) ||
        (plan.dirty_rect.h != PET2D_MVP_A_HOME_OBSERVE_RECONCILE_DIRTY_ACTION_H) ||
        (plan.cmd_count != 1u)) {
        return PET_RESULT_ERROR;
    }

    ret = pet2d_mvp_a_renderer_build_idle_plan(&plan);
    if ((ret != PET_RESULT_OK) || (plan.cmd_count != 0u) ||
        (plan.skipped_flush == 0u) ||
        (pet2d_mvp_a_rect_is_valid(&plan.dirty_rect) != PET_FALSE)) {
        return PET_RESULT_ERROR;
    }

    return PET_RESULT_OK;
}

static pet_result_t reconcile_check_exit_semantics(void)
{
    if ((PET2D_MVP_A_HOME_OBSERVE_IMPORTED_EXIT_CANCEL != 1) ||
        (PET2D_MVP_A_HOME_OBSERVE_IMPORTED_EXIT_TIMEOUT != 2) ||
        (PET2D_MVP_A_HOME_OBSERVE_IMPORTED_EXIT_END != 3) ||
        (PET2D_MVP_A_HOME_OBSERVE_IMPORTED_EXIT_ERROR != 4)) {
        return PET_RESULT_ERROR;
    }

    if ((PET2D_MVP_A_HOME_OBSERVE_IMPORTED_STATE_IDLE != 2) ||
        (PET2D_MVP_A_HOME_OBSERVE_IMPORTED_STATE_MOVE_LEFT != 3) ||
        (PET2D_MVP_A_HOME_OBSERVE_IMPORTED_STATE_MOVE_RIGHT != 4) ||
        (PET2D_MVP_A_HOME_OBSERVE_IMPORTED_STATE_ACTION != 5) ||
        (PET2D_MVP_A_HOME_OBSERVE_IMPORTED_STATE_DONE != 7)) {
        return PET_RESULT_ERROR;
    }

    return PET_RESULT_OK;
}

const char *pet2d_mvp_a_home_observe_reconcile_scene_id(void)
{
    return "MVP_A_HOME_OBSERVE_PLACEHOLDER";
}

const char *pet2d_mvp_a_home_observe_reconcile_scene_status(void)
{
    return "test_only_imported_placeholder";
}

pet_result_t pet2d_mvp_a_home_observe_reconcile_get_summary(
    pet2d_mvp_a_home_observe_reconcile_summary_t *out_summary)
{
    if (out_summary == 0) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    out_summary->exact_match_count =
        PET2D_MVP_A_HOME_OBSERVE_RECONCILE_EXACT_MATCH_COUNT;
    out_summary->semantic_match_count =
        PET2D_MVP_A_HOME_OBSERVE_RECONCILE_SEMANTIC_MATCH_COUNT;
    out_summary->explicit_non_match_count =
        PET2D_MVP_A_HOME_OBSERVE_RECONCILE_NON_MATCH_COUNT;
    out_summary->imported_constants_match =
        (reconcile_check_imported_constants() == PET_RESULT_OK) ? 1u : 0u;
    out_summary->imported_dirty_rules_match =
        (reconcile_check_dirty_contract() == PET_RESULT_OK) ? 1u : 0u;
    out_summary->imported_exit_semantics_match =
        (reconcile_check_exit_semantics() == PET_RESULT_OK) ? 1u : 0u;
    out_summary->p37_manual_real_board_smoke_verified = 1u;
    out_summary->host_crc_is_jieli_lcd_crc = 0u;
    out_summary->sdl_visible_parity_claimed = 0u;
    out_summary->production_resource_parity_claimed = 0u;
    out_summary->home_observe_enabled = 0u;
    out_summary->full_pet2d_runtime_enabled = 0u;
    out_summary->pet2d_runtime_enabled = 0u;

    return PET_RESULT_OK;
}

pet_result_t pet2d_mvp_a_home_observe_reconcile_self_test(void)
{
    pet2d_mvp_a_home_observe_reconcile_summary_t summary;
    pet_result_t ret;

    ret = reconcile_check_imported_constants();
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    ret = reconcile_check_dirty_contract();
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    ret = reconcile_check_exit_semantics();
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    ret = pet2d_mvp_a_home_observe_imported_self_test();
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    ret = pet2d_mvp_a_home_observe_reconcile_get_summary(&summary);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    if ((summary.exact_match_count !=
         PET2D_MVP_A_HOME_OBSERVE_RECONCILE_EXACT_MATCH_COUNT) ||
        (summary.semantic_match_count !=
         PET2D_MVP_A_HOME_OBSERVE_RECONCILE_SEMANTIC_MATCH_COUNT) ||
        (summary.explicit_non_match_count !=
         PET2D_MVP_A_HOME_OBSERVE_RECONCILE_NON_MATCH_COUNT) ||
        (summary.imported_constants_match == 0u) ||
        (summary.imported_dirty_rules_match == 0u) ||
        (summary.imported_exit_semantics_match == 0u) ||
        (summary.p37_manual_real_board_smoke_verified == 0u) ||
        (summary.host_crc_is_jieli_lcd_crc != 0u) ||
        (summary.sdl_visible_parity_claimed != 0u) ||
        (summary.production_resource_parity_claimed != 0u) ||
        (summary.home_observe_enabled != 0u) ||
        (summary.full_pet2d_runtime_enabled != 0u) ||
        (summary.pet2d_runtime_enabled != 0u)) {
        return PET_RESULT_ERROR;
    }

    return PET_RESULT_OK;
}
