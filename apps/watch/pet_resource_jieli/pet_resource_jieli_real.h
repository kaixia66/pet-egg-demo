#ifndef PET_RESOURCE_JIELI_REAL_H
#define PET_RESOURCE_JIELI_REAL_H

#include "pet_resource_jieli.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PET_RESOURCE_JIELI_REAL_SOURCE_NONE       0u
#define PET_RESOURCE_JIELI_REAL_SOURCE_RES_FILE   1u
#define PET_RESOURCE_JIELI_REAL_SOURCE_FLASH_INFO 2u

#define PET_RESOURCE_JIELI_REAL_HEADER_PROBE_SIZE PET_RESOURCE_JIELI_HEADER_SIZE

typedef struct {
    pet_u32_t base_addr;
    pet_u32_t size;
    pet_u32_t crc32;
    pet_u8_t source_type;
} pet_resource_jieli_real_info_t;

pet_result_t pet_resource_jieli_real_probe_info(pet_resource_jieli_real_info_t *out);
pet_result_t pet_resource_jieli_real_read(pet_u32_t offset, pet_u8_t *out, pet_u32_t len);
pet_result_t pet_resource_jieli_real_open_manifest(void);
pet_result_t pet_resource_jieli_real_package_probe(void);
pet_result_t pet_resource_jieli_real_self_test(void);

#ifdef __cplusplus
}
#endif

#endif
