#include "pet2d_boundary.h"
#include "pet2d_dirty_rect_poc.h"
#include "pet2d_minimal_visual.h"
#include "pet2d_resource_sprite_poc.h"
#include "pet_platform_jieli_internal.h"
#include "pet_resource_jieli.h"

extern int printf(const char *format, ...);

static pet2d_boundary_repeated_flush_stats_t g_pet2d_repeated_flush_stats;
static pet2d_boundary_resource_sprite_stats_t g_pet2d_resource_sprite_stats;

static void pet2d_boundary_record_resource_sprite_result(
    const pet2d_minimal_surface_t *surface,
    const pet2d_resource_sprite_view_t *sprite,
    pet_result_t result)
{
    if (sprite != 0) {
        g_pet2d_resource_sprite_stats.last_resource_id = sprite->resource_id;
        g_pet2d_resource_sprite_stats.last_sprite_w = sprite->width;
        g_pet2d_resource_sprite_stats.last_sprite_h = sprite->height;
    }
    if (surface != 0) {
        g_pet2d_resource_sprite_stats.last_surface_w = surface->width;
        g_pet2d_resource_sprite_stats.last_surface_h = surface->height;
    }
    g_pet2d_resource_sprite_stats.last_result = result;
}

static void pet2d_boundary_record_repeated_result(const pet2d_repeated_flush_config_t *config,
                                                  pet_u16_t size,
                                                  pet_u16_t success_count,
                                                  pet_u16_t fail_index,
                                                  pet_result_t result,
                                                  pet_u32_t total_ms,
                                                  pet_u32_t max_ms)
{
    g_pet2d_repeated_flush_stats.last_pattern_size = size;
    g_pet2d_repeated_flush_stats.last_repeat_count = config->repeat_count;
    g_pet2d_repeated_flush_stats.last_success_count = success_count;
    g_pet2d_repeated_flush_stats.last_fail_index = fail_index;
    g_pet2d_repeated_flush_stats.last_rect_x = config->x;
    g_pet2d_repeated_flush_stats.last_rect_y = config->y;
    g_pet2d_repeated_flush_stats.last_rect_w = size;
    g_pet2d_repeated_flush_stats.last_rect_h = size;
    g_pet2d_repeated_flush_stats.last_result = result;
    g_pet2d_repeated_flush_stats.total_flush_ms = total_ms;
    g_pet2d_repeated_flush_stats.max_single_flush_ms = max_ms;
}

pet_result_t pet2d_boundary_reset_repeated_flush_stats(void)
{
    pet_u8_t *bytes = (pet_u8_t *)&g_pet2d_repeated_flush_stats;
    pet_u32_t i;

    for (i = 0u; i < (pet_u32_t)sizeof(g_pet2d_repeated_flush_stats); i++) {
        bytes[i] = 0u;
    }
    g_pet2d_repeated_flush_stats.last_result = PET_RESULT_NOT_READY;
    g_pet2d_repeated_flush_stats.last_fail_index = 0xFFFFu;
    return PET_RESULT_OK;
}

pet_result_t pet2d_boundary_get_repeated_flush_stats(
    pet2d_boundary_repeated_flush_stats_t *out_stats)
{
    if (out_stats == 0) {
        return PET_RESULT_INVALID_ARGUMENT;
    }
    *out_stats = g_pet2d_repeated_flush_stats;
    return PET_RESULT_OK;
}

pet_result_t pet2d_boundary_reset_resource_sprite_stats(void)
{
    pet_u8_t *bytes = (pet_u8_t *)&g_pet2d_resource_sprite_stats;
    pet_u32_t i;

    for (i = 0u; i < (pet_u32_t)sizeof(g_pet2d_resource_sprite_stats); i++) {
        bytes[i] = 0u;
    }
    g_pet2d_resource_sprite_stats.last_result = PET_RESULT_NOT_READY;
    return PET_RESULT_OK;
}

pet_result_t pet2d_boundary_get_resource_sprite_stats(
    pet2d_boundary_resource_sprite_stats_t *out_stats)
{
    if (out_stats == 0) {
        return PET_RESULT_INVALID_ARGUMENT;
    }
    *out_stats = g_pet2d_resource_sprite_stats;
    return PET_RESULT_OK;
}

pet_result_t pet2d_boundary_enter_placeholder(void)
{
    const pet_platform_t *platform = pet_platform_jieli_get();
    pet_result_t ret;

    if ((platform == 0) || (platform->display_acquire == 0)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    ret = platform->display_acquire(platform->ctx, PET_DISPLAY_OWNER_PET2D, 0u);
    printf("[PET_RENDER_OWNER] pet2d placeholder enter ret=%d owner=%d\n",
           ret, pet_display_jieli_get_owner());
    return ret;
}

pet_result_t pet2d_boundary_exit_placeholder(void)
{
    const pet_platform_t *platform = pet_platform_jieli_get();
    pet_result_t ret;

    if ((platform == 0) || (platform->display_release == 0)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    ret = platform->display_release(platform->ctx, PET_DISPLAY_OWNER_PET2D);
    printf("[PET_RENDER_OWNER] pet2d placeholder exit ret=%d owner=%d\n",
           ret, pet_display_jieli_get_owner());
    return ret;
}

pet_result_t pet2d_boundary_self_test(void)
{
    const pet_platform_t *platform = pet_platform_jieli_get();
    pet_display_owner_t original_owner;
    pet_result_t ret;

    if ((platform == 0) || (platform->display_acquire == 0) || (platform->display_release == 0)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    original_owner = pet_display_jieli_get_owner();
    pet_display_jieli_init();

    ret = platform->display_acquire(platform->ctx, PET_DISPLAY_OWNER_LVGL_SYSTEM_UI, 0u);
    if (ret != PET_RESULT_OK) {
        return ret;
    }

    ret = pet2d_boundary_enter_placeholder();
    if (ret != PET_RESULT_BUSY) {
        (void)platform->display_release(platform->ctx, PET_DISPLAY_OWNER_LVGL_SYSTEM_UI);
        return PET_RESULT_ERROR;
    }

    ret = platform->display_release(platform->ctx, PET_DISPLAY_OWNER_LVGL_SYSTEM_UI);
    if (ret != PET_RESULT_OK) {
        return ret;
    }

    ret = pet2d_boundary_enter_placeholder();
    if (ret != PET_RESULT_OK) {
        return ret;
    }

    ret = pet2d_boundary_exit_placeholder();
    if (ret != PET_RESULT_OK) {
        return ret;
    }

    ret = platform->display_acquire(platform->ctx, PET_DISPLAY_OWNER_LVGL_SYSTEM_UI, 0u);
    if (ret != PET_RESULT_OK) {
        return ret;
    }

    ret = platform->display_release(platform->ctx, PET_DISPLAY_OWNER_LVGL_SYSTEM_UI);
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

pet_result_t pet2d_boundary_resource_probe_self_test(void)
{
    /*
     * P5 probe only: validates manifest lookup plumbing without loading sprites,
     * entering Pet2D runtime, allocating a framebuffer, or calling LCD flush.
     */
    return pet_resource_jieli_self_test();
}

pet_result_t pet2d_boundary_tiny_visual_probe(void)
{
#if PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC
    const pet_platform_t *platform = pet_platform_jieli_get();
    pet_display_owner_t original_owner;
    pet_result_t ret;
    pet_bool_t acquired_here = PET_FALSE;

    if ((platform == 0) || (platform->display_acquire == 0) ||
        (platform->display_release == 0)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    original_owner = pet_display_jieli_get_owner();
    if (original_owner == PET_DISPLAY_OWNER_NONE) {
        ret = platform->display_acquire(platform->ctx, PET_DISPLAY_OWNER_PET2D, 0u);
        if (ret != PET_RESULT_OK) {
            return ret;
        }
        acquired_here = PET_TRUE;
    } else if (original_owner != PET_DISPLAY_OWNER_PET2D) {
        return PET_RESULT_BUSY;
    }

    ret = pet_display_jieli_tiny_flush_poc();
    if (acquired_here == PET_TRUE) {
        (void)platform->display_release(platform->ctx, PET_DISPLAY_OWNER_PET2D);
    }
    return ret;
#else
    return PET_RESULT_UNSUPPORTED;
#endif
}

pet_result_t pet2d_boundary_minimal_real_flush_probe(void)
{
#if PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC
    const pet_platform_t *platform = pet_platform_jieli_get();
    pet_display_owner_t original_owner;
    pet_result_t ret;
    pet_bool_t acquired_here = PET_FALSE;
    static pet_u16_t pixels[PET2D_MINIMAL_VISUAL_WIDTH * PET2D_MINIMAL_VISUAL_HEIGHT];
    pet2d_minimal_surface_t surface;
    int x;
    int y;

    if ((platform == 0) || (platform->display_acquire == 0) ||
        (platform->display_release == 0)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    surface.width = PET2D_MINIMAL_VISUAL_WIDTH;
    surface.height = PET2D_MINIMAL_VISUAL_HEIGHT;
    surface.pitch_pixels = PET2D_MINIMAL_VISUAL_WIDTH;
    surface.pixels = pixels;

    ret = pet2d_minimal_visual_fill_test_pattern(&surface);
    if (ret != PET_RESULT_OK) {
        return ret;
    }

    original_owner = pet_display_jieli_get_owner();
    if (original_owner == PET_DISPLAY_OWNER_NONE) {
        ret = platform->display_acquire(platform->ctx, PET_DISPLAY_OWNER_PET2D, 0u);
        if (ret != PET_RESULT_OK) {
            return ret;
        }
        acquired_here = PET_TRUE;
    } else if (original_owner != PET_DISPLAY_OWNER_PET2D) {
        return PET_RESULT_BUSY;
    }

    x = ((int)PET_JIELI_DISPLAY_WIDTH - (int)PET2D_MINIMAL_VISUAL_WIDTH) / 2;
    y = ((int)PET_JIELI_DISPLAY_HEIGHT - (int)PET2D_MINIMAL_VISUAL_HEIGHT) / 2;
    ret = pet_display_jieli_real_flush_poc_rect(x, y,
                                                PET2D_MINIMAL_VISUAL_WIDTH,
                                                PET2D_MINIMAL_VISUAL_HEIGHT,
                                                pixels,
                                                PET2D_MINIMAL_VISUAL_WIDTH);
    if (acquired_here == PET_TRUE) {
        (void)platform->display_release(platform->ctx, PET_DISPLAY_OWNER_PET2D);
    }
    return ret;
#else
    return PET_RESULT_UNSUPPORTED;
#endif
}

pet_result_t pet2d_boundary_repeated_flush_probe(const pet2d_repeated_flush_config_t *config)
{
#if PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC
    const pet_platform_t *platform = pet_platform_jieli_get();
    pet_display_owner_t original_owner;
    pet_result_t ret;
    pet_bool_t acquired_here = PET_FALSE;
    pet_u16_t size;
    pet_u16_t index;
    pet_u16_t success_count = 0u;
    pet_u16_t fail_index = 0xFFFFu;
    pet_u32_t total_ms = 0u;
    pet_u32_t max_ms = 0u;
    static pet_u16_t pixels[PET2D_DIRTY_RECT_SIZE_64 * PET2D_DIRTY_RECT_SIZE_64];
    pet2d_minimal_surface_t surface;
    pet2d_dirty_rect_case_t rect;

    if ((config == 0) || (platform == 0) || (platform->display_acquire == 0) ||
        (platform->display_release == 0)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    size = pet2d_dirty_rect_poc_pattern_size(config->pattern);
    if ((size == 0u) || (config->repeat_count == 0u) ||
        (config->repeat_count > PET2D_DIRTY_RECT_REPEAT_MAX)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    rect.x = config->x;
    rect.y = config->y;
    rect.width = size;
    rect.height = size;
    ret = pet2d_dirty_rect_poc_rect_in_bounds(&rect);
    if (ret != PET_RESULT_OK) {
        return ret;
    }

    surface.width = size;
    surface.height = size;
    surface.pitch_pixels = size;
    surface.pixels = pixels;
    ret = pet2d_dirty_rect_poc_fill_pattern(config->pattern, &surface);
    if (ret != PET_RESULT_OK) {
        return ret;
    }

    original_owner = pet_display_jieli_get_owner();
    if (original_owner == PET_DISPLAY_OWNER_NONE) {
        ret = platform->display_acquire(platform->ctx, PET_DISPLAY_OWNER_PET2D, 0u);
        if (ret != PET_RESULT_OK) {
            return ret;
        }
        acquired_here = PET_TRUE;
    } else if (original_owner != PET_DISPLAY_OWNER_PET2D) {
        g_pet2d_repeated_flush_stats.repeated_probe_fail_count++;
        pet2d_boundary_record_repeated_result(config, size, 0u, 0u, PET_RESULT_BUSY, 0u, 0u);
        return PET_RESULT_BUSY;
    }

    g_pet2d_repeated_flush_stats.repeated_probe_attempt_count++;
    for (index = 0u; index < config->repeat_count; index++) {
        pet_u32_t start_ms = 0u;
        pet_u32_t end_ms = 0u;
        pet_u32_t elapsed_ms = 0u;
        int x = (int)config->x;
        int y = (int)config->y;

        if (config->move_each_flush != 0u) {
            x += (int)(index & 3u);
            y += (int)((index >> 1u) & 3u);
        }
        rect.x = (pet_i16_t)x;
        rect.y = (pet_i16_t)y;
        rect.width = size;
        rect.height = size;
        ret = pet2d_dirty_rect_poc_rect_in_bounds(&rect);
        if (ret != PET_RESULT_OK) {
            fail_index = index;
            break;
        }

        if (platform->millis != 0) {
            start_ms = platform->millis(platform->ctx);
        }
        ret = pet_display_jieli_real_flush_poc_rect(x, y, size, size, pixels, size);
        if (platform->millis != 0) {
            end_ms = platform->millis(platform->ctx);
            elapsed_ms = end_ms - start_ms;
            total_ms += elapsed_ms;
            if (elapsed_ms > max_ms) {
                max_ms = elapsed_ms;
            }
        }
        if (ret != PET_RESULT_OK) {
            fail_index = index;
            break;
        }
        success_count++;
        (void)config->delay_ms;
    }

    if (ret == PET_RESULT_OK) {
        g_pet2d_repeated_flush_stats.repeated_probe_success_count++;
    } else {
        g_pet2d_repeated_flush_stats.repeated_probe_fail_count++;
    }
    pet2d_boundary_record_repeated_result(config, size, success_count, fail_index,
                                          ret, total_ms, max_ms);

    if (acquired_here == PET_TRUE) {
        (void)platform->display_release(platform->ctx, PET_DISPLAY_OWNER_PET2D);
    }
    return ret;
#else
    (void)config;
    return PET_RESULT_UNSUPPORTED;
#endif
}

pet_result_t pet2d_boundary_repeated_flush_default_probe(void)
{
    pet2d_repeated_flush_config_t config;
    pet2d_dirty_rect_case_t rect;
    pet_result_t ret;

    ret = pet2d_dirty_rect_poc_rect_for_case(PET2D_DIRTY_RECT_PATTERN_16, 0u, &rect);
    if (ret != PET_RESULT_OK) {
        return ret;
    }

    config.pattern = PET2D_DIRTY_RECT_PATTERN_16;
    config.repeat_count = PET2D_DIRTY_RECT_DEFAULT_REPEAT;
    config.x = rect.x;
    config.y = rect.y;
    config.delay_ms = 50u;
    config.move_each_flush = 0u;
    return pet2d_boundary_repeated_flush_probe(&config);
}

pet_result_t pet2d_boundary_repeated_flush_gate_self_test(void)
{
    pet2d_boundary_repeated_flush_stats_t stats;
    pet2d_repeated_flush_config_t config;
    pet2d_dirty_rect_case_t rect;
    pet_result_t ret;

    ret = pet2d_dirty_rect_poc_self_test();
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    if (pet2d_boundary_reset_repeated_flush_stats() != PET_RESULT_OK) {
        return PET_RESULT_ERROR;
    }
    if (pet2d_boundary_get_repeated_flush_stats(&stats) != PET_RESULT_OK) {
        return PET_RESULT_ERROR;
    }
    if (stats.repeated_probe_attempt_count != 0u) {
        return PET_RESULT_ERROR;
    }

    ret = pet2d_dirty_rect_poc_rect_for_case(PET2D_DIRTY_RECT_PATTERN_32, 0u, &rect);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    config.pattern = PET2D_DIRTY_RECT_PATTERN_32;
    config.repeat_count = PET2D_DIRTY_RECT_DEFAULT_REPEAT;
    config.x = rect.x;
    config.y = rect.y;
    config.delay_ms = 50u;
    config.move_each_flush = 0u;

    ret = pet2d_boundary_repeated_flush_probe(&config);
#if PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC
    if ((ret != PET_RESULT_OK) && (ret != PET_RESULT_BUSY)) {
        return ret;
    }
    return PET_RESULT_UNSUPPORTED;
#else
    if (ret != PET_RESULT_UNSUPPORTED) {
        return PET_RESULT_ERROR;
    }
    if (pet2d_boundary_get_repeated_flush_stats(&stats) != PET_RESULT_OK) {
        return PET_RESULT_ERROR;
    }
    if (stats.repeated_probe_attempt_count != 0u) {
        return PET_RESULT_ERROR;
    }
    return PET_RESULT_UNSUPPORTED;
#endif
}

pet_result_t pet2d_boundary_resource_sprite_flush_probe(void)
{
#if PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC
    const pet_platform_t *platform = pet_platform_jieli_get();
    pet_display_owner_t original_owner;
    pet_result_t ret;
    pet_bool_t acquired_here = PET_FALSE;
    static pet_u16_t pixels[PET2D_DIRTY_RECT_SIZE_32 * PET2D_DIRTY_RECT_SIZE_32];
    pet2d_minimal_surface_t surface;
    pet2d_resource_sprite_view_t sprite;
    pet2d_resource_sprite_view_t background;
    int x;
    int y;

    if ((platform == 0) || (platform->display_acquire == 0) ||
        (platform->display_release == 0)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    surface.width = PET2D_DIRTY_RECT_SIZE_32;
    surface.height = PET2D_DIRTY_RECT_SIZE_32;
    surface.pitch_pixels = PET2D_DIRTY_RECT_SIZE_32;
    surface.pixels = pixels;

    ret = pet2d_dirty_rect_poc_fill_pattern(PET2D_DIRTY_RECT_PATTERN_32, &surface);
    if (ret != PET_RESULT_OK) {
        pet2d_boundary_record_resource_sprite_result(&surface, 0, ret);
        return ret;
    }
    ret = pet2d_resource_sprite_poc_open(PET2D_RESOURCE_SPRITE_POC_ID_8X8, &background);
    if (ret != PET_RESULT_OK) {
        pet2d_boundary_record_resource_sprite_result(&surface, 0, ret);
        return ret;
    }
    ret = pet2d_minimal_visual_blit_sprite(&surface, 4, 4, &background);
    if (ret != PET_RESULT_OK) {
        pet2d_boundary_record_resource_sprite_result(&surface, &background, ret);
        return ret;
    }
    ret = pet2d_resource_sprite_poc_open(PET2D_RESOURCE_SPRITE_POC_ID_4X4, &sprite);
    if (ret != PET_RESULT_OK) {
        pet2d_boundary_record_resource_sprite_result(&surface, &background, ret);
        return ret;
    }
    ret = pet2d_minimal_visual_blit_sprite(&surface,
                                           ((int)surface.width - (int)sprite.width) / 2,
                                           ((int)surface.height - (int)sprite.height) / 2,
                                           &sprite);
    if (ret != PET_RESULT_OK) {
        pet2d_boundary_record_resource_sprite_result(&surface, &sprite, ret);
        return ret;
    }

    original_owner = pet_display_jieli_get_owner();
    if (original_owner == PET_DISPLAY_OWNER_NONE) {
        ret = platform->display_acquire(platform->ctx, PET_DISPLAY_OWNER_PET2D, 0u);
        if (ret != PET_RESULT_OK) {
            pet2d_boundary_record_resource_sprite_result(&surface, &sprite, ret);
            return ret;
        }
        acquired_here = PET_TRUE;
    } else if (original_owner != PET_DISPLAY_OWNER_PET2D) {
        g_pet2d_resource_sprite_stats.resource_sprite_probe_fail_count++;
        pet2d_boundary_record_resource_sprite_result(&surface, &sprite, PET_RESULT_BUSY);
        return PET_RESULT_BUSY;
    }

    g_pet2d_resource_sprite_stats.resource_sprite_probe_attempt_count++;
    x = ((int)PET_JIELI_DISPLAY_WIDTH - (int)surface.width) / 2;
    y = ((int)PET_JIELI_DISPLAY_HEIGHT - (int)surface.height) / 2;
    ret = pet_display_jieli_real_flush_poc_rect(x, y,
                                                surface.width,
                                                surface.height,
                                                surface.pixels,
                                                surface.pitch_pixels);
    if (ret == PET_RESULT_OK) {
        g_pet2d_resource_sprite_stats.resource_sprite_probe_success_count++;
    } else {
        g_pet2d_resource_sprite_stats.resource_sprite_probe_fail_count++;
    }
    pet2d_boundary_record_resource_sprite_result(&surface, &sprite, ret);

    if (acquired_here == PET_TRUE) {
        (void)platform->display_release(platform->ctx, PET_DISPLAY_OWNER_PET2D);
    }
    return ret;
#else
    return PET_RESULT_UNSUPPORTED;
#endif
}

pet_result_t pet2d_boundary_resource_sprite_gate_self_test(void)
{
    pet2d_boundary_resource_sprite_stats_t stats;
    pet_result_t ret;

    ret = pet2d_resource_sprite_poc_self_test();
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    if (pet2d_boundary_reset_resource_sprite_stats() != PET_RESULT_OK) {
        return PET_RESULT_ERROR;
    }
    if (pet2d_boundary_get_resource_sprite_stats(&stats) != PET_RESULT_OK) {
        return PET_RESULT_ERROR;
    }
    if (stats.resource_sprite_probe_attempt_count != 0u) {
        return PET_RESULT_ERROR;
    }

    ret = pet2d_boundary_resource_sprite_flush_probe();
#if PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC
    if ((ret != PET_RESULT_OK) && (ret != PET_RESULT_BUSY)) {
        return ret;
    }
    return PET_RESULT_UNSUPPORTED;
#else
    if (ret != PET_RESULT_UNSUPPORTED) {
        return PET_RESULT_ERROR;
    }
    if (pet2d_boundary_get_resource_sprite_stats(&stats) != PET_RESULT_OK) {
        return PET_RESULT_ERROR;
    }
    if (stats.resource_sprite_probe_attempt_count != 0u) {
        return PET_RESULT_ERROR;
    }
    return PET_RESULT_UNSUPPORTED;
#endif
}
