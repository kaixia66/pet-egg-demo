#include "pet_display_profile.h"
#include "pet_platform.h"
#include "pet_platform_jieli.h"
#include "pet_protocol_jieli.h"

pet_result_t pet2d_boundary_self_test(void);

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
    pet_result_t protocol_ret;
    pet_result_t ble_ret;
    pet_result_t nfc_ret;
    pet_result_t debug_ret;

    display_ret = pet_platform_jieli_display_self_test();
    owner_ret = pet_display_jieli_owner_self_test();
    input_ret = pet_platform_jieli_input_self_test();
    pet2d_ret = pet2d_boundary_self_test();
    protocol_ret = pet_protocol_jieli_self_test();
    ble_ret = pet_ble_jieli_self_test();
    nfc_ret = pet_nfc_jieli_self_test();
    debug_ret = pet_debug_jieli_self_test();
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
    if (protocol_ret != PET_RESULT_OK) {
        return protocol_ret;
    }
    if (ble_ret != PET_RESULT_OK) {
        return ble_ret;
    }
    if (nfc_ret != PET_RESULT_OK) {
        return nfc_ret;
    }
    if (debug_ret != PET_RESULT_OK) {
        return debug_ret;
    }
    return PET_RESULT_OK;
}
