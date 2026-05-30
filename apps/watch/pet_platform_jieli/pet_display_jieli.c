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

pet_result_t pet_display_jieli_get_profile(void *ctx, pet_display_profile_t *profile)
{
    (void)ctx;

    if (profile == 0) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    /* TODO(P3): confirm real LCD size, safe area, byte order, TE and flush alignment on hardware. */
    profile->width = 454u;
    profile->height = 454u;
    profile->shape = PET_SCREEN_SHAPE_CIRCLE;
    profile->safe_margin_percent = 12;
    profile->default_scale = 1;
    profile->flush_mode = PET_DISPLAY_FLUSH_MODE_RGB565_RECT;
    profile->rotation = PET_DISPLAY_ROTATION_0;
    profile->rgb565_order = PET_RGB565_ORDER_RGB;
    profile->safe_area.left = 34u;
    profile->safe_area.top = 34u;
    profile->safe_area.right = 420u;
    profile->safe_area.bottom = 420u;
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
    /* P2 must not write to the real LCD or alter the current LVGL flush path. */
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
