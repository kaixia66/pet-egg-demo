#ifndef PET_RESOURCE_JIELI_H
#define PET_RESOURCE_JIELI_H

#include "pet_resource_format.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PET_RESOURCE_JIELI_HEADER_SIZE 16u
#define PET_RESOURCE_JIELI_ENTRY_SIZE  28u

typedef struct {
    const pet_u8_t *base;
    pet_u32_t size;
} pet_resource_jieli_blob_t;

typedef struct {
    pet_u16_t version;
    pet_u16_t entry_count;
    pet_u32_t header_size;
    pet_u32_t entry_size;
    pet_u32_t table_offset;
    pet_u32_t table_size;
    pet_u32_t data_offset;
    pet_u32_t data_size;
    pet_u32_t table_crc32;
} pet_resource_jieli_manifest_info_t;

pet_result_t pet_resource_jieli_open_blob(const pet_u8_t *data, pet_u32_t size);
pet_result_t pet_resource_jieli_validate_manifest(void);
pet_result_t pet_resource_jieli_get_manifest_info(pet_resource_jieli_manifest_info_t *out_info);
pet_result_t pet_resource_jieli_find_entry_by_id(pet_u32_t resource_id,
                                                 pet_resource_entry_t *out_entry);
pet_result_t pet_resource_jieli_find_entry_by_type_index(pet_u16_t type, pet_u16_t index,
                                                         pet_resource_entry_t *out_entry);
pet_result_t pet_resource_jieli_read_entry(pet_u32_t resource_id,
                                           const pet_u8_t **out_data,
                                           pet_u32_t *out_size);
pet_u32_t pet_resource_jieli_crc32(const pet_u8_t *data, pet_u32_t size);
pet_result_t pet_resource_jieli_self_test(void);

#ifdef __cplusplus
}
#endif

#endif
