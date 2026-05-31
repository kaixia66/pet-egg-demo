#include "pet2d_mvp_a_scene_skeleton.h"

#include "pet_platform_jieli_internal.h"

extern int printf(const char *format, ...);

void mvp_a_lvgl_shell_release_display_owner(void);
void mvp_a_lvgl_shell_request_refresh(void);

typedef struct {
    pet_i16_t stage_x;
    pet_i16_t stage_y;
    pet_i16_t pet_x;
    pet_i16_t pet_y;
    pet_i16_t prev_pet_x;
    pet_i16_t prev_pet_y;
    pet2d_mvp_a_scene_pose_t pose;
    pet_u32_t enter_ms;
    pet_u32_t last_render_ms;
    pet_u8_t full_patch_pending;
} pet2d_mvp_a_scene_context_t;

static pet2d_mvp_a_scene_state_t g_mvp_a_scene_state = PET2D_MVP_A_SCENE_STATE_IDLE;
static pet2d_mvp_a_scene_context_t g_mvp_a_scene_ctx;
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

static void pet2d_mvp_a_scene_set_state(pet2d_mvp_a_scene_state_t state)
{
    g_mvp_a_scene_state = state;
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
    return ((((x >> 3) + (y >> 3) + (pet_u16_t)frame) & 0x01u) != 0u) ?
           0xf81fu : 0x07e0u;
}

static void pet2d_mvp_a_scene_init_context(pet_u32_t now_ms)
{
    g_mvp_a_scene_ctx.stage_x =
        (pet_i16_t)(PET_JIELI_DISPLAY_SAFE_X +
                    ((PET_JIELI_DISPLAY_SAFE_W - PET2D_MVP_A_SCENE_STAGE_W) / 2u));
    g_mvp_a_scene_ctx.stage_y =
        (pet_i16_t)(PET_JIELI_DISPLAY_SAFE_Y +
                    ((PET_JIELI_DISPLAY_SAFE_H - PET2D_MVP_A_SCENE_STAGE_H) / 2u));
    g_mvp_a_scene_ctx.pet_x =
        (pet_i16_t)(g_mvp_a_scene_ctx.stage_x +
                    ((PET2D_MVP_A_SCENE_STAGE_W - PET2D_MVP_A_SCENE_SPRITE_SIZE) / 2u));
    g_mvp_a_scene_ctx.pet_y =
        (pet_i16_t)(g_mvp_a_scene_ctx.stage_y +
                    ((PET2D_MVP_A_SCENE_STAGE_H - PET2D_MVP_A_SCENE_SPRITE_SIZE) / 2u));
    g_mvp_a_scene_ctx.prev_pet_x = g_mvp_a_scene_ctx.pet_x;
    g_mvp_a_scene_ctx.prev_pet_y = g_mvp_a_scene_ctx.pet_y;
    g_mvp_a_scene_ctx.pose = PET2D_MVP_A_SCENE_POSE_IDLE;
    g_mvp_a_scene_ctx.enter_ms = now_ms;
    g_mvp_a_scene_ctx.last_render_ms = now_ms;
    g_mvp_a_scene_ctx.full_patch_pending = 1u;
}

static void pet2d_mvp_a_scene_clamp_pet(void)
{
    pet_i16_t min_x = g_mvp_a_scene_ctx.stage_x;
    pet_i16_t max_x = (pet_i16_t)(g_mvp_a_scene_ctx.stage_x +
                                  PET2D_MVP_A_SCENE_STAGE_W -
                                  PET2D_MVP_A_SCENE_SPRITE_SIZE);

    if (g_mvp_a_scene_ctx.pet_x < min_x) {
        g_mvp_a_scene_ctx.pet_x = min_x;
    }
    if (g_mvp_a_scene_ctx.pet_x > max_x) {
        g_mvp_a_scene_ctx.pet_x = max_x;
    }
}

static pet_result_t pet2d_mvp_a_scene_compute_dirty(pet_i16_t *out_x,
                                                    pet_i16_t *out_y,
                                                    pet_u16_t *out_w,
                                                    pet_u16_t *out_h)
{
    pet_i16_t min_x;
    pet_i16_t min_y;
    pet_i16_t max_x;
    pet_i16_t max_y;

    if ((out_x == 0) || (out_y == 0) || (out_w == 0) || (out_h == 0)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    if (g_mvp_a_scene_ctx.full_patch_pending != 0u) {
        *out_x = g_mvp_a_scene_ctx.stage_x;
        *out_y = g_mvp_a_scene_ctx.stage_y;
        *out_w = PET2D_MVP_A_SCENE_STAGE_W;
        *out_h = PET2D_MVP_A_SCENE_STAGE_H;
        return PET_RESULT_OK;
    }

    min_x = (g_mvp_a_scene_ctx.prev_pet_x < g_mvp_a_scene_ctx.pet_x) ?
            g_mvp_a_scene_ctx.prev_pet_x : g_mvp_a_scene_ctx.pet_x;
    min_y = (g_mvp_a_scene_ctx.prev_pet_y < g_mvp_a_scene_ctx.pet_y) ?
            g_mvp_a_scene_ctx.prev_pet_y : g_mvp_a_scene_ctx.pet_y;
    max_x = (g_mvp_a_scene_ctx.prev_pet_x > g_mvp_a_scene_ctx.pet_x) ?
            g_mvp_a_scene_ctx.prev_pet_x : g_mvp_a_scene_ctx.pet_x;
    max_y = (g_mvp_a_scene_ctx.prev_pet_y > g_mvp_a_scene_ctx.pet_y) ?
            g_mvp_a_scene_ctx.prev_pet_y : g_mvp_a_scene_ctx.pet_y;
    *out_x = min_x;
    *out_y = min_y;
    *out_w = (pet_u16_t)((max_x - min_x) + PET2D_MVP_A_SCENE_SPRITE_SIZE);
    *out_h = (pet_u16_t)((max_y - min_y) + PET2D_MVP_A_SCENE_SPRITE_SIZE);
    if ((*out_w > PET2D_MVP_A_SCENE_STAGE_W) ||
        (*out_h > PET2D_MVP_A_SCENE_STAGE_H)) {
        return PET_RESULT_UNSUPPORTED;
    }
    return PET_RESULT_OK;
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

            if ((abs_x >= g_mvp_a_scene_ctx.pet_x) &&
                (abs_y >= g_mvp_a_scene_ctx.pet_y) &&
                (abs_x < (pet_i16_t)(g_mvp_a_scene_ctx.pet_x +
                                      PET2D_MVP_A_SCENE_SPRITE_SIZE)) &&
                (abs_y < (pet_i16_t)(g_mvp_a_scene_ctx.pet_y +
                                      PET2D_MVP_A_SCENE_SPRITE_SIZE))) {
                pixel = pet2d_mvp_a_scene_pet_pixel(
                    (pet_u16_t)(abs_x - g_mvp_a_scene_ctx.pet_x),
                    (pet_u16_t)(abs_y - g_mvp_a_scene_ctx.pet_y),
                    g_mvp_a_scene_ctx.pose,
                    g_mvp_a_scene_stats.frame_count);
            }
            pixels[((pet_u32_t)y * (pet_u32_t)dirty_w) + (pet_u32_t)x] = pixel;
        }
    }
}
#endif

static pet_result_t pet2d_mvp_a_scene_render_once(void)
{
    pet_i16_t dirty_x;
    pet_i16_t dirty_y;
    pet_u16_t dirty_w;
    pet_u16_t dirty_h;
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
    ret = pet2d_mvp_a_scene_compute_dirty(&dirty_x, &dirty_y, &dirty_w, &dirty_h);
    logic_end_ms = pet2d_mvp_a_scene_now_ms();
    if (ret != PET_RESULT_OK) {
        g_mvp_a_scene_stats.last_result = (pet_u8_t)ret;
        return ret;
    }

    render_start_ms = pet2d_mvp_a_scene_now_ms();
#if PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC
    pet2d_mvp_a_scene_fill_dirty_surface(g_mvp_a_scene_pixels,
                                         dirty_x, dirty_y, dirty_w, dirty_h);
#else
    (void)pet2d_mvp_a_scene_background_pixel(dirty_x, dirty_y);
#endif
    render_end_ms = pet2d_mvp_a_scene_now_ms();

    flush_start_ms = pet2d_mvp_a_scene_now_ms();
#if PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC
    ret = pet_display_jieli_real_flush_poc_rect(dirty_x, dirty_y, dirty_w, dirty_h,
                                                g_mvp_a_scene_pixels, dirty_w);
#else
    ret = PET_RESULT_UNSUPPORTED;
#endif
    flush_end_ms = pet2d_mvp_a_scene_now_ms();
    frame_end_ms = pet2d_mvp_a_scene_now_ms();

    g_mvp_a_scene_stats.frame_count++;
    g_mvp_a_scene_stats.render_count++;
    g_mvp_a_scene_stats.last_dirty_x = (pet_u16_t)dirty_x;
    g_mvp_a_scene_stats.last_dirty_y = (pet_u16_t)dirty_y;
    g_mvp_a_scene_stats.last_dirty_w = dirty_w;
    g_mvp_a_scene_stats.last_dirty_h = dirty_h;
    g_mvp_a_scene_stats.last_pet_x = (pet_u16_t)g_mvp_a_scene_ctx.pet_x;
    g_mvp_a_scene_stats.last_pet_y = (pet_u16_t)g_mvp_a_scene_ctx.pet_y;
    g_mvp_a_scene_stats.last_pose = (pet_u8_t)g_mvp_a_scene_ctx.pose;

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
    g_mvp_a_scene_ctx.prev_pet_x = g_mvp_a_scene_ctx.pet_x;
    g_mvp_a_scene_ctx.prev_pet_y = g_mvp_a_scene_ctx.pet_y;
    g_mvp_a_scene_ctx.full_patch_pending = 0u;

    printf("[PET2D_MVP_A_SCENE] frame=%lu pose=%u pet=%u,%u dirty=%ux%u ret=%d owner=%d\n",
           (unsigned long)g_mvp_a_scene_stats.frame_count,
           g_mvp_a_scene_stats.last_pose,
           g_mvp_a_scene_stats.last_pet_x,
           g_mvp_a_scene_stats.last_pet_y,
           g_mvp_a_scene_stats.last_dirty_w,
           g_mvp_a_scene_stats.last_dirty_h,
           ret,
           pet_display_jieli_get_owner());
    return ret;
}

static pet_result_t pet2d_mvp_a_scene_exit_with_reason(
    pet2d_mvp_a_scene_exit_reason_t reason,
    pet_u32_t now_ms)
{
    const pet_platform_t *platform = pet_platform_jieli_get();
    pet_display_owner_t owner;
    pet_result_t release_ret = PET_RESULT_OK;
    pet_u32_t duration_ms;

    if ((g_mvp_a_scene_state != PET2D_MVP_A_SCENE_STATE_RUNNING) &&
        (g_mvp_a_scene_state != PET2D_MVP_A_SCENE_STATE_ENTERING) &&
        (g_mvp_a_scene_state != PET2D_MVP_A_SCENE_STATE_ERROR)) {
        return PET_RESULT_OK;
    }

    pet2d_mvp_a_scene_set_state(PET2D_MVP_A_SCENE_STATE_EXITING);
    now_ms = pet2d_mvp_a_scene_resolve_now(now_ms);
    owner = pet_display_jieli_get_owner();
    if ((owner == PET_DISPLAY_OWNER_PET2D) &&
        (platform != 0) && (platform->display_release != 0)) {
        release_ret = platform->display_release(platform->ctx, PET_DISPLAY_OWNER_PET2D);
    }

    duration_ms = pet2d_mvp_a_scene_elapsed_ms(now_ms, g_mvp_a_scene_ctx.enter_ms);
    g_mvp_a_scene_stats.exit_count++;
    g_mvp_a_scene_stats.last_exit_ms = now_ms;
    g_mvp_a_scene_stats.last_duration_ms = duration_ms;
    if (duration_ms > g_mvp_a_scene_stats.max_duration_ms) {
        g_mvp_a_scene_stats.max_duration_ms = duration_ms;
    }
    g_mvp_a_scene_stats.last_exit_reason = (pet_u8_t)reason;
    if ((reason == PET2D_MVP_A_SCENE_EXIT_ERROR) ||
        (release_ret != PET_RESULT_OK)) {
        g_mvp_a_scene_stats.error_count++;
        pet2d_mvp_a_scene_set_state(PET2D_MVP_A_SCENE_STATE_ERROR);
    } else {
        pet2d_mvp_a_scene_set_state(PET2D_MVP_A_SCENE_STATE_DONE);
    }

    mvp_a_lvgl_shell_request_refresh();
    printf("[PET2D_MVP_A_SCENE] exit reason=%d release_ret=%d duration=%lu "
           "frames=%lu keys=%lu owner=%d\n",
           reason,
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

    if ((g_mvp_a_scene_state == PET2D_MVP_A_SCENE_STATE_ENTERING) ||
        (g_mvp_a_scene_state == PET2D_MVP_A_SCENE_STATE_RUNNING)) {
        return PET_RESULT_BUSY;
    }
    if ((platform == 0) || (platform->display_acquire == 0)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    pet2d_mvp_a_scene_set_state(PET2D_MVP_A_SCENE_STATE_ENTERING);
    now_ms = pet2d_mvp_a_scene_now_ms();
    pet2d_mvp_a_scene_init_context(now_ms);
    g_mvp_a_scene_stats.enter_count++;
    g_mvp_a_scene_stats.last_enter_ms = now_ms;
    g_mvp_a_scene_stats.last_exit_reason = PET2D_MVP_A_SCENE_EXIT_NONE;

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

    pet2d_mvp_a_scene_set_state(PET2D_MVP_A_SCENE_STATE_RUNNING);
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
    return PET_RESULT_OK;
}

pet_result_t pet2d_mvp_a_scene_skeleton_tick(pet_u32_t now_ms)
{
    pet_result_t ret;
    pet_u32_t elapsed_ms;

    if (g_mvp_a_scene_state != PET2D_MVP_A_SCENE_STATE_RUNNING) {
        return PET_RESULT_OK;
    }

    now_ms = pet2d_mvp_a_scene_resolve_now(now_ms);
    g_mvp_a_scene_stats.tick_count++;
    elapsed_ms = pet2d_mvp_a_scene_elapsed_ms(now_ms, g_mvp_a_scene_ctx.enter_ms);
    if (elapsed_ms >= PET2D_MVP_A_SCENE_TIMEOUT_MS) {
        return pet2d_mvp_a_scene_exit_with_reason(PET2D_MVP_A_SCENE_EXIT_TIMEOUT,
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

    if (event == 0) {
        return PET_RESULT_INVALID_ARGUMENT;
    }
    if (g_mvp_a_scene_state != PET2D_MVP_A_SCENE_STATE_RUNNING) {
        return PET_RESULT_NOT_READY;
    }

    g_mvp_a_scene_stats.key_event_count++;
    if (event->type != PET_KEY_EVENT_CLICK) {
        return PET_RESULT_OK;
    }

    if (event->key == PET_KEY_CANCEL) {
        return pet2d_mvp_a_scene_exit_with_reason(
            PET2D_MVP_A_SCENE_EXIT_CANCEL,
            pet2d_mvp_a_scene_resolve_now(event->timestamp_ms));
    }

    g_mvp_a_scene_ctx.prev_pet_x = g_mvp_a_scene_ctx.pet_x;
    g_mvp_a_scene_ctx.prev_pet_y = g_mvp_a_scene_ctx.pet_y;
    if (event->key == PET_KEY_LEFT_UP) {
        g_mvp_a_scene_ctx.pet_x =
            (pet_i16_t)(g_mvp_a_scene_ctx.pet_x - PET2D_MVP_A_SCENE_STEP_PIXELS);
        pet2d_mvp_a_scene_clamp_pet();
    } else if (event->key == PET_KEY_RIGHT_DOWN) {
        g_mvp_a_scene_ctx.pet_x =
            (pet_i16_t)(g_mvp_a_scene_ctx.pet_x + PET2D_MVP_A_SCENE_STEP_PIXELS);
        pet2d_mvp_a_scene_clamp_pet();
    } else if (event->key == PET_KEY_OK) {
        g_mvp_a_scene_ctx.pose =
            (pet2d_mvp_a_scene_pose_t)(g_mvp_a_scene_ctx.pose + 1);
        if (g_mvp_a_scene_ctx.pose >= PET2D_MVP_A_SCENE_POSE_MAX) {
            g_mvp_a_scene_ctx.pose = PET2D_MVP_A_SCENE_POSE_IDLE;
        }
        g_mvp_a_scene_stats.action_toggle_count++;
    } else {
        return PET_RESULT_OK;
    }

    printf("[PET2D_MVP_A_SCENE] key=%d pose=%d pet=%d,%d owner=%d\n",
           event->key,
           g_mvp_a_scene_ctx.pose,
           g_mvp_a_scene_ctx.pet_x,
           g_mvp_a_scene_ctx.pet_y,
           pet_display_jieli_get_owner());
    ret = pet2d_mvp_a_scene_render_once();
    if (ret != PET_RESULT_OK) {
        return pet2d_mvp_a_scene_exit_with_reason(
            PET2D_MVP_A_SCENE_EXIT_ERROR,
            pet2d_mvp_a_scene_resolve_now(event->timestamp_ms));
    }
    g_mvp_a_scene_ctx.last_render_ms = pet2d_mvp_a_scene_now_ms();
    return PET_RESULT_OK;
}

pet_result_t pet2d_mvp_a_scene_skeleton_exit(void)
{
    return pet2d_mvp_a_scene_exit_with_reason(PET2D_MVP_A_SCENE_EXIT_CANCEL,
                                              pet2d_mvp_a_scene_now_ms());
}

pet_bool_t pet2d_mvp_a_scene_skeleton_is_active(void)
{
    return (g_mvp_a_scene_state == PET2D_MVP_A_SCENE_STATE_ENTERING) ||
           (g_mvp_a_scene_state == PET2D_MVP_A_SCENE_STATE_RUNNING) ||
           (g_mvp_a_scene_state == PET2D_MVP_A_SCENE_STATE_EXITING);
}

pet_result_t pet2d_mvp_a_scene_skeleton_get_state(pet2d_mvp_a_scene_state_t *out_state)
{
    if (out_state == 0) {
        return PET_RESULT_INVALID_ARGUMENT;
    }
    *out_state = g_mvp_a_scene_state;
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
    pet_u8_t *bytes = (pet_u8_t *)&g_mvp_a_scene_stats;
    pet_u32_t i;

    for (i = 0u; i < (pet_u32_t)sizeof(g_mvp_a_scene_stats); i++) {
        bytes[i] = 0u;
    }
    pet2d_mvp_a_scene_set_state(PET2D_MVP_A_SCENE_STATE_IDLE);
    return PET_RESULT_OK;
}

pet_result_t pet2d_mvp_a_scene_skeleton_self_test(void)
{
    const pet_platform_t *platform = pet_platform_jieli_get();
    pet_display_owner_t original_owner;
    pet_key_event_t event;
    pet2d_mvp_a_scene_state_t state;
    pet2d_mvp_a_scene_stats_t stats;
    pet_result_t ret;

    if ((platform == 0) || (platform->display_acquire == 0) ||
        (platform->display_release == 0)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    original_owner = pet_display_jieli_get_owner();
    if (original_owner != PET_DISPLAY_OWNER_NONE) {
        (void)platform->display_release(platform->ctx, original_owner);
    }

    ret = pet2d_mvp_a_scene_skeleton_reset_stats();
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    ret = pet2d_mvp_a_scene_skeleton_enter();
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    if (pet2d_mvp_a_scene_skeleton_get_state(&state) != PET_RESULT_OK) {
        return PET_RESULT_ERROR;
    }
    if (state != PET2D_MVP_A_SCENE_STATE_RUNNING) {
        return PET_RESULT_ERROR;
    }

    event.key = PET_KEY_RIGHT_DOWN;
    event.type = PET_KEY_EVENT_CLICK;
    event.timestamp_ms = 0u;
    event.hold_ms = 0u;
    event.repeat_count = 0u;
    event.raw_code = 0u;
    ret = pet2d_mvp_a_scene_skeleton_handle_key(&event);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    event.key = PET_KEY_OK;
    event.timestamp_ms = 0u;
    ret = pet2d_mvp_a_scene_skeleton_handle_key(&event);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    event.key = PET_KEY_CANCEL;
    event.timestamp_ms = 0u;
    ret = pet2d_mvp_a_scene_skeleton_handle_key(&event);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    if (pet2d_mvp_a_scene_skeleton_get_state(&state) != PET_RESULT_OK) {
        return PET_RESULT_ERROR;
    }
    if (state != PET2D_MVP_A_SCENE_STATE_DONE) {
        return PET_RESULT_ERROR;
    }

    ret = pet2d_mvp_a_scene_skeleton_enter();
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    ret = pet2d_mvp_a_scene_skeleton_tick(
        g_mvp_a_scene_ctx.enter_ms + PET2D_MVP_A_SCENE_TIMEOUT_MS + 1u);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    if (pet2d_mvp_a_scene_skeleton_get_stats(&stats) != PET_RESULT_OK) {
        return PET_RESULT_ERROR;
    }
    if ((stats.enter_count != 2u) ||
        (stats.exit_count != 2u) ||
        (stats.key_event_count < 3u) ||
        (stats.action_toggle_count == 0u) ||
        (stats.frame_count == 0u) ||
        (stats.last_exit_reason != (pet_u8_t)PET2D_MVP_A_SCENE_EXIT_TIMEOUT)) {
        return PET_RESULT_ERROR;
    }

    if (pet_display_jieli_get_owner() == PET_DISPLAY_OWNER_PET2D) {
        (void)platform->display_release(platform->ctx, PET_DISPLAY_OWNER_PET2D);
    }
    if (original_owner != PET_DISPLAY_OWNER_NONE) {
        (void)platform->display_acquire(platform->ctx, original_owner, 0u);
    }
    pet2d_mvp_a_scene_set_state(PET2D_MVP_A_SCENE_STATE_DONE);
    return PET_RESULT_OK;
}
