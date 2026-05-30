#include "pet_platform.h"
#include "pet_platform_jieli.h"

PET_STATIC_ASSERT(jieli_platform_pointer_size,
                  sizeof(pet_platform_jieli_get()) == sizeof(const pet_platform_t *));

const pet_platform_t *pet_platform_jieli_compile_check_get(void)
{
    return pet_platform_jieli_get();
}
