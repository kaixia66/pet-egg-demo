#include "pet2d_boundary.h"
#include "pet_platform_jieli.h"

PET_STATIC_ASSERT(pet2d_boundary_result_size,
                  sizeof(pet2d_boundary_enter_placeholder()) == sizeof(pet_result_t));
PET_STATIC_ASSERT(pet2d_boundary_tiny_visual_result_size,
                  sizeof(pet2d_boundary_tiny_visual_probe()) == sizeof(pet_result_t));

pet_result_t pet2d_boundary_compile_check_self_test(void)
{
    pet_result_t ret = pet2d_boundary_self_test();

    if (ret != PET_RESULT_OK) {
        return ret;
    }
    ret = pet2d_boundary_tiny_visual_probe();
    if (ret != PET_RESULT_UNSUPPORTED) {
        return PET_RESULT_ERROR;
    }
    return pet2d_boundary_resource_probe_self_test();
}

const pet_platform_t *pet2d_boundary_compile_check_platform(void)
{
    return pet_platform_jieli_get();
}
