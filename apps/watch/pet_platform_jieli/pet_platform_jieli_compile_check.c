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
    pet_result_t input_ret;

    display_ret = pet_platform_jieli_display_self_test();
    input_ret = pet_platform_jieli_input_self_test();
    if (display_ret != PET_RESULT_OK) {
        return display_ret;
    }
    return input_ret;
}
