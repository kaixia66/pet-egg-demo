#include "pet_platform_jieli_internal.h"

typedef struct {
    pet_display_owner_t owner;
    pet_bool_t sleeping;
    pet_u8_t brightness;
} pet_display_jieli_state_t;

static pet_display_jieli_state_t g_pet_display_jieli_state;

void pet_display_jieli_init(void)
{
    g_pet_display_jieli_state.owner = PET_DISPLAY_OWNER_NONE;
    g_pet_display_jieli_state.sleeping = PET_FALSE;
    g_pet_display_jieli_state.brightness = 80u;
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
    (void)ctx;
    (void)rect;
    (void)rgb565_pixels;
    (void)stride_bytes;
    /* P3 keeps real LCD writes disabled and does not alter the current LVGL flush path. */
    return PET_RESULT_NOT_READY;
}

pet_result_t pet_display_jieli_wait(void *ctx, pet_u32_t timeout_ms)
{
    (void)ctx;
    (void)timeout_ms;
    return PET_RESULT_OK;
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
