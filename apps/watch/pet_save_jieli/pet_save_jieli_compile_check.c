#include "pet_save_format.h"
#include "pet_save_jieli.h"
#include "pet_save_jieli_syscfg_backend.h"

PET_STATIC_ASSERT(p6_save_slot_header_still_64,
                  sizeof(pet_save_slot_header_t) == PET_SAVE_SLOT_HEADER_SIZE);

pet_result_t pet_save_jieli_compile_check(void)
{
    pet_result_t ret = pet_save_jieli_self_test();

    if (ret != PET_RESULT_OK) {
        return ret;
    }
    return pet_save_jieli_syscfg_self_test();
}
