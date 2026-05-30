#include "pet2d_boundary.h"
#include "pet_display_profile.h"
#include "pet_platform.h"
#include "pet_platform_jieli.h"

PET_STATIC_ASSERT(jieli_platform_pointer_size,
                  sizeof(pet_platform_jieli_get()) == sizeof(const pet_platform_t *));

const pet_platform_t *pet_platform_jieli_compile_check_get(void)
{
    return pet_platform_jieli_get();
}

pet_result_t pet_platform_jieli_compile_check_self_tests(void)
{
    pet_result_t display_ret;
    pet_result_t owner_ret;
    pet_result_t input_ret;
    pet_result_t pet2d_ret;

    display_ret = pet_platform_jieli_display_self_test();
    owner_ret = pet_display_jieli_owner_self_test();
    input_ret = pet_platform_jieli_input_self_test();
    pet2d_ret = pet2d_boundary_self_test();
    if (display_ret != PET_RESULT_OK) {
        return display_ret;
    }
    if (owner_ret != PET_RESULT_OK) {
        return owner_ret;
    }
    if (input_ret != PET_RESULT_OK) {
        return input_ret;
    }
    if (pet2d_ret != PET_RESULT_OK) {
        return pet2d_ret;
    }
    return PET_RESULT_OK;
}
