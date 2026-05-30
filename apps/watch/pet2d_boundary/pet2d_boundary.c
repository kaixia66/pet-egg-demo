#include "pet2d_boundary.h"
#include "pet2d_minimal_visual.h"
#include "pet_platform_jieli_internal.h"
#include "pet_resource_jieli.h"

extern int printf(const char *format, ...);

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
