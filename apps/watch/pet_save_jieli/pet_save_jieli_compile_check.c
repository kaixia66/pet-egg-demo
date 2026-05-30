#include "pet_save_format.h"
#include "pet_save_jieli.h"

PET_STATIC_ASSERT(p6_save_slot_header_still_64,
                  sizeof(pet_save_slot_header_t) == PET_SAVE_SLOT_HEADER_SIZE);

pet_result_t pet_save_jieli_compile_check(void)
{
    return pet_save_jieli_self_test();
}
