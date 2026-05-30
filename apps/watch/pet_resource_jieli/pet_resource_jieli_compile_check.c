#include "pet_resource_format.h"
#include "pet_resource_jieli.h"

PET_STATIC_ASSERT(jieli_resource_entry_size,
                  sizeof(pet_resource_entry_t) == PET_RESOURCE_JIELI_ENTRY_SIZE);
PET_STATIC_ASSERT(jieli_resource_header_size,
                  sizeof(pet_resource_manifest_header_t) == PET_RESOURCE_JIELI_HEADER_SIZE);

pet_result_t pet_resource_jieli_compile_check_self_test(void)
{
    return pet_resource_jieli_self_test();
}

pet_result_t pet_resource_jieli_compile_check_lookup(pet_resource_entry_t *out_entry)
{
    return pet_resource_jieli_find_entry_by_id(1001u, out_entry);
}
