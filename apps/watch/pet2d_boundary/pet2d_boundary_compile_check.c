#include "pet2d_boundary.h"
#include "pet2d_dirty_rect_poc.h"
#include "pet2d_minimal_visual.h"
#include "pet2d_movement_poc.h"
#include "pet2d_resource_sprite_poc.h"
#include "pet_platform_jieli.h"

PET_STATIC_ASSERT(pet2d_boundary_result_size,
                  sizeof(pet2d_boundary_enter_placeholder()) == sizeof(pet_result_t));
PET_STATIC_ASSERT(pet2d_boundary_tiny_visual_result_size,
                  sizeof(pet2d_boundary_tiny_visual_probe()) == sizeof(pet_result_t));
PET_STATIC_ASSERT(pet2d_minimal_visual_surface_size,
                  sizeof(pet2d_minimal_surface_t) >= (sizeof(pet_u16_t) * 4u));
PET_STATIC_ASSERT(pet2d_dirty_rect_repeat_limit,
                  PET2D_DIRTY_RECT_REPEAT_MAX <= 60u);
PET_STATIC_ASSERT(pet2d_resource_sprite_view_has_id,
                  sizeof(pet2d_resource_sprite_view_t) >= (sizeof(pet_u16_t) * 4u));
PET_STATIC_ASSERT(pet2d_movement_surface_small,
                  PET2D_MOVEMENT_POC_SURFACE_SIZE <= PET2D_DIRTY_RECT_SIZE_64);
PET_STATIC_ASSERT(pet2d_movement_repeat_limit,
                  PET2D_MOVEMENT_POC_REPEAT_MAX <= PET2D_DIRTY_RECT_REPEAT_MAX);

pet_result_t pet2d_boundary_compile_check_self_test(void)
{
    pet_result_t ret = pet2d_boundary_self_test();

    if (ret != PET_RESULT_OK) {
        return ret;
    }
    ret = pet2d_minimal_visual_self_test();
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    ret = pet2d_boundary_tiny_visual_probe();
    if (ret != PET_RESULT_UNSUPPORTED) {
        return PET_RESULT_ERROR;
    }
    ret = pet2d_boundary_minimal_real_flush_probe();
    if (ret != PET_RESULT_UNSUPPORTED) {
        return PET_RESULT_ERROR;
    }
    ret = pet2d_dirty_rect_poc_self_test();
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    ret = pet2d_boundary_repeated_flush_default_probe();
    if (ret != PET_RESULT_UNSUPPORTED) {
        return PET_RESULT_ERROR;
    }
    ret = pet2d_boundary_repeated_flush_gate_self_test();
    if (ret != PET_RESULT_UNSUPPORTED) {
        return PET_RESULT_ERROR;
    }
    ret = pet2d_resource_sprite_poc_self_test();
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    ret = pet2d_movement_poc_self_test();
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    ret = pet2d_boundary_movement_probe_step(PET_KEY_LEFT_UP);
    if (ret != PET_RESULT_UNSUPPORTED) {
        return PET_RESULT_ERROR;
    }
    ret = pet2d_boundary_movement_repeated_probe(PET2D_MOVEMENT_POC_DEFAULT_REPEAT, 0u);
    if (ret != PET_RESULT_UNSUPPORTED) {
        return PET_RESULT_ERROR;
    }
    ret = pet2d_boundary_resource_sprite_flush_probe();
    if (ret != PET_RESULT_UNSUPPORTED) {
        return PET_RESULT_ERROR;
    }
    ret = pet2d_boundary_resource_sprite_gate_self_test();
    if (ret != PET_RESULT_UNSUPPORTED) {
        return PET_RESULT_ERROR;
    }
    return pet2d_boundary_resource_probe_self_test();
}

const pet_platform_t *pet2d_boundary_compile_check_platform(void)
{
    return pet_platform_jieli_get();
}
