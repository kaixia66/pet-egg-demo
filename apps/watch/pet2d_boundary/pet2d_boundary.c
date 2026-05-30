#include "pet2d_boundary.h"
#include "pet_platform_jieli_internal.h"

#include <stdio.h>

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
