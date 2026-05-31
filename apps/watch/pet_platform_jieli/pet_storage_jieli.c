#include "pet_platform_jieli_internal.h"
#include "pet_save_jieli_syscfg_backend.h"

void pet_storage_jieli_init(void)
{
}

pet_result_t pet_storage_jieli_read(void *ctx, pet_storage_area_t area, pet_u32_t offset,
                                    void *dst, pet_u32_t len)
{
    pet_u32_t out_len = 0u;
    pet_u64_t counter = 0u;
    pet_result_t ret;

    (void)ctx;

    if (area != PET_STORAGE_AREA_SAVE) {
        return PET_RESULT_UNSUPPORTED;
    }
    if (offset != 0u) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    ret = pet_save_jieli_syscfg_load_latest((pet_u8_t *)dst, len, &out_len, &counter);
    (void)out_len;
    (void)counter;
    return ret;
}

pet_result_t pet_storage_jieli_write_atomic(void *ctx, pet_storage_area_t area, pet_u32_t offset,
                                            const void *src, pet_u32_t len)
{
    (void)ctx;

    if (area != PET_STORAGE_AREA_SAVE) {
        return PET_RESULT_UNSUPPORTED;
    }
    if (offset != 0u) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    return pet_save_jieli_syscfg_write_transaction((const pet_u8_t *)src, len, 0);
}
