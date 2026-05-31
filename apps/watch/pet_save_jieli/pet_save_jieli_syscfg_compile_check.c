#include "pet_save_jieli_syscfg_backend.h"

PET_STATIC_ASSERT(p21_syscfg_slot_payload_fits_test_payload,
                  PET_SAVE_JIELI_SYSCFG_MAX_PAYLOAD >= 32u);

pet_result_t pet_save_jieli_syscfg_compile_check(void)
{
    pet_save_jieli_syscfg_stats_t stats;
    pet_result_t ret = pet_save_jieli_syscfg_get_last_stats(&stats);

    if ((ret != PET_RESULT_OK) && (ret != PET_RESULT_INVALID_ARGUMENT)) {
        return ret;
    }
    return pet_save_jieli_syscfg_self_test();
}
