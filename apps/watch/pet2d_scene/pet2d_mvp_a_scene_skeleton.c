#include "pet2d_mvp_a_scene_skeleton.h"

#include "pet_platform_jieli_internal.h"

extern int printf(const char *format, ...);

void mvp_a_lvgl_shell_release_display_owner(void);
void mvp_a_lvgl_shell_request_refresh(void);

typedef struct {
    pet_i16_t stage_x;
    pet_i16_t stage_y;
    pet_u32_t last_render_ms;
    pet_u8_t full_patch_pending;
    pet_u8_t visual_dirty_pending;
    pet_u8_t pose_cycle;
} pet2d_mvp_a_scene_context_t;

static pet2d_mvp_a_scene_context_t g_mvp_a_scene_ctx;
static pet2d_mvp_a_scene_model_t g_mvp_a_scene_model;
static pet2d_mvp_a_scene_draw_cmd_t g_mvp_a_scene_draw_cmd;
static pet2d_mvp_a_render_plan_t g_mvp_a_scene_render_plan;
static pet2d_mvp_a_scene_stats_t g_mvp_a_scene_stats;

#if PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC
static pet_u16_t g_mvp_a_scene_pixels[PET2D_MVP_A_SCENE_STAGE_W *
                                      PET2D_MVP_A_SCENE_STAGE_H];
#endif

static pet_u32_t pet2d_mvp_a_scene_now_ms(void)
{
    const pet_platform_t *platform = pet_platform_jieli_get();

    if ((platform == 0) || (platform->millis == 0)) {
        return 0u;
    }
    return platform->millis(platform->ctx);
}

static pet_u32_t pet2d_mvp_a_scene_resolve_now(pet_u32_t now_ms)
{
    if (now_ms != 0u) {
        return now_ms;
    }
    return pet2d_mvp_a_scene_now_ms();
}

static pet_u32_t pet2d_mvp_a_scene_elapsed_ms(pet_u32_t end_ms, pet_u32_t start_ms)
{
    return (pet_u32_t)(end_ms - start_ms);
}

static pet_bool_t pet2d_mvp_a_scene_state_is_active(pet2d_mvp_a_scene_state_t state)
{
    return (state == PET2D_MVP_A_SCENE_STATE_ENTER) ||
           (state == PET2D_MVP_A_SCENE_STATE_IDLE) ||
           (state == PET2D_MVP_A_SCENE_STATE_MOVE_LEFT) ||
           (state == PET2D_MVP_A_SCENE_STATE_MOVE_RIGHT) ||
           (state == PET2D_MVP_A_SCENE_STATE_ACTION) ||
           (state == PET2D_MVP_A_SCENE_STATE_EXITING);
}

static pet_bool_t pet2d_mvp_a_scene_state_is_action(pet2d_mvp_a_scene_state_t state)
{
    return (state == PET2D_MVP_A_SCENE_STATE_MOVE_LEFT) ||
           (state == PET2D_MVP_A_SCENE_STATE_MOVE_RIGHT) ||
           (state == PET2D_MVP_A_SCENE_STATE_ACTION);
}

static const char *pet2d_mvp_a_scene_state_name(pet2d_mvp_a_scene_state_t state)
{
    switch (state) {
    case PET2D_MVP_A_SCENE_STATE_ENTER:
        return "ENTER";
    case PET2D_MVP_A_SCENE_STATE_IDLE:
        return "IDLE";
    case PET2D_MVP_A_SCENE_STATE_MOVE_LEFT:
        return "MOVE_LEFT";
    case PET2D_MVP_A_SCENE_STATE_MOVE_RIGHT:
        return "MOVE_RIGHT";
    case PET2D_MVP_A_SCENE_STATE_ACTION:
        return "ACTION";
    case PET2D_MVP_A_SCENE_STATE_EXITING:
        return "EXITING";
    case PET2D_MVP_A_SCENE_STATE_DONE:
        return "DONE";
    case PET2D_MVP_A_SCENE_STATE_ERROR:
        return "ERROR";
    case PET2D_MVP_A_SCENE_STATE_NONE:
    default:
        return "NONE";
    }
}

static const char *pet2d_mvp_a_scene_pose_name(pet2d_mvp_a_scene_pose_t pose)
{
    switch (pose) {
    case PET2D_MVP_A_SCENE_POSE_HAPPY:
        return "HAPPY";
    case PET2D_MVP_A_SCENE_POSE_BLINK:
        return "BLINK";
    case PET2D_MVP_A_SCENE_POSE_STEP:
        return "STEP";
    case PET2D_MVP_A_SCENE_POSE_IDLE:
    default:
        return "IDLE";
    }
}

static void pet2d_mvp_a_scene_set_state(pet2d_mvp_a_scene_state_t state)
{
    g_mvp_a_scene_model.state = state;
    g_mvp_a_scene_stats.last_state = (pet_u8_t)state;
}

static void pet2d_mvp_a_scene_update_max(pet_u32_t value, pet_u32_t *max_value)
{
    if (value > *max_value) {
        *max_value = value;
    }
}

static pet_u16_t pet2d_mvp_a_scene_background_pixel(pet_i16_t x, pet_i16_t y)
{
    pet_u16_t grid = (pet_u16_t)((((pet_u16_t)x >> 3) ^ ((pet_u16_t)y >> 3)) & 0x01u);

    return grid ? 0x2945u : 0x18c3u;
}

static pet_u16_t pet2d_mvp_a_scene_pet_pixel(pet_u16_t x,
                                             pet_u16_t y,
                                             pet2d_mvp_a_scene_pose_t pose,
                                             pet_u32_t frame)
{
    pet_u8_t border = (pet_u8_t)((x < 2u) || (y < 2u) ||
                                 (x >= (PET2D_MVP_A_SCENE_SPRITE_SIZE - 2u)) ||
                                 (y >= (PET2D_MVP_A_SCENE_SPRITE_SIZE - 2u)));

    if (border != 0u) {
        return 0xffffu;
    }
    if ((pose == PET2D_MVP_A_SCENE_POSE_BLINK) &&
        (y >= 12u) && (y < 16u) && (x >= 8u) && (x < 24u)) {
        return 0x0000u;
    }
    if (pose == PET2D_MVP_A_SCENE_POSE_HAPPY) {
        return ((((x >> 2) + (y >> 2) + (pet_u16_t)frame) & 0x01u) != 0u) ?
               0xfbe0u : 0x07ffu;
    }
    if (pose == PET2D_MVP_A_SCENE_POSE_STEP) {
        return ((((x >> 1) + (pet_u16_t)frame) & 0x01u) != 0u) ?
               0xffe0u : 0x001fu;
    }
    return ((((x >> 3) + (y >> 3) + (pet_u16_t)frame) & 0x01u) != 0u) ?
           0xf81fu : 0x07e0u;
}

static void pet2d_mvp_a_scene_update_draw_cmd(void)
{
    g_mvp_a_scene_draw_cmd.type = PET2D_MVP_A_RENDER_CMD_PET_PLACEHOLDER;
    g_mvp_a_scene_draw_cmd.dst.x = g_mvp_a_scene_model.pet_x;
    g_mvp_a_scene_draw_cmd.dst.y = g_mvp_a_scene_model.pet_y;
    g_mvp_a_scene_draw_cmd.dst.w = PET2D_MVP_A_SCENE_SPRITE_SIZE;
    g_mvp_a_scene_draw_cmd.dst.h = PET2D_MVP_A_SCENE_SPRITE_SIZE;
    g_mvp_a_scene_draw_cmd.pattern =
        pet2d_mvp_a_renderer_pattern_from_pose((pet_u8_t)g_mvp_a_scene_model.pose);
    g_mvp_a_scene_draw_cmd.pose = (pet_u8_t)g_mvp_a_scene_model.pose;
    g_mvp_a_scene_draw_cmd.alpha_mode = 0u;
    g_mvp_a_scene_draw_cmd.flags =
        PET2D_MVP_A_RENDER_FLAG_PET_DRAW;
    g_mvp_a_scene_draw_cmd.reserved = 0u;
}

static pet2d_mvp_a_rect_t pet2d_mvp_a_scene_stage_rect(void)
{
    pet2d_mvp_a_rect_t rect;

    rect.x = g_mvp_a_scene_ctx.stage_x;
    rect.y = g_mvp_a_scene_ctx.stage_y;
    rect.w = PET2D_MVP_A_SCENE_STAGE_W;
    rect.h = PET2D_MVP_A_SCENE_STAGE_H;
    return rect;
}

static pet2d_mvp_a_rect_t pet2d_mvp_a_scene_pet_rect(pet_i16_t x, pet_i16_t y)
{
    pet2d_mvp_a_rect_t rect;

    rect.x = x;
    rect.y = y;
    rect.w = PET2D_MVP_A_SCENE_SPRITE_SIZE;
    rect.h = PET2D_MVP_A_SCENE_SPRITE_SIZE;
    return rect;
}

static pet_result_t pet2d_mvp_a_scene_build_render_plan(void)
{
    pet2d_mvp_a_rect_t stage = pet2d_mvp_a_scene_stage_rect();
    pet2d_mvp_a_rect_t pet = pet2d_mvp_a_scene_pet_rect(
        g_mvp_a_scene_model.pet_x,
        g_mvp_a_scene_model.pet_y);
    pet2d_mvp_a_rect_t prev = pet2d_mvp_a_scene_pet_rect(
        g_mvp_a_scene_model.prev_x,
        g_mvp_a_scene_model.prev_y);
    pet_result_t ret;
    pet_u8_t i;

    if (g_mvp_a_scene_ctx.full_patch_pending != 0u) {
        ret = pet2d_mvp_a_renderer_build_initial_plan(
            &stage, &pet, (pet_u8_t)g_mvp_a_scene_model.pose,
            &g_mvp_a_scene_render_plan);
    } else if (g_mvp_a_scene_ctx.visual_dirty_pending != 0u) {
        ret = pet2d_mvp_a_renderer_build_pet_change_plan(
            &stage, &prev, &pet, (pet_u8_t)g_mvp_a_scene_model.pose,
            PET_TRUE, &g_mvp_a_scene_render_plan);
    } else {
        ret = pet2d_mvp_a_renderer_build_idle_plan(&g_mvp_a_scene_render_plan);
    }
    if (ret != PET_RESULT_OK) {
        return ret;
    }

    pet2d_mvp_a_scene_update_draw_cmd();
    for (i = 0u; i < g_mvp_a_scene_render_plan.cmd_count; i++) {
        if (g_mvp_a_scene_render_plan.cmds[i].type ==
            PET2D_MVP_A_RENDER_CMD_PET_PLACEHOLDER) {
            g_mvp_a_scene_draw_cmd = g_mvp_a_scene_render_plan.cmds[i];
        }
    }
    return PET_RESULT_OK;
}

static void pet2d_mvp_a_scene_init_context(pet_u32_t now_ms)
{
    pet_i16_t start_x;
    pet_i16_t start_y;

    g_mvp_a_scene_ctx.stage_x =
        (pet_i16_t)(PET_JIELI_DISPLAY_SAFE_X +
                    ((PET_JIELI_DISPLAY_SAFE_W - PET2D_MVP_A_SCENE_STAGE_W) / 2u));
    g_mvp_a_scene_ctx.stage_y =
        (pet_i16_t)(PET_JIELI_DISPLAY_SAFE_Y +
                    ((PET_JIELI_DISPLAY_SAFE_H - PET2D_MVP_A_SCENE_STAGE_H) / 2u));
    start_x =
        (pet_i16_t)(g_mvp_a_scene_ctx.stage_x +
                    ((PET2D_MVP_A_SCENE_STAGE_W - PET2D_MVP_A_SCENE_SPRITE_SIZE) / 2u));
    start_y =
        (pet_i16_t)(g_mvp_a_scene_ctx.stage_y +
                    ((PET2D_MVP_A_SCENE_STAGE_H - PET2D_MVP_A_SCENE_SPRITE_SIZE) / 2u));

    g_mvp_a_scene_model.state = PET2D_MVP_A_SCENE_STATE_ENTER;
    g_mvp_a_scene_model.pose = PET2D_MVP_A_SCENE_POSE_IDLE;
    g_mvp_a_scene_model.pet_x = start_x;
    g_mvp_a_scene_model.pet_y = start_y;
    g_mvp_a_scene_model.prev_x = start_x;
    g_mvp_a_scene_model.prev_y = start_y;
    g_mvp_a_scene_model.frame_index = 0u;
    g_mvp_a_scene_model.action_started_ms = 0u;
    g_mvp_a_scene_model.action_duration_ms = 0u;
    g_mvp_a_scene_model.enter_ms = now_ms;
    g_mvp_a_scene_model.timeout_ms = now_ms + PET2D_MVP_A_SCENE_TIMEOUT_MS;
    g_mvp_a_scene_model.exit_reason = PET2D_MVP_A_SCENE_EXIT_NONE;

    g_mvp_a_scene_ctx.last_render_ms = now_ms;
    g_mvp_a_scene_ctx.full_patch_pending = 1u;
    g_mvp_a_scene_ctx.visual_dirty_pending = 1u;
    g_mvp_a_scene_ctx.pose_cycle = 0u;
    pet2d_mvp_a_scene_update_draw_cmd();
}

static void pet2d_mvp_a_scene_clamp_pet(void)
{
    pet_i16_t min_x = g_mvp_a_scene_ctx.stage_x;
    pet_i16_t max_x = (pet_i16_t)(g_mvp_a_scene_ctx.stage_x +
                                  PET2D_MVP_A_SCENE_STAGE_W -
                                  PET2D_MVP_A_SCENE_SPRITE_SIZE);

    if (g_mvp_a_scene_model.pet_x < min_x) {
        g_mvp_a_scene_model.pet_x = min_x;
    }
    if (g_mvp_a_scene_model.pet_x > max_x) {
        g_mvp_a_scene_model.pet_x = max_x;
    }
}

static void pet2d_mvp_a_scene_begin_action(pet2d_mvp_a_scene_state_t state,
                                           pet2d_mvp_a_scene_pose_t pose,
                                           pet_u32_t now_ms,
                                           pet_u32_t duration_ms)
{
    pet2d_mvp_a_scene_set_state(state);
    g_mvp_a_scene_model.pose = pose;
    g_mvp_a_scene_model.action_started_ms = now_ms;
    g_mvp_a_scene_model.action_duration_ms = duration_ms;
    g_mvp_a_scene_ctx.visual_dirty_pending = 1u;
    pet2d_mvp_a_scene_update_draw_cmd();
}

#if PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC
static void pet2d_mvp_a_scene_fill_dirty_surface(pet_u16_t *pixels,
                                                 pet_i16_t dirty_x,
                                                 pet_i16_t dirty_y,
                                                 pet_u16_t dirty_w,
                                                 pet_u16_t dirty_h)
{
    pet_u16_t x;
    pet_u16_t y;

    for (y = 0u; y < dirty_h; y++) {
        for (x = 0u; x < dirty_w; x++) {
            pet_i16_t abs_x = (pet_i16_t)(dirty_x + (pet_i16_t)x);
            pet_i16_t abs_y = (pet_i16_t)(dirty_y + (pet_i16_t)y);
            pet_u16_t pixel = pet2d_mvp_a_scene_background_pixel(abs_x, abs_y);

            if ((abs_x >= g_mvp_a_scene_model.pet_x) &&
                (abs_y >= g_mvp_a_scene_model.pet_y) &&
                (abs_x < (pet_i16_t)(g_mvp_a_scene_model.pet_x +
                                      PET2D_MVP_A_SCENE_SPRITE_SIZE)) &&
                (abs_y < (pet_i16_t)(g_mvp_a_scene_model.pet_y +
                                      PET2D_MVP_A_SCENE_SPRITE_SIZE))) {
                pixel = pet2d_mvp_a_scene_pet_pixel(
                    (pet_u16_t)(abs_x - g_mvp_a_scene_model.pet_x),
                    (pet_u16_t)(abs_y - g_mvp_a_scene_model.pet_y),
                    g_mvp_a_scene_model.pose,
                    g_mvp_a_scene_stats.frame_count);
            }
            pixels[((pet_u32_t)y * (pet_u32_t)dirty_w) + (pet_u32_t)x] = pixel;
        }
    }
}
#endif

static pet_result_t pet2d_mvp_a_scene_render_once(void)
{
    pet2d_mvp_a_rect_t dirty;
    pet_u32_t frame_start_ms;
    pet_u32_t logic_start_ms;
    pet_u32_t logic_end_ms;
    pet_u32_t render_start_ms;
    pet_u32_t render_end_ms;
    pet_u32_t flush_start_ms;
    pet_u32_t flush_end_ms;
    pet_u32_t frame_end_ms;
    pet_result_t ret;

    frame_start_ms = pet2d_mvp_a_scene_now_ms();
    logic_start_ms = frame_start_ms;
    ret = pet2d_mvp_a_scene_build_render_plan();
    logic_end_ms = pet2d_mvp_a_scene_now_ms();
    if (ret != PET_RESULT_OK) {
        g_mvp_a_scene_stats.render_count++;
        g_mvp_a_scene_stats.last_result = (pet_u8_t)ret;
        return ret;
    }

    if ((g_mvp_a_scene_render_plan.skipped_flush != 0u) ||
        (g_mvp_a_scene_render_plan.cmd_count == 0u) ||
        (pet2d_mvp_a_rect_is_valid(&g_mvp_a_scene_render_plan.dirty_rect) == PET_FALSE)) {
        frame_end_ms = pet2d_mvp_a_scene_now_ms();
        g_mvp_a_scene_stats.frame_count++;
        g_mvp_a_scene_stats.render_count++;
        g_mvp_a_scene_model.frame_index = g_mvp_a_scene_stats.frame_count;
        g_mvp_a_scene_stats.flush_skipped_count++;
        g_mvp_a_scene_stats.last_dirty_x = 0u;
        g_mvp_a_scene_stats.last_dirty_y = 0u;
        g_mvp_a_scene_stats.last_dirty_w = 0u;
        g_mvp_a_scene_stats.last_dirty_h = 0u;
        g_mvp_a_scene_stats.last_pet_x = (pet_u16_t)g_mvp_a_scene_model.pet_x;
        g_mvp_a_scene_stats.last_pet_y = (pet_u16_t)g_mvp_a_scene_model.pet_y;
        g_mvp_a_scene_stats.last_pose = (pet_u8_t)g_mvp_a_scene_model.pose;
        g_mvp_a_scene_stats.logic_total_ms +=
            pet2d_mvp_a_scene_elapsed_ms(logic_end_ms, logic_start_ms);
        g_mvp_a_scene_stats.frame_total_ms +=
            pet2d_mvp_a_scene_elapsed_ms(frame_end_ms, frame_start_ms);
        pet2d_mvp_a_scene_update_max(
            pet2d_mvp_a_scene_elapsed_ms(logic_end_ms, logic_start_ms),
            &g_mvp_a_scene_stats.logic_max_ms);
        pet2d_mvp_a_scene_update_max(
            pet2d_mvp_a_scene_elapsed_ms(frame_end_ms, frame_start_ms),
            &g_mvp_a_scene_stats.frame_max_ms);
        g_mvp_a_scene_stats.last_result = PET_RESULT_OK;
        printf("[PET2D_MVP_A_RENDER] frame=%lu cmd_count=0 dirty=0x0 stage_restore=0 pet_draw=0 skipped=%lu\n",
               (unsigned long)g_mvp_a_scene_stats.frame_count,
               (unsigned long)g_mvp_a_scene_stats.flush_skipped_count);
        return PET_RESULT_OK;
    }

    dirty = g_mvp_a_scene_render_plan.dirty_rect;
    if ((dirty.w > PET2D_MVP_A_SCENE_STAGE_W) ||
        (dirty.h > PET2D_MVP_A_SCENE_STAGE_H)) {
        ret = PET_RESULT_UNSUPPORTED;
        g_mvp_a_scene_stats.last_result = (pet_u8_t)ret;
        return ret;
    }

    render_start_ms = pet2d_mvp_a_scene_now_ms();
#if PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC
    pet2d_mvp_a_scene_fill_dirty_surface(g_mvp_a_scene_pixels,
                                         dirty.x, dirty.y, dirty.w, dirty.h);
#else
    (void)pet2d_mvp_a_scene_background_pixel(dirty.x, dirty.y);
#endif
    render_end_ms = pet2d_mvp_a_scene_now_ms();

    flush_start_ms = pet2d_mvp_a_scene_now_ms();
#if PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC
    ret = pet_display_jieli_real_flush_poc_rect(dirty.x, dirty.y, dirty.w, dirty.h,
                                                g_mvp_a_scene_pixels, dirty.w);
#else
    ret = PET_RESULT_UNSUPPORTED;
#endif
    flush_end_ms = pet2d_mvp_a_scene_now_ms();
    frame_end_ms = pet2d_mvp_a_scene_now_ms();

    g_mvp_a_scene_stats.frame_count++;
    g_mvp_a_scene_stats.render_count++;
    g_mvp_a_scene_model.frame_index = g_mvp_a_scene_stats.frame_count;
    g_mvp_a_scene_stats.last_dirty_x = (pet_u16_t)dirty.x;
    g_mvp_a_scene_stats.last_dirty_y = (pet_u16_t)dirty.y;
    g_mvp_a_scene_stats.last_dirty_w = dirty.w;
    g_mvp_a_scene_stats.last_dirty_h = dirty.h;
    g_mvp_a_scene_stats.last_pet_x = (pet_u16_t)g_mvp_a_scene_model.pet_x;
    g_mvp_a_scene_stats.last_pet_y = (pet_u16_t)g_mvp_a_scene_model.pet_y;
    g_mvp_a_scene_stats.last_pose = (pet_u8_t)g_mvp_a_scene_model.pose;

    g_mvp_a_scene_stats.logic_total_ms +=
        pet2d_mvp_a_scene_elapsed_ms(logic_end_ms, logic_start_ms);
    g_mvp_a_scene_stats.render_total_ms +=
        pet2d_mvp_a_scene_elapsed_ms(render_end_ms, render_start_ms);
    g_mvp_a_scene_stats.flush_total_ms +=
        pet2d_mvp_a_scene_elapsed_ms(flush_end_ms, flush_start_ms);
    g_mvp_a_scene_stats.frame_total_ms +=
        pet2d_mvp_a_scene_elapsed_ms(frame_end_ms, frame_start_ms);
    pet2d_mvp_a_scene_update_max(
        pet2d_mvp_a_scene_elapsed_ms(logic_end_ms, logic_start_ms),
        &g_mvp_a_scene_stats.logic_max_ms);
    pet2d_mvp_a_scene_update_max(
        pet2d_mvp_a_scene_elapsed_ms(render_end_ms, render_start_ms),
        &g_mvp_a_scene_stats.render_max_ms);
    pet2d_mvp_a_scene_update_max(
        pet2d_mvp_a_scene_elapsed_ms(flush_end_ms, flush_start_ms),
        &g_mvp_a_scene_stats.flush_max_ms);
    pet2d_mvp_a_scene_update_max(
        pet2d_mvp_a_scene_elapsed_ms(frame_end_ms, frame_start_ms),
        &g_mvp_a_scene_stats.frame_max_ms);

    if (ret == PET_RESULT_OK) {
        g_mvp_a_scene_stats.flush_success_count++;
    } else if (ret == PET_RESULT_UNSUPPORTED) {
        g_mvp_a_scene_stats.flush_skipped_count++;
        ret = PET_RESULT_OK;
    } else {
        g_mvp_a_scene_stats.flush_fail_count++;
    }
    g_mvp_a_scene_stats.last_result = (pet_u8_t)ret;
    g_mvp_a_scene_model.prev_x = g_mvp_a_scene_model.pet_x;
    g_mvp_a_scene_model.prev_y = g_mvp_a_scene_model.pet_y;
    g_mvp_a_scene_ctx.full_patch_pending = 0u;
    g_mvp_a_scene_ctx.visual_dirty_pending = 0u;

    printf("[PET2D_MVP_A_SCENE] frame=%lu state=%s pose=%s pet=%u,%u dirty=%ux%u ret=%d owner=%d\n",
           (unsigned long)g_mvp_a_scene_stats.frame_count,
           pet2d_mvp_a_scene_state_name(g_mvp_a_scene_model.state),
           pet2d_mvp_a_scene_pose_name(g_mvp_a_scene_model.pose),
           g_mvp_a_scene_stats.last_pet_x,
           g_mvp_a_scene_stats.last_pet_y,
           g_mvp_a_scene_stats.last_dirty_w,
           g_mvp_a_scene_stats.last_dirty_h,
           ret,
           pet_display_jieli_get_owner());
    printf("[PET2D_MVP_A_RENDER] frame=%lu cmd_count=%u dirty=%ux%u stage_restore=%u pet_draw=%u skipped=%lu\n",
           (unsigned long)g_mvp_a_scene_stats.frame_count,
           g_mvp_a_scene_render_plan.cmd_count,
           g_mvp_a_scene_render_plan.dirty_rect.w,
           g_mvp_a_scene_render_plan.dirty_rect.h,
           g_mvp_a_scene_render_plan.needs_stage_restore,
           g_mvp_a_scene_render_plan.needs_pet_draw,
           (unsigned long)g_mvp_a_scene_stats.flush_skipped_count);
    if (g_mvp_a_scene_render_plan.cmd_count > 0u) {
        const pet2d_mvp_a_render_cmd_t *cmd = &g_mvp_a_scene_render_plan.cmds[0];
        printf("[PET2D_MVP_A_RENDER] cmd[0] type=%d x=%d y=%d w=%u h=%u pattern=%d\n",
               cmd->type,
               cmd->dst.x,
               cmd->dst.y,
               cmd->dst.w,
               cmd->dst.h,
               cmd->pattern);
    }
    return ret;
}

static pet_result_t pet2d_mvp_a_scene_finish_action_if_due(pet_u32_t now_ms,
                                                           pet_u8_t render_on_done)
{
    pet_result_t ret;

    if (pet2d_mvp_a_scene_state_is_action(g_mvp_a_scene_model.state) == PET_FALSE) {
        return PET_RESULT_OK;
    }
    if (pet2d_mvp_a_scene_elapsed_ms(now_ms, g_mvp_a_scene_model.action_started_ms) <
        g_mvp_a_scene_model.action_duration_ms) {
        return PET_RESULT_OK;
    }

    g_mvp_a_scene_model.prev_x = g_mvp_a_scene_model.pet_x;
    g_mvp_a_scene_model.prev_y = g_mvp_a_scene_model.pet_y;
    g_mvp_a_scene_model.pose = PET2D_MVP_A_SCENE_POSE_IDLE;
    g_mvp_a_scene_model.action_duration_ms = 0u;
    pet2d_mvp_a_scene_set_state(PET2D_MVP_A_SCENE_STATE_IDLE);
    g_mvp_a_scene_stats.action_done_count++;
    g_mvp_a_scene_ctx.visual_dirty_pending = 1u;
    pet2d_mvp_a_scene_update_draw_cmd();

    printf("[PET2D_MVP_A_ACTION] state=IDLE action_done=1 pose=%s pet=%d,%d\n",
           pet2d_mvp_a_scene_pose_name(g_mvp_a_scene_model.pose),
           g_mvp_a_scene_model.pet_x,
           g_mvp_a_scene_model.pet_y);
    if (render_on_done == 0u) {
        return PET_RESULT_OK;
    }
    ret = pet2d_mvp_a_scene_render_once();
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    g_mvp_a_scene_ctx.last_render_ms = now_ms;
    return PET_RESULT_OK;
}

static pet_result_t pet2d_mvp_a_scene_exit_with_reason(
    pet2d_mvp_a_scene_exit_reason_t reason,
    pet_u32_t now_ms)
{
    const pet_platform_t *platform = pet_platform_jieli_get();
    pet_display_owner_t owner;
    pet_result_t release_ret = PET_RESULT_OK;
    pet_u32_t duration_ms;

    if ((pet2d_mvp_a_scene_state_is_active(g_mvp_a_scene_model.state) == PET_FALSE) &&
        (g_mvp_a_scene_model.state != PET2D_MVP_A_SCENE_STATE_ERROR)) {
        return PET_RESULT_OK;
    }

    pet2d_mvp_a_scene_set_state(PET2D_MVP_A_SCENE_STATE_EXITING);
    now_ms = pet2d_mvp_a_scene_resolve_now(now_ms);
    owner = pet_display_jieli_get_owner();
    if ((owner == PET_DISPLAY_OWNER_PET2D) &&
        (platform != 0) && (platform->display_release != 0)) {
        release_ret = platform->display_release(platform->ctx, PET_DISPLAY_OWNER_PET2D);
    }

    duration_ms = pet2d_mvp_a_scene_elapsed_ms(now_ms, g_mvp_a_scene_model.enter_ms);
    g_mvp_a_scene_stats.exit_count++;
    g_mvp_a_scene_stats.last_exit_ms = now_ms;
    g_mvp_a_scene_stats.last_duration_ms = duration_ms;
    if (duration_ms > g_mvp_a_scene_stats.max_duration_ms) {
        g_mvp_a_scene_stats.max_duration_ms = duration_ms;
    }
    g_mvp_a_scene_model.exit_reason = reason;
    g_mvp_a_scene_stats.last_exit_reason = (pet_u8_t)reason;
    if ((reason == PET2D_MVP_A_SCENE_EXIT_ERROR) ||
        (release_ret != PET_RESULT_OK)) {
        g_mvp_a_scene_stats.error_count++;
        pet2d_mvp_a_scene_set_state(PET2D_MVP_A_SCENE_STATE_ERROR);
    } else {
        pet2d_mvp_a_scene_set_state(PET2D_MVP_A_SCENE_STATE_DONE);
    }

    mvp_a_lvgl_shell_request_refresh();
    printf("[PET2D_MVP_A_ACTION] exit reason=%d state=%s release_ret=%d duration=%lu "
           "frames=%lu keys=%lu owner=%d\n",
           reason,
           pet2d_mvp_a_scene_state_name(g_mvp_a_scene_model.state),
           release_ret,
           (unsigned long)duration_ms,
           (unsigned long)g_mvp_a_scene_stats.frame_count,
           (unsigned long)g_mvp_a_scene_stats.key_event_count,
           pet_display_jieli_get_owner());
    return release_ret;
}

pet_result_t pet2d_mvp_a_scene_skeleton_enter(void)
{
    const pet_platform_t *platform = pet_platform_jieli_get();
    pet_result_t ret;
    pet_u32_t now_ms;

    if (pet2d_mvp_a_scene_state_is_active(g_mvp_a_scene_model.state) != PET_FALSE) {
        return PET_RESULT_BUSY;
    }
    if ((platform == 0) || (platform->display_acquire == 0)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    now_ms = pet2d_mvp_a_scene_now_ms();
    pet2d_mvp_a_scene_init_context(now_ms);
    g_mvp_a_scene_stats.enter_count++;
    g_mvp_a_scene_stats.last_enter_ms = now_ms;
    g_mvp_a_scene_stats.last_exit_reason = PET2D_MVP_A_SCENE_EXIT_NONE;
    pet2d_mvp_a_scene_set_state(PET2D_MVP_A_SCENE_STATE_ENTER);

    printf("[PET2D_MVP_A_SCENE] enter start owner=%d\n", pet_display_jieli_get_owner());
    mvp_a_lvgl_shell_release_display_owner();
    ret = platform->display_acquire(platform->ctx, PET_DISPLAY_OWNER_PET2D, 0u);
    if (ret != PET_RESULT_OK) {
        g_mvp_a_scene_stats.error_count++;
        pet2d_mvp_a_scene_set_state(PET2D_MVP_A_SCENE_STATE_ERROR);
        (void)pet2d_mvp_a_scene_exit_with_reason(PET2D_MVP_A_SCENE_EXIT_ERROR, now_ms);
        printf("[PET2D_MVP_A_SCENE] acquire failed ret=%d owner=%d\n",
               ret, pet_display_jieli_get_owner());
        return ret;
    }

    pet2d_mvp_a_scene_set_state(PET2D_MVP_A_SCENE_STATE_IDLE);
    ret = pet2d_mvp_a_scene_render_once();
    if (ret != PET_RESULT_OK) {
        (void)pet2d_mvp_a_scene_exit_with_reason(PET2D_MVP_A_SCENE_EXIT_ERROR, now_ms);
        return ret;
    }

    printf("[PET2D_MVP_A_SCENE] enter ok owner=%d timeout=%lu patch=%ux%u\n",
           pet_display_jieli_get_owner(),
           (unsigned long)PET2D_MVP_A_SCENE_TIMEOUT_MS,
           PET2D_MVP_A_SCENE_STAGE_W,
           PET2D_MVP_A_SCENE_STAGE_H);
    printf("[PET2D_MVP_A_ACTION] state=IDLE pose=IDLE pet=%d,%d\n",
           g_mvp_a_scene_model.pet_x,
           g_mvp_a_scene_model.pet_y);
    return PET_RESULT_OK;
}

pet_result_t pet2d_mvp_a_scene_skeleton_tick(pet_u32_t now_ms)
{
    pet_result_t ret;
    pet_u32_t elapsed_ms;

    if (pet2d_mvp_a_scene_state_is_active(g_mvp_a_scene_model.state) == PET_FALSE) {
        return PET_RESULT_OK;
    }
    if (g_mvp_a_scene_model.state == PET2D_MVP_A_SCENE_STATE_EXITING) {
        return PET_RESULT_OK;
    }

    now_ms = pet2d_mvp_a_scene_resolve_now(now_ms);
    g_mvp_a_scene_stats.tick_count++;
    elapsed_ms = pet2d_mvp_a_scene_elapsed_ms(now_ms, g_mvp_a_scene_model.enter_ms);
    if (elapsed_ms >= PET2D_MVP_A_SCENE_TIMEOUT_MS) {
        return pet2d_mvp_a_scene_exit_with_reason(PET2D_MVP_A_SCENE_EXIT_TIMEOUT,
                                                  now_ms);
    }

    ret = pet2d_mvp_a_scene_finish_action_if_due(now_ms, 1u);
    if (ret != PET_RESULT_OK) {
        return pet2d_mvp_a_scene_exit_with_reason(PET2D_MVP_A_SCENE_EXIT_ERROR,
                                                  now_ms);
    }

    if (pet2d_mvp_a_scene_elapsed_ms(now_ms, g_mvp_a_scene_ctx.last_render_ms) >=
        PET2D_MVP_A_SCENE_RENDER_INTERVAL_MS) {
        g_mvp_a_scene_ctx.last_render_ms = now_ms;
        ret = pet2d_mvp_a_scene_render_once();
        if (ret != PET_RESULT_OK) {
            return pet2d_mvp_a_scene_exit_with_reason(PET2D_MVP_A_SCENE_EXIT_ERROR,
                                                      now_ms);
        }
    }
    return PET_RESULT_OK;
}

pet_result_t pet2d_mvp_a_scene_skeleton_handle_key(const pet_key_event_t *event)
{
    pet_result_t ret;
    pet_u32_t now_ms;
    pet_i16_t old_x;
    pet_i16_t old_y;
    pet2d_mvp_a_scene_pose_t old_pose;

    if (event == 0) {
        return PET_RESULT_INVALID_ARGUMENT;
    }
    if (pet2d_mvp_a_scene_state_is_active(g_mvp_a_scene_model.state) == PET_FALSE) {
        return PET_RESULT_NOT_READY;
    }
    if (g_mvp_a_scene_model.state == PET2D_MVP_A_SCENE_STATE_EXITING) {
        return PET_RESULT_NOT_READY;
    }

    g_mvp_a_scene_stats.key_event_count++;
    if (event->type != PET_KEY_EVENT_CLICK) {
        return PET_RESULT_OK;
    }

    now_ms = pet2d_mvp_a_scene_resolve_now(event->timestamp_ms);
    if (event->key == PET_KEY_CANCEL) {
        pet2d_mvp_a_scene_set_state(PET2D_MVP_A_SCENE_STATE_EXITING);
        g_mvp_a_scene_model.exit_reason = PET2D_MVP_A_SCENE_EXIT_CANCEL;
        return pet2d_mvp_a_scene_exit_with_reason(PET2D_MVP_A_SCENE_EXIT_CANCEL,
                                                  now_ms);
    }

    ret = pet2d_mvp_a_scene_finish_action_if_due(now_ms, 0u);
    if (ret != PET_RESULT_OK) {
        return ret;
    }

    old_x = g_mvp_a_scene_model.pet_x;
    old_y = g_mvp_a_scene_model.pet_y;
    old_pose = g_mvp_a_scene_model.pose;
    g_mvp_a_scene_model.prev_x = g_mvp_a_scene_model.pet_x;
    g_mvp_a_scene_model.prev_y = g_mvp_a_scene_model.pet_y;

    if (event->key == PET_KEY_LEFT_UP) {
        g_mvp_a_scene_model.pet_x =
            (pet_i16_t)(g_mvp_a_scene_model.pet_x - PET2D_MVP_A_SCENE_STEP_PIXELS);
        pet2d_mvp_a_scene_clamp_pet();
        pet2d_mvp_a_scene_begin_action(PET2D_MVP_A_SCENE_STATE_MOVE_LEFT,
                                       PET2D_MVP_A_SCENE_POSE_STEP,
                                       now_ms,
                                       PET2D_MVP_A_SCENE_MOVE_ACTION_MS);
        printf("[PET2D_MVP_A_ACTION] input=LEFT_UP state=MOVE_LEFT old=%d,%d new=%d,%d\n",
               old_x, old_y, g_mvp_a_scene_model.pet_x, g_mvp_a_scene_model.pet_y);
    } else if (event->key == PET_KEY_RIGHT_DOWN) {
        g_mvp_a_scene_model.pet_x =
            (pet_i16_t)(g_mvp_a_scene_model.pet_x + PET2D_MVP_A_SCENE_STEP_PIXELS);
        pet2d_mvp_a_scene_clamp_pet();
        pet2d_mvp_a_scene_begin_action(PET2D_MVP_A_SCENE_STATE_MOVE_RIGHT,
                                       PET2D_MVP_A_SCENE_POSE_STEP,
                                       now_ms,
                                       PET2D_MVP_A_SCENE_MOVE_ACTION_MS);
        printf("[PET2D_MVP_A_ACTION] input=RIGHT_DOWN state=MOVE_RIGHT old=%d,%d new=%d,%d\n",
               old_x, old_y, g_mvp_a_scene_model.pet_x, g_mvp_a_scene_model.pet_y);
    } else if (event->key == PET_KEY_OK) {
        g_mvp_a_scene_ctx.pose_cycle =
            (pet_u8_t)((g_mvp_a_scene_ctx.pose_cycle + 1u) % 3u);
        if (g_mvp_a_scene_ctx.pose_cycle == 1u) {
            g_mvp_a_scene_model.pose = PET2D_MVP_A_SCENE_POSE_HAPPY;
        } else if (g_mvp_a_scene_ctx.pose_cycle == 2u) {
            g_mvp_a_scene_model.pose = PET2D_MVP_A_SCENE_POSE_BLINK;
        } else {
            g_mvp_a_scene_model.pose = PET2D_MVP_A_SCENE_POSE_IDLE;
        }
        pet2d_mvp_a_scene_begin_action(PET2D_MVP_A_SCENE_STATE_ACTION,
                                       g_mvp_a_scene_model.pose,
                                       now_ms,
                                       PET2D_MVP_A_SCENE_POSE_ACTION_MS);
        g_mvp_a_scene_stats.action_toggle_count++;
        printf("[PET2D_MVP_A_ACTION] input=OK state=ACTION old_pose=%s new_pose=%s\n",
               pet2d_mvp_a_scene_pose_name(old_pose),
               pet2d_mvp_a_scene_pose_name(g_mvp_a_scene_model.pose));
    } else {
        return PET_RESULT_OK;
    }

    ret = pet2d_mvp_a_scene_render_once();
    if (ret != PET_RESULT_OK) {
        return pet2d_mvp_a_scene_exit_with_reason(PET2D_MVP_A_SCENE_EXIT_ERROR,
                                                  now_ms);
    }
    g_mvp_a_scene_ctx.last_render_ms = now_ms;
    return PET_RESULT_OK;
}

pet_result_t pet2d_mvp_a_scene_skeleton_exit(void)
{
    return pet2d_mvp_a_scene_exit_with_reason(PET2D_MVP_A_SCENE_EXIT_CANCEL,
                                              pet2d_mvp_a_scene_now_ms());
}

pet_bool_t pet2d_mvp_a_scene_skeleton_is_active(void)
{
    return pet2d_mvp_a_scene_state_is_active(g_mvp_a_scene_model.state);
}

pet_result_t pet2d_mvp_a_scene_skeleton_get_state(pet2d_mvp_a_scene_state_t *out_state)
{
    if (out_state == 0) {
        return PET_RESULT_INVALID_ARGUMENT;
    }
    *out_state = g_mvp_a_scene_model.state;
    return PET_RESULT_OK;
}

pet_result_t pet2d_mvp_a_scene_skeleton_get_model(
    pet2d_mvp_a_scene_model_t *out_model)
{
    if (out_model == 0) {
        return PET_RESULT_INVALID_ARGUMENT;
    }
    *out_model = g_mvp_a_scene_model;
    return PET_RESULT_OK;
}

pet_result_t pet2d_mvp_a_scene_skeleton_get_draw_cmd(
    pet2d_mvp_a_scene_draw_cmd_t *out_cmd)
{
    if (out_cmd == 0) {
        return PET_RESULT_INVALID_ARGUMENT;
    }
    *out_cmd = g_mvp_a_scene_draw_cmd;
    return PET_RESULT_OK;
}

pet_result_t pet2d_mvp_a_scene_skeleton_get_render_plan(
    pet2d_mvp_a_render_plan_t *out_plan)
{
    if (out_plan == 0) {
        return PET_RESULT_INVALID_ARGUMENT;
    }
    *out_plan = g_mvp_a_scene_render_plan;
    return PET_RESULT_OK;
}

pet_result_t pet2d_mvp_a_scene_skeleton_get_stats(
    pet2d_mvp_a_scene_stats_t *out_stats)
{
    if (out_stats == 0) {
        return PET_RESULT_INVALID_ARGUMENT;
    }
    *out_stats = g_mvp_a_scene_stats;
    return PET_RESULT_OK;
}

pet_result_t pet2d_mvp_a_scene_skeleton_reset_stats(void)
{
    pet_u8_t *stats_bytes = (pet_u8_t *)&g_mvp_a_scene_stats;
    pet_u8_t *model_bytes = (pet_u8_t *)&g_mvp_a_scene_model;
    pet_u8_t *cmd_bytes = (pet_u8_t *)&g_mvp_a_scene_draw_cmd;
    pet_u8_t *plan_bytes = (pet_u8_t *)&g_mvp_a_scene_render_plan;
    pet_u32_t i;

    for (i = 0u; i < (pet_u32_t)sizeof(g_mvp_a_scene_stats); i++) {
        stats_bytes[i] = 0u;
    }
    for (i = 0u; i < (pet_u32_t)sizeof(g_mvp_a_scene_model); i++) {
        model_bytes[i] = 0u;
    }
    for (i = 0u; i < (pet_u32_t)sizeof(g_mvp_a_scene_draw_cmd); i++) {
        cmd_bytes[i] = 0u;
    }
    for (i = 0u; i < (pet_u32_t)sizeof(g_mvp_a_scene_render_plan); i++) {
        plan_bytes[i] = 0u;
    }
    pet2d_mvp_a_scene_set_state(PET2D_MVP_A_SCENE_STATE_NONE);
    return PET_RESULT_OK;
}

static pet_result_t pet2d_mvp_a_scene_action_loop_core_self_test(void)
{
    pet_key_event_t event;
    pet2d_mvp_a_scene_model_t model;
    pet2d_mvp_a_scene_draw_cmd_t draw_cmd;
    pet2d_mvp_a_scene_stats_t stats;
    pet_i16_t start_x;
    pet_result_t ret;

    ret = pet2d_mvp_a_scene_skeleton_enter();
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    ret = pet2d_mvp_a_scene_skeleton_get_model(&model);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    if ((model.state != PET2D_MVP_A_SCENE_STATE_IDLE) ||
        (model.pose != PET2D_MVP_A_SCENE_POSE_IDLE)) {
        return PET_RESULT_ERROR;
    }
    start_x = model.pet_x;

    event.type = PET_KEY_EVENT_CLICK;
    event.timestamp_ms = model.enter_ms + 10u;
    event.hold_ms = 0u;
    event.repeat_count = 0u;
    event.raw_code = 0u;

    event.key = PET_KEY_LEFT_UP;
    ret = pet2d_mvp_a_scene_skeleton_handle_key(&event);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    ret = pet2d_mvp_a_scene_skeleton_get_model(&model);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    if ((model.state != PET2D_MVP_A_SCENE_STATE_MOVE_LEFT) ||
        (model.pose != PET2D_MVP_A_SCENE_POSE_STEP) ||
        (model.pet_x != (pet_i16_t)(start_x - PET2D_MVP_A_SCENE_STEP_PIXELS))) {
        return PET_RESULT_ERROR;
    }
    ret = pet2d_mvp_a_scene_skeleton_get_stats(&stats);
    if ((ret != PET_RESULT_OK) ||
        (stats.last_dirty_w !=
         (PET2D_MVP_A_SCENE_SPRITE_SIZE + PET2D_MVP_A_SCENE_STEP_PIXELS)) ||
        (stats.last_dirty_h != PET2D_MVP_A_SCENE_SPRITE_SIZE)) {
        return PET_RESULT_ERROR;
    }
    ret = pet2d_mvp_a_scene_skeleton_tick(
        model.action_started_ms + model.action_duration_ms + 1u);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    ret = pet2d_mvp_a_scene_skeleton_get_model(&model);
    if ((ret != PET_RESULT_OK) ||
        (model.state != PET2D_MVP_A_SCENE_STATE_IDLE) ||
        (model.pose != PET2D_MVP_A_SCENE_POSE_IDLE)) {
        return PET_RESULT_ERROR;
    }

    event.key = PET_KEY_RIGHT_DOWN;
    event.timestamp_ms = model.enter_ms + 300u;
    ret = pet2d_mvp_a_scene_skeleton_handle_key(&event);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    ret = pet2d_mvp_a_scene_skeleton_get_model(&model);
    if ((ret != PET_RESULT_OK) ||
        (model.state != PET2D_MVP_A_SCENE_STATE_MOVE_RIGHT) ||
        (model.pet_x != start_x)) {
        return PET_RESULT_ERROR;
    }

    event.key = PET_KEY_OK;
    event.timestamp_ms = model.enter_ms + 500u;
    ret = pet2d_mvp_a_scene_skeleton_handle_key(&event);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    ret = pet2d_mvp_a_scene_skeleton_get_model(&model);
    if ((ret != PET_RESULT_OK) ||
        (model.state != PET2D_MVP_A_SCENE_STATE_ACTION) ||
        (model.pose == PET2D_MVP_A_SCENE_POSE_IDLE)) {
        return PET_RESULT_ERROR;
    }
    ret = pet2d_mvp_a_scene_skeleton_get_draw_cmd(&draw_cmd);
    if ((ret != PET_RESULT_OK) ||
        (draw_cmd.type != PET2D_MVP_A_RENDER_CMD_PET_PLACEHOLDER) ||
        (draw_cmd.dst.x != model.pet_x) ||
        (draw_cmd.dst.y != model.pet_y) ||
        (draw_cmd.dst.w != PET2D_MVP_A_SCENE_SPRITE_SIZE) ||
        (draw_cmd.dst.h != PET2D_MVP_A_SCENE_SPRITE_SIZE) ||
        (draw_cmd.pose != (pet_u8_t)model.pose)) {
        return PET_RESULT_ERROR;
    }
    ret = pet2d_mvp_a_scene_skeleton_tick(
        model.action_started_ms + model.action_duration_ms + 1u);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    ret = pet2d_mvp_a_scene_skeleton_get_model(&model);
    if ((ret != PET_RESULT_OK) ||
        (model.state != PET2D_MVP_A_SCENE_STATE_IDLE)) {
        return PET_RESULT_ERROR;
    }

    event.key = PET_KEY_CANCEL;
    event.timestamp_ms = model.enter_ms + 800u;
    ret = pet2d_mvp_a_scene_skeleton_handle_key(&event);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    ret = pet2d_mvp_a_scene_skeleton_get_model(&model);
    if ((ret != PET_RESULT_OK) ||
        (model.exit_reason != PET2D_MVP_A_SCENE_EXIT_CANCEL) ||
        (model.state != PET2D_MVP_A_SCENE_STATE_DONE)) {
        return PET_RESULT_ERROR;
    }

    ret = pet2d_mvp_a_scene_skeleton_enter();
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    ret = pet2d_mvp_a_scene_skeleton_get_model(&model);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    ret = pet2d_mvp_a_scene_skeleton_tick(
        model.enter_ms + PET2D_MVP_A_SCENE_TIMEOUT_MS + 1u);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    ret = pet2d_mvp_a_scene_skeleton_get_model(&model);
    if ((ret != PET_RESULT_OK) ||
        (model.exit_reason != PET2D_MVP_A_SCENE_EXIT_TIMEOUT) ||
        (model.state != PET2D_MVP_A_SCENE_STATE_DONE)) {
        return PET_RESULT_ERROR;
    }
    return PET_RESULT_OK;
}

pet_result_t pet2d_mvp_a_scene_action_loop_self_test(void)
{
    const pet_platform_t *platform = pet_platform_jieli_get();
    pet_display_owner_t original_owner;
    pet_result_t ret;

    if ((platform == 0) || (platform->display_acquire == 0) ||
        (platform->display_release == 0)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    original_owner = pet_display_jieli_get_owner();
    if (original_owner != PET_DISPLAY_OWNER_NONE) {
        (void)platform->display_release(platform->ctx, original_owner);
    }

    ret = pet2d_mvp_a_renderer_contract_self_test();
    if (ret == PET_RESULT_OK) {
        ret = pet2d_mvp_a_scene_skeleton_reset_stats();
    }
    if (ret == PET_RESULT_OK) {
        ret = pet2d_mvp_a_scene_action_loop_core_self_test();
    }
    if (pet_display_jieli_get_owner() == PET_DISPLAY_OWNER_PET2D) {
        (void)platform->display_release(platform->ctx, PET_DISPLAY_OWNER_PET2D);
    }
    if (original_owner != PET_DISPLAY_OWNER_NONE) {
        (void)platform->display_acquire(platform->ctx, original_owner, 0u);
    }
    if (ret == PET_RESULT_OK) {
        pet2d_mvp_a_scene_set_state(PET2D_MVP_A_SCENE_STATE_DONE);
    }
    return ret;
}

pet_result_t pet2d_mvp_a_scene_skeleton_self_test(void)
{
    return pet2d_mvp_a_scene_action_loop_self_test();
}
