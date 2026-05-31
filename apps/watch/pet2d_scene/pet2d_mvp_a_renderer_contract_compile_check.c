#include "pet2d_mvp_a_renderer_contract.h"

PET_STATIC_ASSERT(pet2d_mvp_a_render_plan_cmd_bound,
                  PET2D_MVP_A_RENDER_PLAN_MAX_CMDS == 4u);
PET_STATIC_ASSERT(pet2d_mvp_a_rect_small,
                  sizeof(pet2d_mvp_a_rect_t) == 8u);
PET_STATIC_ASSERT(pet2d_mvp_a_render_cmd_has_type,
                  sizeof(((pet2d_mvp_a_render_cmd_t *)0)->type) >=
                      sizeof(pet_u8_t));

pet_result_t pet2d_mvp_a_renderer_contract_compile_check_self_test(void)
{
    return pet2d_mvp_a_renderer_contract_self_test();
}

pet_result_t pet2d_mvp_a_renderer_contract_compile_check_plan(void)
{
    pet2d_mvp_a_rect_t stage = {0, 0, 96u, 64u};
    pet2d_mvp_a_rect_t old_pet = {32, 16, 32u, 32u};
    pet2d_mvp_a_rect_t pet = {24, 16, 32u, 32u};
    pet2d_mvp_a_render_plan_t plan;
    pet_result_t ret;

    ret = pet2d_mvp_a_renderer_build_pet_change_plan(&stage,
                                                     &old_pet,
                                                     &pet,
                                                     3u,
                                                     PET_TRUE,
                                                     &plan);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    if ((plan.dirty_rect.w != 40u) || (plan.dirty_rect.h != 32u) ||
        (plan.cmd_count == 0u) || (plan.needs_pet_draw == 0u)) {
        return PET_RESULT_ERROR;
    }

    return PET_RESULT_OK;
}
