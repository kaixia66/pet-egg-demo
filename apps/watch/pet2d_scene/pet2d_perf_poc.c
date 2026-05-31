#include "pet2d_perf_poc.h"

#include "pet_platform_jieli_internal.h"

extern int printf(const char *format, ...);

void mvp_a_lvgl_shell_release_display_owner(void);
void mvp_a_lvgl_shell_request_refresh(void);

typedef struct {
    pet_i16_t x;
    pet_i16_t y;
    pet_i16_t dx;
} pet2d_perf_motion_t;

static pet2d_perf_stats_t g_pet2d_perf_stats;

static pet_u32_t pet2d_perf_now_ms(void)
{
    const pet_platform_t *platform = pet_platform_jieli_get();

    if ((platform == 0) || (platform->millis == 0)) {
        return 0u;
    }
    return platform->millis(platform->ctx);
}

static pet_u32_t pet2d_perf_elapsed_ms(pet_u32_t end_ms, pet_u32_t start_ms)
{
    return (pet_u32_t)(end_ms - start_ms);
}

static pet_u16_t pet2d_perf_mode_size(pet2d_perf_mode_t mode)
{
    switch (mode) {
    case PET2D_PERF_MODE_RECT_32:
        return 32u;
    case PET2D_PERF_MODE_RECT_64:
        return 64u;
    case PET2D_PERF_MODE_RECT_96:
        return 96u;
    case PET2D_PERF_MODE_RECT_128:
        return 128u;
    default:
        break;
    }
    return 0u;
}

static pet_result_t pet2d_perf_validate_args(pet2d_perf_mode_t mode,
                                             pet_u16_t frame_count)
{
    if ((mode < PET2D_PERF_MODE_RECT_32) || (mode >= PET2D_PERF_MODE_MAX)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }
    if ((frame_count == 0u) || (frame_count > PET2D_PERF_POC_FRAME_MAX)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }
    return PET_RESULT_OK;
}

static void pet2d_perf_clear_last_run(void)
{
    pet_u32_t run_count = g_pet2d_perf_stats.run_count;
    pet_u8_t *bytes = (pet_u8_t *)&g_pet2d_perf_stats;
    pet_u32_t i;

    for (i = 0u; i < (pet_u32_t)sizeof(g_pet2d_perf_stats); i++) {
        bytes[i] = 0u;
    }
    g_pet2d_perf_stats.run_count = run_count;
    g_pet2d_perf_stats.logic_min_ms = 0xffffffffu;
    g_pet2d_perf_stats.render_min_ms = 0xffffffffu;
    g_pet2d_perf_stats.flush_min_ms = 0xffffffffu;
    g_pet2d_perf_stats.frame_min_ms = 0xffffffffu;
    g_pet2d_perf_stats.last_result = (pet_u8_t)PET_RESULT_NOT_READY;
}

static void pet2d_perf_update_metric(pet_u32_t value,
                                     pet_u32_t *total,
                                     pet_u32_t *min_value,
                                     pet_u32_t *max_value)
{
    *total += value;
    if (value < *min_value) {
        *min_value = value;
    }
    if (value > *max_value) {
        *max_value = value;
    }
}

static void pet2d_perf_finalize_averages(void)
{
    pet_u32_t frames = g_pet2d_perf_stats.frame_attempt_count;

    if (frames == 0u) {
        g_pet2d_perf_stats.logic_min_ms = 0u;
        g_pet2d_perf_stats.render_min_ms = 0u;
        g_pet2d_perf_stats.flush_min_ms = 0u;
        g_pet2d_perf_stats.frame_min_ms = 0u;
        return;
    }

    g_pet2d_perf_stats.logic_avg_ms = g_pet2d_perf_stats.logic_total_ms / frames;
    g_pet2d_perf_stats.render_avg_ms = g_pet2d_perf_stats.render_total_ms / frames;
    g_pet2d_perf_stats.flush_avg_ms = g_pet2d_perf_stats.flush_total_ms / frames;
    g_pet2d_perf_stats.frame_avg_ms = g_pet2d_perf_stats.frame_total_ms / frames;
    if (g_pet2d_perf_stats.frame_total_ms != 0u) {
        g_pet2d_perf_stats.approx_fps_x100 =
            (frames * 100000u) / g_pet2d_perf_stats.frame_total_ms;
    }
}

static void pet2d_perf_wait_ms(pet_u16_t delay_ms)
{
    pet_u32_t start_ms;

    if (delay_ms == 0u) {
        return;
    }
    start_ms = pet2d_perf_now_ms();
    while (pet2d_perf_elapsed_ms(pet2d_perf_now_ms(), start_ms) < delay_ms) {
        /* Bounded manual board-test delay only. */
    }
}

static pet_u16_t pet2d_perf_background_pixel(pet_i16_t x, pet_i16_t y)
{
    pet_u16_t shade = (pet_u16_t)((((pet_u16_t)x >> 4) ^ ((pet_u16_t)y >> 4)) & 0x01u);

    return shade ? 0x18c3u : 0x2104u;
}

static pet_u16_t pet2d_perf_sprite_pixel(pet_u16_t x, pet_u16_t y,
                                         pet_u16_t size, pet_u16_t frame)
{
    pet_u16_t border = (pet_u16_t)((x < 2u) || (y < 2u) ||
                                   (x >= (pet_u16_t)(size - 2u)) ||
                                   (y >= (pet_u16_t)(size - 2u)));

    if (border != 0u) {
        return 0xffffu;
    }
    if ((((x >> 3) + (y >> 3) + frame) & 0x01u) != 0u) {
        return 0xfbe0u;
    }
    return (frame & 0x01u) ? 0x07ffu : 0xf81fu;
}

static void pet2d_perf_fill_dirty_surface(pet_u16_t *pixels,
                                          pet_u16_t dirty_w,
                                          pet_u16_t dirty_h,
                                          pet_i16_t dirty_x,
                                          pet_i16_t dirty_y,
                                          pet_i16_t sprite_x,
                                          pet_i16_t sprite_y,
                                          pet_u16_t sprite_size,
                                          pet_u16_t frame_index)
{
    pet_u16_t x;
    pet_u16_t y;

    for (y = 0u; y < dirty_h; y++) {
        for (x = 0u; x < dirty_w; x++) {
            pet_i16_t abs_x = (pet_i16_t)(dirty_x + (pet_i16_t)x);
            pet_i16_t abs_y = (pet_i16_t)(dirty_y + (pet_i16_t)y);
            pet_u16_t pixel = pet2d_perf_background_pixel(abs_x, abs_y);

            if ((abs_x >= sprite_x) &&
                (abs_y >= sprite_y) &&
                (abs_x < (pet_i16_t)(sprite_x + (pet_i16_t)sprite_size)) &&
                (abs_y < (pet_i16_t)(sprite_y + (pet_i16_t)sprite_size))) {
                pixel = pet2d_perf_sprite_pixel((pet_u16_t)(abs_x - sprite_x),
                                                (pet_u16_t)(abs_y - sprite_y),
                                                sprite_size,
                                                frame_index);
            }
            pixels[((pet_u32_t)y * (pet_u32_t)dirty_w) + (pet_u32_t)x] = pixel;
        }
    }
}

static void pet2d_perf_init_motion(pet2d_perf_motion_t *motion,
                                   pet_u16_t rect_size)
{
    motion->x = (pet_i16_t)(PET_JIELI_DISPLAY_SAFE_X +
                            ((PET_JIELI_DISPLAY_SAFE_W - rect_size) / 2u));
    motion->y = (pet_i16_t)(PET_JIELI_DISPLAY_SAFE_Y +
                            ((PET_JIELI_DISPLAY_SAFE_H - rect_size) / 2u));
    motion->dx = (pet_i16_t)PET2D_PERF_POC_STEP_PIXELS;
}

static pet_result_t pet2d_perf_compute_dirty(pet_u16_t rect_size,
                                             pet_i16_t old_x,
                                             pet_i16_t old_y,
                                             pet_i16_t new_x,
                                             pet_i16_t new_y,
                                             pet_i16_t *out_x,
                                             pet_i16_t *out_y,
                                             pet_u16_t *out_w,
                                             pet_u16_t *out_h)
{
    pet_i16_t min_x;
    pet_i16_t min_y;
    pet_i16_t max_x;
    pet_i16_t max_y;
    pet_u16_t dirty_w;
    pet_u16_t dirty_h;

    if ((out_x == 0) || (out_y == 0) || (out_w == 0) || (out_h == 0)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    min_x = (old_x < new_x) ? old_x : new_x;
    min_y = (old_y < new_y) ? old_y : new_y;
    max_x = (old_x > new_x) ? old_x : new_x;
    max_y = (old_y > new_y) ? old_y : new_y;
    dirty_w = (pet_u16_t)((max_x - min_x) + (pet_i16_t)rect_size);
    dirty_h = (pet_u16_t)((max_y - min_y) + (pet_i16_t)rect_size);
    if ((dirty_w > PET2D_PERF_POC_MAX_SURFACE_SIZE) ||
        (dirty_h > PET2D_PERF_POC_MAX_SURFACE_SIZE)) {
        return PET_RESULT_UNSUPPORTED;
    }

    *out_x = min_x;
    *out_y = min_y;
    *out_w = dirty_w;
    *out_h = dirty_h;
    return PET_RESULT_OK;
}

#if PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC
static pet_result_t pet2d_perf_run_frames(pet2d_perf_mode_t mode,
                                          pet_u16_t frame_count,
                                          pet_u16_t frame_delay_ms)
{
    static pet_u16_t pixels[PET2D_PERF_POC_MAX_SURFACE_SIZE *
                            PET2D_PERF_POC_MAX_SURFACE_SIZE];
    const pet_platform_t *platform = pet_platform_jieli_get();
    pet2d_perf_motion_t motion;
    pet_u16_t rect_size = pet2d_perf_mode_size(mode);
    pet_u16_t frame;
    pet_i16_t min_x = (pet_i16_t)PET_JIELI_DISPLAY_SAFE_X;
    pet_i16_t max_x = (pet_i16_t)(PET_JIELI_DISPLAY_SAFE_X +
                                  PET_JIELI_DISPLAY_SAFE_W - rect_size);
    pet_u32_t run_start_ms;
    pet_result_t ret = PET_RESULT_OK;
    pet_display_owner_t owner;

    if ((platform == 0) || (platform->display_acquire == 0) ||
        (platform->display_release == 0)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }
    if (rect_size >= PET2D_PERF_POC_MAX_SURFACE_SIZE) {
        return PET_RESULT_UNSUPPORTED;
    }

    mvp_a_lvgl_shell_release_display_owner();
    ret = platform->display_acquire(platform->ctx, PET_DISPLAY_OWNER_PET2D, 0u);
    if (ret != PET_RESULT_OK) {
        mvp_a_lvgl_shell_request_refresh();
        return ret;
    }

    pet2d_perf_init_motion(&motion, rect_size);
    run_start_ms = pet2d_perf_now_ms();
    for (frame = 0u; frame < frame_count; frame++) {
        pet_i16_t old_x = motion.x;
        pet_i16_t old_y = motion.y;
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

        frame_start_ms = pet2d_perf_now_ms();
        logic_start_ms = frame_start_ms;
        motion.x = (pet_i16_t)(motion.x + motion.dx);
        if ((motion.x > max_x) || (motion.x < min_x)) {
            motion.dx = (pet_i16_t)(0 - motion.dx);
            motion.x = (pet_i16_t)(old_x + motion.dx);
        }
        if (motion.x > max_x) {
            motion.x = max_x;
        }
        if (motion.x < min_x) {
            motion.x = min_x;
        }
        logic_end_ms = pet2d_perf_now_ms();

        ret = pet2d_perf_compute_dirty(rect_size, old_x, old_y, motion.x, motion.y,
                                       &dirty_x, &dirty_y, &dirty_w, &dirty_h);
        if (ret != PET_RESULT_OK) {
            g_pet2d_perf_stats.last_result = (pet_u8_t)ret;
            break;
        }

        render_start_ms = pet2d_perf_now_ms();
        pet2d_perf_fill_dirty_surface(pixels, dirty_w, dirty_h,
                                      dirty_x, dirty_y,
                                      motion.x, motion.y, rect_size, frame);
        render_end_ms = pet2d_perf_now_ms();

        flush_start_ms = pet2d_perf_now_ms();
        ret = pet_display_jieli_real_flush_poc_rect(dirty_x,
                                                    dirty_y,
                                                    dirty_w,
                                                    dirty_h,
                                                    pixels,
                                                    dirty_w);
        flush_end_ms = pet2d_perf_now_ms();

        if (frame_delay_ms != 0u) {
            pet2d_perf_wait_ms(frame_delay_ms);
        }
        frame_end_ms = pet2d_perf_now_ms();

        g_pet2d_perf_stats.frame_attempt_count++;
        g_pet2d_perf_stats.last_dirty_w = dirty_w;
        g_pet2d_perf_stats.last_dirty_h = dirty_h;
        pet2d_perf_update_metric(pet2d_perf_elapsed_ms(logic_end_ms, logic_start_ms),
                                 &g_pet2d_perf_stats.logic_total_ms,
                                 &g_pet2d_perf_stats.logic_min_ms,
                                 &g_pet2d_perf_stats.logic_max_ms);
        pet2d_perf_update_metric(pet2d_perf_elapsed_ms(render_end_ms, render_start_ms),
                                 &g_pet2d_perf_stats.render_total_ms,
                                 &g_pet2d_perf_stats.render_min_ms,
                                 &g_pet2d_perf_stats.render_max_ms);
        pet2d_perf_update_metric(pet2d_perf_elapsed_ms(flush_end_ms, flush_start_ms),
                                 &g_pet2d_perf_stats.flush_total_ms,
                                 &g_pet2d_perf_stats.flush_min_ms,
                                 &g_pet2d_perf_stats.flush_max_ms);
        pet2d_perf_update_metric(pet2d_perf_elapsed_ms(frame_end_ms, frame_start_ms),
                                 &g_pet2d_perf_stats.frame_total_ms,
                                 &g_pet2d_perf_stats.frame_min_ms,
                                 &g_pet2d_perf_stats.frame_max_ms);
        if (ret == PET_RESULT_OK) {
            g_pet2d_perf_stats.frame_success_count++;
        } else {
            g_pet2d_perf_stats.frame_fail_count++;
            g_pet2d_perf_stats.last_result = (pet_u8_t)ret;
            break;
        }

        if (pet2d_perf_elapsed_ms(frame_end_ms, run_start_ms) >=
            PET2D_PERF_POC_RUN_TIMEOUT_MS) {
            ret = PET_RESULT_TIMEOUT;
            g_pet2d_perf_stats.last_result = (pet_u8_t)ret;
            break;
        }
    }

    if ((ret == PET_RESULT_OK) && (g_pet2d_perf_stats.frame_fail_count == 0u)) {
        g_pet2d_perf_stats.last_result = (pet_u8_t)PET_RESULT_OK;
    }
    pet2d_perf_finalize_averages();

    owner = pet_display_jieli_get_owner();
    if (owner == PET_DISPLAY_OWNER_PET2D) {
        (void)platform->display_release(platform->ctx, PET_DISPLAY_OWNER_PET2D);
    }
    mvp_a_lvgl_shell_request_refresh();
    return ret;
}
#endif

pet_result_t pet2d_perf_poc_run_mode(pet2d_perf_mode_t mode,
                                     pet_u16_t frame_count,
                                     pet_u16_t frame_delay_ms)
{
    pet_result_t ret;
    pet_u16_t rect_size;

    ret = pet2d_perf_validate_args(mode, frame_count);
    if (ret != PET_RESULT_OK) {
        return ret;
    }

    rect_size = pet2d_perf_mode_size(mode);
    pet2d_perf_clear_last_run();
    g_pet2d_perf_stats.run_count++;
    g_pet2d_perf_stats.last_mode = (pet_u8_t)mode;
    g_pet2d_perf_stats.last_rect_w = rect_size;
    g_pet2d_perf_stats.last_rect_h = rect_size;
    g_pet2d_perf_stats.last_frame_count = frame_count;

#if PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC
    ret = pet2d_perf_run_frames(mode, frame_count, frame_delay_ms);
#else
    (void)frame_delay_ms;
    ret = PET_RESULT_UNSUPPORTED;
    g_pet2d_perf_stats.last_result = (pet_u8_t)ret;
#endif
    if ((ret != PET_RESULT_OK) &&
        (g_pet2d_perf_stats.last_result == (pet_u8_t)PET_RESULT_NOT_READY)) {
        g_pet2d_perf_stats.last_result = (pet_u8_t)ret;
    }

    printf("[PET2D_PERF] mode=%u rect=%ux%u frames=%u ok=%lu fail=%lu "
           "fps_x100=%lu avg_flush=%lu max_flush=%lu avg_frame=%lu max_frame=%lu "
           "dirty=%ux%u ret=%d\n",
           (unsigned int)mode,
           (unsigned int)g_pet2d_perf_stats.last_rect_w,
           (unsigned int)g_pet2d_perf_stats.last_rect_h,
           (unsigned int)g_pet2d_perf_stats.last_frame_count,
           (unsigned long)g_pet2d_perf_stats.frame_success_count,
           (unsigned long)g_pet2d_perf_stats.frame_fail_count,
           (unsigned long)g_pet2d_perf_stats.approx_fps_x100,
           (unsigned long)g_pet2d_perf_stats.flush_avg_ms,
           (unsigned long)g_pet2d_perf_stats.flush_max_ms,
           (unsigned long)g_pet2d_perf_stats.frame_avg_ms,
           (unsigned long)g_pet2d_perf_stats.frame_max_ms,
           (unsigned int)g_pet2d_perf_stats.last_dirty_w,
           (unsigned int)g_pet2d_perf_stats.last_dirty_h,
           ret);
    return ret;
}

pet_result_t pet2d_perf_poc_get_stats(pet2d_perf_stats_t *out_stats)
{
    if (out_stats == 0) {
        return PET_RESULT_INVALID_ARGUMENT;
    }
    *out_stats = g_pet2d_perf_stats;
    return PET_RESULT_OK;
}

pet_result_t pet2d_perf_poc_reset_stats(void)
{
    pet_u8_t *bytes = (pet_u8_t *)&g_pet2d_perf_stats;
    pet_u32_t i;

    for (i = 0u; i < (pet_u32_t)sizeof(g_pet2d_perf_stats); i++) {
        bytes[i] = 0u;
    }
    g_pet2d_perf_stats.logic_min_ms = 0xffffffffu;
    g_pet2d_perf_stats.render_min_ms = 0xffffffffu;
    g_pet2d_perf_stats.flush_min_ms = 0xffffffffu;
    g_pet2d_perf_stats.frame_min_ms = 0xffffffffu;
    g_pet2d_perf_stats.last_result = (pet_u8_t)PET_RESULT_NOT_READY;
    return PET_RESULT_OK;
}

pet_result_t pet2d_perf_poc_self_test(void)
{
    pet2d_perf_stats_t stats;
    pet_i16_t dirty_x;
    pet_i16_t dirty_y;
    pet_u16_t dirty_w;
    pet_u16_t dirty_h;
    pet_result_t ret;

    if (pet2d_perf_poc_reset_stats() != PET_RESULT_OK) {
        return PET_RESULT_ERROR;
    }
    if (pet2d_perf_poc_run_mode(PET2D_PERF_MODE_MAX, 1u, 0u) !=
        PET_RESULT_INVALID_ARGUMENT) {
        return PET_RESULT_ERROR;
    }
    if (pet2d_perf_poc_run_mode(PET2D_PERF_MODE_RECT_32, 0u, 0u) !=
        PET_RESULT_INVALID_ARGUMENT) {
        return PET_RESULT_ERROR;
    }
    if (pet2d_perf_poc_run_mode(PET2D_PERF_MODE_RECT_32,
                                (pet_u16_t)(PET2D_PERF_POC_FRAME_MAX + 1u),
                                0u) != PET_RESULT_INVALID_ARGUMENT) {
        return PET_RESULT_ERROR;
    }

    ret = pet2d_perf_compute_dirty(32u, 100, 120, 108, 120,
                                   &dirty_x, &dirty_y, &dirty_w, &dirty_h);
    if ((ret != PET_RESULT_OK) || (dirty_x != 100) || (dirty_y != 120) ||
        (dirty_w != 40u) || (dirty_h != 32u)) {
        return PET_RESULT_ERROR;
    }
    ret = pet2d_perf_compute_dirty(128u, 100, 120, 108, 120,
                                   &dirty_x, &dirty_y, &dirty_w, &dirty_h);
    if (ret != PET_RESULT_UNSUPPORTED) {
        return PET_RESULT_ERROR;
    }

#if PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC
    return PET_RESULT_UNSUPPORTED;
#else
    ret = pet2d_perf_poc_run_mode(PET2D_PERF_MODE_RECT_32, 2u, 0u);
    if (ret != PET_RESULT_UNSUPPORTED) {
        return PET_RESULT_ERROR;
    }
    if (pet2d_perf_poc_get_stats(&stats) != PET_RESULT_OK) {
        return PET_RESULT_ERROR;
    }
    if ((stats.run_count == 0u) ||
        (stats.frame_attempt_count != 0u) ||
        (stats.last_rect_w != 32u) ||
        (stats.last_result != (pet_u8_t)PET_RESULT_UNSUPPORTED)) {
        return PET_RESULT_ERROR;
    }
    return PET_RESULT_OK;
#endif
}
