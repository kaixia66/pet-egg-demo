#include "pet_platform_jieli_internal.h"

#if PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC
extern int lcd_draw_area(unsigned char index, unsigned char *lcd_buf,
                         int left, int top, int width, int height, int wait);
extern void lcd_wait(void);
extern unsigned int timer_get_ms(void);
#endif

typedef struct {
    pet_display_owner_t owner;
    pet_bool_t sleeping;
    pet_u8_t brightness;
    pet_bool_t flush_busy;
    pet_bool_t real_flush_manual_armed;
    pet_display_jieli_flush_stats_t flush_stats;
} pet_display_jieli_state_t;

static pet_display_jieli_state_t g_pet_display_jieli_state;

void pet_display_jieli_init(void)
{
    g_pet_display_jieli_state.owner = PET_DISPLAY_OWNER_NONE;
    g_pet_display_jieli_state.sleeping = PET_FALSE;
    g_pet_display_jieli_state.brightness = 80u;
    g_pet_display_jieli_state.flush_busy = PET_FALSE;
    g_pet_display_jieli_state.real_flush_manual_armed = PET_FALSE;
    (void)pet_display_jieli_reset_flush_stats();
}

pet_display_owner_t pet_display_jieli_get_owner(void)
{
    return g_pet_display_jieli_state.owner;
}

pet_result_t pet_display_jieli_get_profile(void *ctx, pet_display_profile_t *profile)
{
    (void)ctx;

    if (profile == 0) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    /*
     * P3 audit source:
     * - CONFIG_BOARD_701N_LVGL_DEMO enables TCFG_LCD_SPI_SH8601A_ENABLE.
     * - cpu/br28/ui_driver/lvgl/lv_port_disp.c fixes LVGL to 454x454.
     * TODO(P3 board test): confirm true panel geometry, round mask, byte order, TE and alignment.
     */
    profile->width = PET_JIELI_DISPLAY_WIDTH;
    profile->height = PET_JIELI_DISPLAY_HEIGHT;
    profile->shape = PET_JIELI_DISPLAY_SHAPE;
    profile->safe_margin_percent = 12;
    profile->default_scale = 1;
    profile->flush_mode = PET_DISPLAY_FLUSH_MODE_RGB565_RECT;
    profile->rotation = PET_JIELI_DISPLAY_ROTATION;
    profile->rgb565_order = PET_RGB565_ORDER_RGB;
    profile->safe_area.left = PET_JIELI_DISPLAY_SAFE_X;
    profile->safe_area.top = PET_JIELI_DISPLAY_SAFE_Y;
    profile->safe_area.right = PET_JIELI_DISPLAY_SAFE_X + PET_JIELI_DISPLAY_SAFE_W;
    profile->safe_area.bottom = PET_JIELI_DISPLAY_SAFE_Y + PET_JIELI_DISPLAY_SAFE_H;
    profile->stride_align_pixels = 1u;
    profile->min_flush_width = 1u;
    profile->min_flush_height = 1u;
    profile->requires_even_x = 0u;
    profile->requires_even_width = 0u;
    profile->reserved[0] = 0u;
    profile->reserved[1] = 0u;
    return PET_RESULT_OK;
}

pet_result_t pet_display_jieli_acquire(void *ctx, pet_display_owner_t owner, pet_u32_t timeout_ms)
{
    (void)ctx;
    (void)timeout_ms;

    if (owner == PET_DISPLAY_OWNER_NONE) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    if ((g_pet_display_jieli_state.owner != PET_DISPLAY_OWNER_NONE) &&
        (g_pet_display_jieli_state.owner != owner)) {
        return PET_RESULT_BUSY;
    }

    g_pet_display_jieli_state.owner = owner;
    return PET_RESULT_OK;
}

pet_result_t pet_display_jieli_release(void *ctx, pet_display_owner_t owner)
{
    (void)ctx;

    if (owner == PET_DISPLAY_OWNER_NONE) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    if (g_pet_display_jieli_state.owner == PET_DISPLAY_OWNER_NONE) {
        return PET_RESULT_OK;
    }

    if (g_pet_display_jieli_state.owner != owner) {
        return PET_RESULT_BUSY;
    }

    g_pet_display_jieli_state.owner = PET_DISPLAY_OWNER_NONE;
    return PET_RESULT_OK;
}

pet_result_t pet_display_jieli_flush(void *ctx, const pet_display_rect_t *rect,
                                     const void *rgb565_pixels, pet_u32_t stride_bytes)
{
    pet_display_jieli_flush_stats_t *stats = &g_pet_display_jieli_state.flush_stats;
    pet_u32_t pitch_pixels;
    pet_u32_t right;
    pet_u32_t bottom;

    (void)ctx;

    stats->flush_call_count++;
    stats->last_owner = (pet_u8_t)g_pet_display_jieli_state.owner;
    stats->last_mode = (pet_u8_t)PET_DISPLAY_FLUSH_MODE_RGB565_RECT;
    stats->real_flush_enabled = (pet_u8_t)PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC;
    stats->tiny_poc_enabled = (pet_u8_t)PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC;
    stats->busy = (pet_u8_t)g_pet_display_jieli_state.flush_busy;

    if (rect != 0) {
        stats->last_x = (pet_i16_t)rect->x;
        stats->last_y = (pet_i16_t)rect->y;
        stats->last_w = rect->width;
        stats->last_h = rect->height;
    }
    if ((stride_bytes % PET_RGB565_BYTES_PER_PIXEL) == 0u) {
        stats->last_pitch_pixels = (pet_u16_t)(stride_bytes / PET_RGB565_BYTES_PER_PIXEL);
    } else {
        stats->last_pitch_pixels = 0u;
    }

    if ((rect == 0) || (rgb565_pixels == 0)) {
        stats->rejected_count++;
        return PET_RESULT_INVALID_ARGUMENT;
    }
    if ((rect->width == 0u) || (rect->height == 0u)) {
        stats->rejected_count++;
        return PET_RESULT_INVALID_ARGUMENT;
    }
    if ((stride_bytes % PET_RGB565_BYTES_PER_PIXEL) != 0u) {
        stats->rejected_count++;
        return PET_RESULT_INVALID_ARGUMENT;
    }

    pitch_pixels = stride_bytes / PET_RGB565_BYTES_PER_PIXEL;
    if (pitch_pixels < rect->width) {
        stats->rejected_count++;
        return PET_RESULT_INVALID_ARGUMENT;
    }

    right = (pet_u32_t)rect->x + (pet_u32_t)rect->width;
    bottom = (pet_u32_t)rect->y + (pet_u32_t)rect->height;
    if ((right > PET_JIELI_DISPLAY_WIDTH) || (bottom > PET_JIELI_DISPLAY_HEIGHT) ||
        (right < rect->x) || (bottom < rect->y)) {
        stats->rejected_count++;
        return PET_RESULT_INVALID_ARGUMENT;
    }

    if (g_pet_display_jieli_state.owner == PET_DISPLAY_OWNER_NONE) {
        stats->rejected_count++;
        return PET_RESULT_NOT_READY;
    }
    if (g_pet_display_jieli_state.owner == PET_DISPLAY_OWNER_LVGL_SYSTEM_UI) {
        stats->busy_count++;
        return PET_RESULT_BUSY;
    }
    if ((g_pet_display_jieli_state.owner != PET_DISPLAY_OWNER_PET2D) &&
        (g_pet_display_jieli_state.owner != PET_DISPLAY_OWNER_DEBUG)) {
        stats->rejected_count++;
        return PET_RESULT_BUSY;
    }
    if (g_pet_display_jieli_state.flush_busy) {
        stats->busy_count++;
        return PET_RESULT_BUSY;
    }

    stats->total_requested_pixels += (pet_u32_t)rect->width * (pet_u32_t)rect->height;

#if PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC
    {
        unsigned int start_ms;
        unsigned int end_ms;
        int driver_status;

        if (g_pet_display_jieli_state.real_flush_manual_armed != PET_TRUE) {
            stats->last_real_flush_result = PET_RESULT_NOT_READY;
            return PET_RESULT_NOT_READY;
        }

        stats->real_flush_attempt_count++;
        g_pet_display_jieli_state.flush_busy = PET_TRUE;
        stats->busy = 1u;
        start_ms = timer_get_ms();
        driver_status = lcd_draw_area(0u, (unsigned char *)rgb565_pixels,
                                      (int)rect->x, (int)rect->y,
                                      (int)rect->width, (int)rect->height, 1);
        lcd_wait();
        end_ms = timer_get_ms();
        g_pet_display_jieli_state.flush_busy = PET_FALSE;
        stats->busy = 0u;
        stats->last_driver_status = (pet_i32_t)driver_status;
        stats->last_real_flush_duration_ms = (pet_u32_t)(end_ms - start_ms);
        if (driver_status == 0) {
            stats->real_flush_success_count++;
            stats->last_real_flush_result = PET_RESULT_OK;
            return PET_RESULT_OK;
        }
        stats->real_flush_fail_count++;
        stats->last_real_flush_result = PET_RESULT_ERROR;
        return PET_RESULT_ERROR;
    }
#endif
    /* P9 diagnostic path: real LCD writes remain disabled and LVGL flush is untouched. */
    return PET_RESULT_NOT_READY;
}

pet_result_t pet_display_jieli_wait(void *ctx, pet_u32_t timeout_ms)
{
    (void)ctx;
    (void)timeout_ms;
    return PET_RESULT_OK;
}

pet_result_t pet_display_jieli_get_flush_stats(pet_display_jieli_flush_stats_t *out_stats)
{
    if (out_stats == 0) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    *out_stats = g_pet_display_jieli_state.flush_stats;
    out_stats->real_flush_enabled = (pet_u8_t)PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC;
    out_stats->tiny_poc_enabled = (pet_u8_t)PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC;
    out_stats->busy = (pet_u8_t)g_pet_display_jieli_state.flush_busy;
    return PET_RESULT_OK;
}

pet_result_t pet_display_jieli_reset_flush_stats(void)
{
    pet_display_jieli_flush_stats_t *stats = &g_pet_display_jieli_state.flush_stats;

    stats->flush_call_count = 0u;
    stats->rejected_count = 0u;
    stats->busy_count = 0u;
    stats->total_requested_pixels = 0u;
    stats->last_x = 0;
    stats->last_y = 0;
    stats->last_w = 0u;
    stats->last_h = 0u;
    stats->last_pitch_pixels = 0u;
    stats->last_mode = (pet_u8_t)PET_DISPLAY_FLUSH_MODE_RGB565_RECT;
    stats->last_owner = (pet_u8_t)g_pet_display_jieli_state.owner;
    stats->real_flush_enabled = (pet_u8_t)PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC;
    stats->tiny_poc_enabled = (pet_u8_t)PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC;
    stats->busy = (pet_u8_t)g_pet_display_jieli_state.flush_busy;
    stats->real_flush_attempt_count = 0u;
    stats->real_flush_success_count = 0u;
    stats->real_flush_fail_count = 0u;
    stats->last_real_flush_result = PET_RESULT_UNSUPPORTED;
    stats->last_driver_status = 0;
    stats->last_real_flush_duration_ms = 0u;
    return PET_RESULT_OK;
}

pet_result_t pet_display_jieli_real_flush_poc_rect(int x, int y, int w, int h,
                                                   const pet_u16_t *rgb565,
                                                   int pitch_pixels)
{
#if PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC
    const pet_platform_t *platform = pet_platform_jieli_get();
    pet_display_rect_t rect;
    pet_result_t ret;

    if ((platform == 0) || (platform->display_flush == 0)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }
    if ((x < 0) || (y < 0) || (w <= 0) || (h <= 0) || (pitch_pixels < w) ||
        (rgb565 == 0)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    rect.x = (pet_u16_t)x;
    rect.y = (pet_u16_t)y;
    rect.width = (pet_u16_t)w;
    rect.height = (pet_u16_t)h;
    g_pet_display_jieli_state.real_flush_manual_armed = PET_TRUE;
    ret = platform->display_flush(platform->ctx, &rect, rgb565,
                                  (pet_u32_t)pitch_pixels * PET_RGB565_BYTES_PER_PIXEL);
    g_pet_display_jieli_state.real_flush_manual_armed = PET_FALSE;
    return ret;
#else
    (void)x;
    (void)y;
    (void)w;
    (void)h;
    (void)rgb565;
    (void)pitch_pixels;
    return PET_RESULT_UNSUPPORTED;
#endif
}

pet_result_t pet_display_jieli_tiny_flush_poc(void)
{
#if PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC
    static const pet_u16_t k_pixels[64] = {
        0xF800u, 0xF800u, 0xF800u, 0xF800u, 0x07E0u, 0x07E0u, 0x07E0u, 0x07E0u,
        0xF800u, 0xF800u, 0xF800u, 0xF800u, 0x07E0u, 0x07E0u, 0x07E0u, 0x07E0u,
        0xF800u, 0xF800u, 0xF800u, 0xF800u, 0x07E0u, 0x07E0u, 0x07E0u, 0x07E0u,
        0xF800u, 0xF800u, 0xF800u, 0xF800u, 0x07E0u, 0x07E0u, 0x07E0u, 0x07E0u,
        0x001Fu, 0x001Fu, 0x001Fu, 0x001Fu, 0xFFFFu, 0xFFFFu, 0xFFFFu, 0xFFFFu,
        0x001Fu, 0x001Fu, 0x001Fu, 0x001Fu, 0xFFFFu, 0xFFFFu, 0xFFFFu, 0xFFFFu,
        0x001Fu, 0x001Fu, 0x001Fu, 0x001Fu, 0xFFFFu, 0xFFFFu, 0xFFFFu, 0xFFFFu,
        0x001Fu, 0x001Fu, 0x001Fu, 0x001Fu, 0xFFFFu, 0xFFFFu, 0xFFFFu, 0xFFFFu
    };

    return pet_display_jieli_real_flush_poc_rect((PET_JIELI_DISPLAY_WIDTH / 2) - 4,
                                                 (PET_JIELI_DISPLAY_HEIGHT / 2) - 4,
                                                 8, 8, k_pixels, 8);
#else
    return PET_RESULT_UNSUPPORTED;
#endif
}

pet_result_t pet_display_jieli_set_brightness(void *ctx, pet_u8_t percent)
{
    (void)ctx;

    if (percent > 100u) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    g_pet_display_jieli_state.brightness = percent;
    return PET_RESULT_OK;
}

pet_result_t pet_display_jieli_sleep(void *ctx)
{
    (void)ctx;
    g_pet_display_jieli_state.sleeping = PET_TRUE;
    return PET_RESULT_OK;
}

pet_result_t pet_display_jieli_wakeup(void *ctx)
{
    (void)ctx;
    g_pet_display_jieli_state.sleeping = PET_FALSE;
    return PET_RESULT_OK;
}

pet_result_t pet_platform_jieli_display_self_test(void)
{
    const pet_platform_t *platform;
    pet_display_profile_t profile;
    pet_result_t ret;

    platform = pet_platform_jieli_get();
    if ((platform == 0) || (platform->get_display_profile == 0) ||
        (platform->display_acquire == 0) || (platform->display_release == 0)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    ret = platform->get_display_profile(platform->ctx, &profile);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    if ((profile.width == 0u) || (profile.height == 0u)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }
    if (PET_RGB565_BYTES_PER_PIXEL != 2u) {
        return PET_RESULT_BAD_VERSION;
    }
    if ((profile.flush_mode != PET_DISPLAY_FLUSH_MODE_RGB565_FULL) &&
        (profile.flush_mode != PET_DISPLAY_FLUSH_MODE_RGB565_RECT) &&
        (profile.flush_mode != PET_DISPLAY_FLUSH_MODE_RGB565_RECT_ASYNC)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }
    if ((profile.rgb565_order != PET_RGB565_ORDER_RGB) &&
        (profile.rgb565_order != PET_RGB565_ORDER_BGR)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }
    if ((profile.safe_area.left > profile.safe_area.right) ||
        (profile.safe_area.top > profile.safe_area.bottom) ||
        (profile.safe_area.right > profile.width) ||
        (profile.safe_area.bottom > profile.height)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    ret = pet_display_jieli_owner_self_test();
    if (ret != PET_RESULT_OK) {
        return ret;
    }

    return PET_RESULT_OK;
}

pet_result_t pet_display_jieli_owner_self_test(void)
{
    const pet_platform_t *platform;
    pet_display_owner_t original_owner;
    pet_result_t ret;

    platform = pet_platform_jieli_get();
    if ((platform == 0) || (platform->display_acquire == 0) || (platform->display_release == 0)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    original_owner = pet_display_jieli_get_owner();
    pet_display_jieli_init();

    if (pet_display_jieli_get_owner() != PET_DISPLAY_OWNER_NONE) {
        return PET_RESULT_ERROR;
    }

    ret = platform->display_acquire(platform->ctx, PET_DISPLAY_OWNER_NONE, 0u);
    if (ret != PET_RESULT_INVALID_ARGUMENT) {
        return PET_RESULT_ERROR;
    }

    ret = platform->display_acquire(platform->ctx, PET_DISPLAY_OWNER_LVGL_SYSTEM_UI, 0u);
    if (ret != PET_RESULT_OK) {
        return ret;
    }

    /*
     * P3/P4 stub policy: the same owner may acquire again without ref counting.
     * This is only a boundary check and is not the final LCD driver lock model.
     */
    ret = platform->display_acquire(platform->ctx, PET_DISPLAY_OWNER_LVGL_SYSTEM_UI, 0u);
    if (ret != PET_RESULT_OK) {
        (void)platform->display_release(platform->ctx, PET_DISPLAY_OWNER_LVGL_SYSTEM_UI);
        return PET_RESULT_ERROR;
    }

    ret = platform->display_acquire(platform->ctx, PET_DISPLAY_OWNER_PET2D, 0u);
    if (ret != PET_RESULT_BUSY) {
        (void)platform->display_release(platform->ctx, PET_DISPLAY_OWNER_LVGL_SYSTEM_UI);
        return PET_RESULT_ERROR;
    }

    ret = platform->display_release(platform->ctx, PET_DISPLAY_OWNER_PET2D);
    if (ret != PET_RESULT_BUSY) {
        (void)platform->display_release(platform->ctx, PET_DISPLAY_OWNER_LVGL_SYSTEM_UI);
        return PET_RESULT_ERROR;
    }

    ret = platform->display_release(platform->ctx, PET_DISPLAY_OWNER_LVGL_SYSTEM_UI);
    if (ret != PET_RESULT_OK) {
        return ret;
    }

    if (pet_display_jieli_get_owner() != PET_DISPLAY_OWNER_NONE) {
        return PET_RESULT_ERROR;
    }

    if (original_owner != PET_DISPLAY_OWNER_NONE) {
        ret = platform->display_acquire(platform->ctx, original_owner, 0u);
        if (ret != PET_RESULT_OK) {
            return ret;
        }
    }

    return PET_RESULT_OK;
}

pet_result_t pet_display_jieli_flush_self_test(void)
{
    const pet_platform_t *platform;
    pet_display_owner_t original_owner;
    pet_display_rect_t rect;
    pet_display_rect_t bad_rect;
    pet_u16_t pixels[4] = {0u, 1u, 2u, 3u};
    pet_display_jieli_flush_stats_t stats;
    pet_result_t ret;

    platform = pet_platform_jieli_get();
    if ((platform == 0) || (platform->display_acquire == 0) ||
        (platform->display_release == 0) || (platform->display_flush == 0) ||
        (platform->display_wait == 0)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    original_owner = pet_display_jieli_get_owner();
    pet_display_jieli_init();
    (void)pet_display_jieli_reset_flush_stats();

    rect.x = 10u;
    rect.y = 12u;
    rect.width = 2u;
    rect.height = 2u;

    ret = platform->display_flush(platform->ctx, &rect, pixels, 2u * PET_RGB565_BYTES_PER_PIXEL);
    if (ret != PET_RESULT_NOT_READY) {
        return PET_RESULT_ERROR;
    }

    ret = platform->display_acquire(platform->ctx, PET_DISPLAY_OWNER_LVGL_SYSTEM_UI, 0u);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    ret = platform->display_flush(platform->ctx, &rect, pixels, 2u * PET_RGB565_BYTES_PER_PIXEL);
    if (ret != PET_RESULT_BUSY) {
        (void)platform->display_release(platform->ctx, PET_DISPLAY_OWNER_LVGL_SYSTEM_UI);
        return PET_RESULT_ERROR;
    }
    (void)platform->display_release(platform->ctx, PET_DISPLAY_OWNER_LVGL_SYSTEM_UI);

    ret = platform->display_acquire(platform->ctx, PET_DISPLAY_OWNER_PET2D, 0u);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    ret = platform->display_flush(platform->ctx, &rect, pixels, 2u * PET_RGB565_BYTES_PER_PIXEL);
    if (ret != PET_RESULT_NOT_READY) {
        (void)platform->display_release(platform->ctx, PET_DISPLAY_OWNER_PET2D);
        return PET_RESULT_ERROR;
    }
    ret = platform->display_wait(platform->ctx, 0u);
    if (ret != PET_RESULT_OK) {
        (void)platform->display_release(platform->ctx, PET_DISPLAY_OWNER_PET2D);
        return ret;
    }

    ret = platform->display_flush(platform->ctx, &rect, 0, 2u * PET_RGB565_BYTES_PER_PIXEL);
    if (ret != PET_RESULT_INVALID_ARGUMENT) {
        (void)platform->display_release(platform->ctx, PET_DISPLAY_OWNER_PET2D);
        return PET_RESULT_ERROR;
    }

    bad_rect.x = PET_JIELI_DISPLAY_WIDTH - 1u;
    bad_rect.y = PET_JIELI_DISPLAY_HEIGHT - 1u;
    bad_rect.width = 2u;
    bad_rect.height = 2u;
    ret = platform->display_flush(platform->ctx, &bad_rect, pixels, 2u * PET_RGB565_BYTES_PER_PIXEL);
    if (ret != PET_RESULT_INVALID_ARGUMENT) {
        (void)platform->display_release(platform->ctx, PET_DISPLAY_OWNER_PET2D);
        return PET_RESULT_ERROR;
    }

    ret = platform->display_flush(platform->ctx, &rect, pixels, 1u * PET_RGB565_BYTES_PER_PIXEL);
    if (ret != PET_RESULT_INVALID_ARGUMENT) {
        (void)platform->display_release(platform->ctx, PET_DISPLAY_OWNER_PET2D);
        return PET_RESULT_ERROR;
    }

    ret = pet_display_jieli_get_flush_stats(&stats);
    if (ret != PET_RESULT_OK) {
        (void)platform->display_release(platform->ctx, PET_DISPLAY_OWNER_PET2D);
        return ret;
    }
    if ((stats.flush_call_count != 6u) ||
        (stats.rejected_count != 4u) ||
        (stats.busy_count != 1u) ||
        (stats.total_requested_pixels != 4u) ||
        (stats.real_flush_attempt_count != 0u) ||
        (stats.real_flush_success_count != 0u) ||
        (stats.real_flush_fail_count != 0u) ||
        (stats.last_x != (pet_i16_t)rect.x) ||
        (stats.last_y != (pet_i16_t)rect.y) ||
        (stats.last_w != rect.width) ||
        (stats.last_h != rect.height) ||
        (stats.last_pitch_pixels != 1u) ||
        (stats.tiny_poc_enabled != 0u) ||
        (stats.real_flush_enabled != 0u)) {
        (void)platform->display_release(platform->ctx, PET_DISPLAY_OWNER_PET2D);
        return PET_RESULT_ERROR;
    }

    ret = platform->display_release(platform->ctx, PET_DISPLAY_OWNER_PET2D);
    if (ret != PET_RESULT_OK) {
        return ret;
    }

    if (original_owner != PET_DISPLAY_OWNER_NONE) {
        ret = platform->display_acquire(platform->ctx, original_owner, 0u);
        if (ret != PET_RESULT_OK) {
            return ret;
        }
    }

    return PET_RESULT_OK;
}
