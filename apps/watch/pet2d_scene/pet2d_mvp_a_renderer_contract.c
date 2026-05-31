#include "pet2d_mvp_a_renderer_contract.h"

static void pet2d_mvp_a_renderer_zero_plan(pet2d_mvp_a_render_plan_t *plan)
{
    pet_u8_t *bytes = (pet_u8_t *)plan;
    pet_u32_t i;

    for (i = 0u; i < (pet_u32_t)sizeof(*plan); i++) {
        bytes[i] = 0u;
    }
}

static pet_i16_t pet2d_mvp_a_max_i16(pet_i16_t a, pet_i16_t b)
{
    return (a > b) ? a : b;
}

static pet_i16_t pet2d_mvp_a_min_i16(pet_i16_t a, pet_i16_t b)
{
    return (a < b) ? a : b;
}

static pet_result_t pet2d_mvp_a_renderer_add_cmd(
    pet2d_mvp_a_render_plan_t *plan,
    pet2d_mvp_a_render_cmd_type_t type,
    const pet2d_mvp_a_rect_t *dst,
    pet2d_mvp_a_render_pattern_t pattern,
    pet_u8_t pose,
    pet_u8_t flags)
{
    pet2d_mvp_a_render_cmd_t *cmd;

    if ((plan == 0) || (dst == 0)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }
    if (plan->cmd_count >= PET2D_MVP_A_RENDER_PLAN_MAX_CMDS) {
        return PET_RESULT_BUFFER_TOO_SMALL;
    }

    cmd = &plan->cmds[plan->cmd_count];
    cmd->type = type;
    cmd->dst = *dst;
    cmd->pattern = pattern;
    cmd->pose = pose;
    cmd->alpha_mode = 0u;
    cmd->flags = flags;
    cmd->reserved = 0u;
    plan->cmd_count++;

    return PET_RESULT_OK;
}

pet_bool_t pet2d_mvp_a_rect_is_valid(const pet2d_mvp_a_rect_t *rect)
{
    return ((rect != 0) && (rect->w != 0u) && (rect->h != 0u)) ? PET_TRUE :
                                                                      PET_FALSE;
}

pet_bool_t pet2d_mvp_a_rect_equal(const pet2d_mvp_a_rect_t *a,
                                  const pet2d_mvp_a_rect_t *b)
{
    if ((a == 0) || (b == 0)) {
        return PET_FALSE;
    }

    return ((a->x == b->x) && (a->y == b->y) && (a->w == b->w) &&
            (a->h == b->h)) ? PET_TRUE : PET_FALSE;
}

pet_u32_t pet2d_mvp_a_rect_area(const pet2d_mvp_a_rect_t *rect)
{
    if (pet2d_mvp_a_rect_is_valid(rect) == PET_FALSE) {
        return 0u;
    }

    return (pet_u32_t)rect->w * (pet_u32_t)rect->h;
}

pet_result_t pet2d_mvp_a_rect_union(const pet2d_mvp_a_rect_t *a,
                                    const pet2d_mvp_a_rect_t *b,
                                    pet2d_mvp_a_rect_t *out_rect)
{
    pet_i16_t left;
    pet_i16_t top;
    pet_i16_t right;
    pet_i16_t bottom;

    if ((a == 0) || (b == 0) || (out_rect == 0)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }
    if ((pet2d_mvp_a_rect_is_valid(a) == PET_FALSE) ||
        (pet2d_mvp_a_rect_is_valid(b) == PET_FALSE)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    left = pet2d_mvp_a_min_i16(a->x, b->x);
    top = pet2d_mvp_a_min_i16(a->y, b->y);
    right = pet2d_mvp_a_max_i16((pet_i16_t)(a->x + (pet_i16_t)a->w),
                                (pet_i16_t)(b->x + (pet_i16_t)b->w));
    bottom = pet2d_mvp_a_max_i16((pet_i16_t)(a->y + (pet_i16_t)a->h),
                                 (pet_i16_t)(b->y + (pet_i16_t)b->h));

    out_rect->x = left;
    out_rect->y = top;
    out_rect->w = (pet_u16_t)(right - left);
    out_rect->h = (pet_u16_t)(bottom - top);

    return PET_RESULT_OK;
}

pet_result_t pet2d_mvp_a_rect_clip_to_stage(const pet2d_mvp_a_rect_t *stage,
                                            const pet2d_mvp_a_rect_t *rect,
                                            pet2d_mvp_a_rect_t *out_rect)
{
    pet_i16_t left;
    pet_i16_t top;
    pet_i16_t right;
    pet_i16_t bottom;

    if ((stage == 0) || (rect == 0) || (out_rect == 0)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }
    if ((pet2d_mvp_a_rect_is_valid(stage) == PET_FALSE) ||
        (pet2d_mvp_a_rect_is_valid(rect) == PET_FALSE)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    left = pet2d_mvp_a_max_i16(stage->x, rect->x);
    top = pet2d_mvp_a_max_i16(stage->y, rect->y);
    right = pet2d_mvp_a_min_i16((pet_i16_t)(stage->x + (pet_i16_t)stage->w),
                                (pet_i16_t)(rect->x + (pet_i16_t)rect->w));
    bottom = pet2d_mvp_a_min_i16((pet_i16_t)(stage->y + (pet_i16_t)stage->h),
                                 (pet_i16_t)(rect->y + (pet_i16_t)rect->h));

    if ((right <= left) || (bottom <= top)) {
        out_rect->x = 0;
        out_rect->y = 0;
        out_rect->w = 0u;
        out_rect->h = 0u;
        return PET_RESULT_NOT_FOUND;
    }

    out_rect->x = left;
    out_rect->y = top;
    out_rect->w = (pet_u16_t)(right - left);
    out_rect->h = (pet_u16_t)(bottom - top);

    return PET_RESULT_OK;
}

pet_result_t pet2d_mvp_a_rect_clamp_pet_to_stage(
    const pet2d_mvp_a_rect_t *stage,
    pet2d_mvp_a_rect_t *pet_rect)
{
    pet_i16_t max_x;
    pet_i16_t max_y;

    if ((stage == 0) || (pet_rect == 0)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }
    if ((pet2d_mvp_a_rect_is_valid(stage) == PET_FALSE) ||
        (pet2d_mvp_a_rect_is_valid(pet_rect) == PET_FALSE) ||
        (pet_rect->w > stage->w) || (pet_rect->h > stage->h)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    max_x = (pet_i16_t)(stage->x + (pet_i16_t)(stage->w - pet_rect->w));
    max_y = (pet_i16_t)(stage->y + (pet_i16_t)(stage->h - pet_rect->h));

    if (pet_rect->x < stage->x) {
        pet_rect->x = stage->x;
    }
    if (pet_rect->y < stage->y) {
        pet_rect->y = stage->y;
    }
    if (pet_rect->x > max_x) {
        pet_rect->x = max_x;
    }
    if (pet_rect->y > max_y) {
        pet_rect->y = max_y;
    }

    return PET_RESULT_OK;
}

pet2d_mvp_a_render_pattern_t pet2d_mvp_a_renderer_pattern_from_pose(
    pet_u8_t pose)
{
    switch (pose) {
    case 1u:
        return PET2D_MVP_A_RENDER_PATTERN_HAPPY;
    case 2u:
        return PET2D_MVP_A_RENDER_PATTERN_BLINK;
    case 3u:
        return PET2D_MVP_A_RENDER_PATTERN_STEP;
    case 0u:
    default:
        return PET2D_MVP_A_RENDER_PATTERN_IDLE;
    }
}

pet_result_t pet2d_mvp_a_renderer_build_initial_plan(
    const pet2d_mvp_a_rect_t *stage,
    const pet2d_mvp_a_rect_t *pet_rect,
    pet_u8_t pose,
    pet2d_mvp_a_render_plan_t *out_plan)
{
    pet_result_t ret;

    if ((stage == 0) || (pet_rect == 0) || (out_plan == 0)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }
    if ((pet2d_mvp_a_rect_is_valid(stage) == PET_FALSE) ||
        (pet2d_mvp_a_rect_is_valid(pet_rect) == PET_FALSE)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    pet2d_mvp_a_renderer_zero_plan(out_plan);
    out_plan->stage_rect = *stage;
    out_plan->prev_pet_rect = *pet_rect;
    out_plan->pet_rect = *pet_rect;
    out_plan->dirty_rect = *stage;
    out_plan->needs_stage_restore = 1u;
    out_plan->needs_pet_draw = 1u;

    ret = pet2d_mvp_a_renderer_add_cmd(out_plan,
                                       PET2D_MVP_A_RENDER_CMD_STAGE_PATCH,
                                       stage,
                                       PET2D_MVP_A_RENDER_PATTERN_STAGE,
                                       pose,
                                       PET2D_MVP_A_RENDER_FLAG_STAGE_RESTORE);
    if (ret != PET_RESULT_OK) {
        return ret;
    }

    return pet2d_mvp_a_renderer_add_cmd(
        out_plan,
        PET2D_MVP_A_RENDER_CMD_PET_PLACEHOLDER,
        pet_rect,
        pet2d_mvp_a_renderer_pattern_from_pose(pose),
        pose,
        PET2D_MVP_A_RENDER_FLAG_PET_DRAW);
}

pet_result_t pet2d_mvp_a_renderer_build_pet_change_plan(
    const pet2d_mvp_a_rect_t *stage,
    const pet2d_mvp_a_rect_t *prev_pet_rect,
    const pet2d_mvp_a_rect_t *pet_rect,
    pet_u8_t pose,
    pet_bool_t restore_stage,
    pet2d_mvp_a_render_plan_t *out_plan)
{
    pet_result_t ret;

    if ((stage == 0) || (prev_pet_rect == 0) || (pet_rect == 0) ||
        (out_plan == 0)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }
    if ((pet2d_mvp_a_rect_is_valid(stage) == PET_FALSE) ||
        (pet2d_mvp_a_rect_is_valid(prev_pet_rect) == PET_FALSE) ||
        (pet2d_mvp_a_rect_is_valid(pet_rect) == PET_FALSE)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    pet2d_mvp_a_renderer_zero_plan(out_plan);
    out_plan->stage_rect = *stage;
    out_plan->prev_pet_rect = *prev_pet_rect;
    out_plan->pet_rect = *pet_rect;
    out_plan->needs_stage_restore = (restore_stage != PET_FALSE) ? 1u : 0u;
    out_plan->needs_pet_draw = 1u;

    if (pet2d_mvp_a_rect_equal(prev_pet_rect, pet_rect) == PET_TRUE) {
        out_plan->dirty_rect = *pet_rect;
    } else {
        ret = pet2d_mvp_a_rect_union(prev_pet_rect, pet_rect,
                                     &out_plan->dirty_rect);
        if (ret != PET_RESULT_OK) {
            return ret;
        }
    }

    ret = pet2d_mvp_a_rect_clip_to_stage(stage,
                                         &out_plan->dirty_rect,
                                         &out_plan->dirty_rect);
    if (ret != PET_RESULT_OK) {
        return ret;
    }

    if (restore_stage != PET_FALSE) {
        ret = pet2d_mvp_a_renderer_add_cmd(
            out_plan,
            PET2D_MVP_A_RENDER_CMD_CLEAR_DIRTY,
            &out_plan->dirty_rect,
            PET2D_MVP_A_RENDER_PATTERN_STAGE,
            pose,
            PET2D_MVP_A_RENDER_FLAG_STAGE_RESTORE);
        if (ret != PET_RESULT_OK) {
            return ret;
        }
    }

    return pet2d_mvp_a_renderer_add_cmd(
        out_plan,
        PET2D_MVP_A_RENDER_CMD_PET_PLACEHOLDER,
        pet_rect,
        pet2d_mvp_a_renderer_pattern_from_pose(pose),
        pose,
        PET2D_MVP_A_RENDER_FLAG_PET_DRAW);
}

pet_result_t pet2d_mvp_a_renderer_build_idle_plan(
    pet2d_mvp_a_render_plan_t *out_plan)
{
    if (out_plan == 0) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    pet2d_mvp_a_renderer_zero_plan(out_plan);
    out_plan->skipped_flush = 1u;
    return PET_RESULT_OK;
}

pet_result_t pet2d_mvp_a_renderer_contract_self_test(void)
{
    pet2d_mvp_a_rect_t stage = {10, 20, 96u, 64u};
    pet2d_mvp_a_rect_t pet = {42, 36, 32u, 32u};
    pet2d_mvp_a_rect_t old_pet = {50, 36, 32u, 32u};
    pet2d_mvp_a_rect_t clipped;
    pet2d_mvp_a_rect_t union_rect;
    pet2d_mvp_a_render_plan_t plan;
    pet2d_mvp_a_rect_t clamp_pet = {-10, 100, 32u, 32u};
    pet_result_t ret;

    if (pet2d_mvp_a_rect_area(&pet) != 1024u) {
        return PET_RESULT_ERROR;
    }
    ret = pet2d_mvp_a_rect_union(&old_pet, &pet, &union_rect);
    if ((ret != PET_RESULT_OK) || (union_rect.w != 40u) ||
        (union_rect.h != 32u)) {
        return PET_RESULT_ERROR;
    }

    ret = pet2d_mvp_a_rect_clip_to_stage(&stage, &union_rect, &clipped);
    if ((ret != PET_RESULT_OK) ||
        (pet2d_mvp_a_rect_equal(&clipped, &union_rect) == PET_FALSE)) {
        return PET_RESULT_ERROR;
    }

    ret = pet2d_mvp_a_rect_clamp_pet_to_stage(&stage, &clamp_pet);
    if ((ret != PET_RESULT_OK) || (clamp_pet.x != stage.x) ||
        (clamp_pet.y != (pet_i16_t)(stage.y + 32))) {
        return PET_RESULT_ERROR;
    }

    ret = pet2d_mvp_a_renderer_build_initial_plan(&stage, &pet, 0u, &plan);
    if ((ret != PET_RESULT_OK) || (plan.cmd_count < 1u) ||
        (plan.dirty_rect.w != 96u) || (plan.dirty_rect.h != 64u) ||
        (plan.cmds[0].type != PET2D_MVP_A_RENDER_CMD_STAGE_PATCH)) {
        return PET_RESULT_ERROR;
    }

    ret = pet2d_mvp_a_renderer_build_pet_change_plan(&stage,
                                                     &old_pet,
                                                     &pet,
                                                     3u,
                                                     PET_TRUE,
                                                     &plan);
    if ((ret != PET_RESULT_OK) || (plan.dirty_rect.w != 40u) ||
        (plan.dirty_rect.h != 32u) || (plan.needs_stage_restore == 0u) ||
        (plan.needs_pet_draw == 0u) ||
        (plan.cmds[plan.cmd_count - 1u].pattern !=
         PET2D_MVP_A_RENDER_PATTERN_STEP)) {
        return PET_RESULT_ERROR;
    }

    ret = pet2d_mvp_a_renderer_build_pet_change_plan(&stage,
                                                     &pet,
                                                     &pet,
                                                     1u,
                                                     PET_FALSE,
                                                     &plan);
    if ((ret != PET_RESULT_OK) || (plan.dirty_rect.w != 32u) ||
        (plan.dirty_rect.h != 32u) ||
        (plan.cmds[plan.cmd_count - 1u].pattern !=
         PET2D_MVP_A_RENDER_PATTERN_HAPPY)) {
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
