#include "pet_platform_jieli_internal.h"

void pet_storage_jieli_init(void)
{
}

pet_result_t pet_storage_jieli_read(void *ctx, pet_storage_area_t area, pet_u32_t offset,
                                    void *dst, pet_u32_t len)
{
    (void)ctx;
    (void)area;
    (void)offset;
    (void)dst;
    (void)len;
    /* TODO(P6): map portable A/B save bytes to confirmed Jieli VM/Flash ownership. */
    return PET_RESULT_NOT_READY;
}

pet_result_t pet_storage_jieli_write_atomic(void *ctx, pet_storage_area_t area, pet_u32_t offset,
                                            const void *src, pet_u32_t len)
{
    (void)ctx;
    (void)area;
    (void)offset;
    (void)src;
    (void)len;
    /* P2 must not write VM, Flash or file-system storage. */
    return PET_RESULT_UNSUPPORTED;
}
