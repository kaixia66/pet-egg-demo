#include "pet_resource_jieli.h"
#include "pet_resource_jieli_test_blob.h"

typedef struct {
    const pet_u8_t *base;
    pet_u32_t size;
    pet_bool_t opened;
} pet_resource_jieli_state_t;

static pet_resource_jieli_state_t g_pet_resource_jieli_state;

static pet_u16_t pet_resource_jieli_read_u16(const pet_u8_t *p)
{
    return (pet_u16_t)(((pet_u16_t)p[0]) | (((pet_u16_t)p[1]) << 8));
}

static pet_u32_t pet_resource_jieli_read_u32(const pet_u8_t *p)
{
    return ((pet_u32_t)p[0]) |
           (((pet_u32_t)p[1]) << 8) |
           (((pet_u32_t)p[2]) << 16) |
           (((pet_u32_t)p[3]) << 24);
}

static void pet_resource_jieli_decode_header(const pet_u8_t *p,
                                             pet_resource_manifest_header_t *out_header)
{
    out_header->magic = pet_resource_jieli_read_u32(&p[0]);
    out_header->version = pet_resource_jieli_read_u16(&p[4]);
    out_header->entry_count = pet_resource_jieli_read_u16(&p[6]);
    out_header->table_crc32 = pet_resource_jieli_read_u32(&p[8]);
    out_header->reserved = pet_resource_jieli_read_u32(&p[12]);
}

static void pet_resource_jieli_decode_entry(const pet_u8_t *p,
                                            pet_resource_entry_t *out_entry)
{
    out_entry->resource_id = pet_resource_jieli_read_u16(&p[0]);
    out_entry->resource_type = p[2];
    out_entry->format = p[3];
    out_entry->offset = pet_resource_jieli_read_u32(&p[4]);
    out_entry->size = pet_resource_jieli_read_u32(&p[8]);
    out_entry->crc32 = pet_resource_jieli_read_u32(&p[12]);
    out_entry->width = pet_resource_jieli_read_u16(&p[16]);
    out_entry->height = pet_resource_jieli_read_u16(&p[18]);
    out_entry->frame_count = pet_resource_jieli_read_u16(&p[20]);
    out_entry->flags = pet_resource_jieli_read_u16(&p[22]);
    out_entry->reserved = pet_resource_jieli_read_u32(&p[24]);
}

static pet_result_t pet_resource_jieli_get_header(pet_resource_manifest_header_t *out_header)
{
    if (out_header == 0) {
        return PET_RESULT_INVALID_ARGUMENT;
    }
    if ((g_pet_resource_jieli_state.opened != PET_TRUE) ||
        (g_pet_resource_jieli_state.base == 0) ||
        (g_pet_resource_jieli_state.size < PET_RESOURCE_JIELI_HEADER_SIZE)) {
        return PET_RESULT_NOT_READY;
    }

    pet_resource_jieli_decode_header(g_pet_resource_jieli_state.base, out_header);
    return PET_RESULT_OK;
}

static pet_result_t pet_resource_jieli_get_entry_at(pet_u16_t index,
                                                    pet_resource_entry_t *out_entry)
{
    pet_resource_manifest_header_t header;
    const pet_u8_t *entry_ptr;
    pet_u32_t table_offset;
    pet_result_t ret;

    if (out_entry == 0) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    ret = pet_resource_jieli_get_header(&header);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    if (index >= header.entry_count) {
        return PET_RESULT_NOT_FOUND;
    }

    table_offset = PET_RESOURCE_JIELI_HEADER_SIZE +
                   ((pet_u32_t)index * PET_RESOURCE_JIELI_ENTRY_SIZE);
    if ((table_offset > g_pet_resource_jieli_state.size) ||
        (PET_RESOURCE_JIELI_ENTRY_SIZE > g_pet_resource_jieli_state.size - table_offset)) {
        return PET_RESULT_BUFFER_TOO_SMALL;
    }

    entry_ptr = &g_pet_resource_jieli_state.base[table_offset];
    pet_resource_jieli_decode_entry(entry_ptr, out_entry);
    return PET_RESULT_OK;
}

pet_u32_t pet_resource_jieli_crc32(const pet_u8_t *data, pet_u32_t size)
{
    pet_u32_t crc = 0xffffffffu;
    pet_u32_t i;
    pet_u8_t bit;

    if ((data == 0) && (size != 0u)) {
        return 0u;
    }

    for (i = 0u; i < size; ++i) {
        crc ^= data[i];
        for (bit = 0u; bit < 8u; ++bit) {
            if ((crc & 1u) != 0u) {
                crc = (crc >> 1) ^ 0xedb88320u;
            } else {
                crc >>= 1;
            }
        }
    }

    return ~crc;
}

pet_result_t pet_resource_jieli_open_blob(const pet_u8_t *data, pet_u32_t size)
{
    if ((data == 0) || (size == 0u)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }
    if (size < PET_RESOURCE_JIELI_HEADER_SIZE) {
        return PET_RESULT_BUFFER_TOO_SMALL;
    }

    g_pet_resource_jieli_state.base = data;
    g_pet_resource_jieli_state.size = size;
    g_pet_resource_jieli_state.opened = PET_TRUE;
    return PET_RESULT_OK;
}

pet_result_t pet_resource_jieli_validate_manifest(void)
{
    pet_resource_manifest_header_t header;
    pet_resource_entry_t entry;
    pet_u32_t table_offset = PET_RESOURCE_JIELI_HEADER_SIZE;
    pet_u32_t table_size;
    pet_u32_t i;
    pet_result_t ret;

    ret = pet_resource_jieli_get_header(&header);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    if (header.magic != PET_RESOURCE_MANIFEST_MAGIC) {
        return PET_RESULT_BAD_VERSION;
    }
    if (header.version != PET_RESOURCE_FORMAT_VERSION) {
        return PET_RESULT_BAD_VERSION;
    }
    if ((sizeof(pet_resource_manifest_header_t) != PET_RESOURCE_JIELI_HEADER_SIZE) ||
        (sizeof(pet_resource_entry_t) != PET_RESOURCE_JIELI_ENTRY_SIZE)) {
        return PET_RESULT_BAD_VERSION;
    }

    table_size = (pet_u32_t)header.entry_count * PET_RESOURCE_JIELI_ENTRY_SIZE;
    if ((header.entry_count == 0u) ||
        (table_size / PET_RESOURCE_JIELI_ENTRY_SIZE != header.entry_count) ||
        (table_offset > g_pet_resource_jieli_state.size) ||
        (table_size > g_pet_resource_jieli_state.size - table_offset)) {
        return PET_RESULT_BUFFER_TOO_SMALL;
    }
    if (pet_resource_jieli_crc32(&g_pet_resource_jieli_state.base[table_offset],
                                 table_size) != header.table_crc32) {
        return PET_RESULT_BAD_CRC;
    }

    for (i = 0u; i < header.entry_count; ++i) {
        ret = pet_resource_jieli_get_entry_at((pet_u16_t)i, &entry);
        if (ret != PET_RESULT_OK) {
            return ret;
        }
        if ((entry.offset < table_offset + table_size) ||
            (entry.offset > g_pet_resource_jieli_state.size) ||
            (entry.size > g_pet_resource_jieli_state.size - entry.offset)) {
            return PET_RESULT_BUFFER_TOO_SMALL;
        }
        if (pet_resource_jieli_crc32(&g_pet_resource_jieli_state.base[entry.offset],
                                     entry.size) != entry.crc32) {
            return PET_RESULT_BAD_CRC;
        }
    }

    return PET_RESULT_OK;
}

pet_result_t pet_resource_jieli_get_manifest_info(pet_resource_jieli_manifest_info_t *out_info)
{
    pet_resource_manifest_header_t header;
    pet_result_t ret;

    if (out_info == 0) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    ret = pet_resource_jieli_get_header(&header);
    if (ret != PET_RESULT_OK) {
        return ret;
    }

    out_info->version = header.version;
    out_info->entry_count = header.entry_count;
    out_info->header_size = PET_RESOURCE_JIELI_HEADER_SIZE;
    out_info->entry_size = PET_RESOURCE_JIELI_ENTRY_SIZE;
    out_info->table_offset = PET_RESOURCE_JIELI_HEADER_SIZE;
    out_info->table_size = (pet_u32_t)header.entry_count * PET_RESOURCE_JIELI_ENTRY_SIZE;
    out_info->data_offset = out_info->table_offset + out_info->table_size;
    out_info->data_size = (out_info->data_offset <= g_pet_resource_jieli_state.size) ?
                          (g_pet_resource_jieli_state.size - out_info->data_offset) : 0u;
    out_info->table_crc32 = header.table_crc32;
    return PET_RESULT_OK;
}

pet_result_t pet_resource_jieli_find_entry_by_id(pet_u32_t resource_id,
                                                 pet_resource_entry_t *out_entry)
{
    pet_resource_manifest_header_t header;
    pet_resource_entry_t entry;
    pet_u32_t i;
    pet_result_t ret;

    if (out_entry == 0) {
        return PET_RESULT_INVALID_ARGUMENT;
    }
    if (resource_id > 0xffffu) {
        return PET_RESULT_NOT_FOUND;
    }

    ret = pet_resource_jieli_get_header(&header);
    if (ret != PET_RESULT_OK) {
        return ret;
    }

    for (i = 0u; i < header.entry_count; ++i) {
        ret = pet_resource_jieli_get_entry_at((pet_u16_t)i, &entry);
        if (ret != PET_RESULT_OK) {
            return ret;
        }
        if (entry.resource_id == (pet_u16_t)resource_id) {
            *out_entry = entry;
            return PET_RESULT_OK;
        }
    }

    return PET_RESULT_NOT_FOUND;
}

pet_result_t pet_resource_jieli_find_entry_by_type_index(pet_u16_t type, pet_u16_t index,
                                                         pet_resource_entry_t *out_entry)
{
    pet_resource_manifest_header_t header;
    pet_resource_entry_t entry;
    pet_u16_t matched = 0u;
    pet_u32_t i;
    pet_result_t ret;

    if ((out_entry == 0) || (type > 0xffu)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    ret = pet_resource_jieli_get_header(&header);
    if (ret != PET_RESULT_OK) {
        return ret;
    }

    for (i = 0u; i < header.entry_count; ++i) {
        ret = pet_resource_jieli_get_entry_at((pet_u16_t)i, &entry);
        if (ret != PET_RESULT_OK) {
            return ret;
        }
        if (entry.resource_type == (pet_u8_t)type) {
            if (matched == index) {
                *out_entry = entry;
                return PET_RESULT_OK;
            }
            ++matched;
        }
    }

    return PET_RESULT_NOT_FOUND;
}

pet_result_t pet_resource_jieli_read_entry(pet_u32_t resource_id,
                                           const pet_u8_t **out_data,
                                           pet_u32_t *out_size)
{
    pet_resource_entry_t entry;
    pet_result_t ret;

    if ((out_data == 0) || (out_size == 0)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    ret = pet_resource_jieli_find_entry_by_id(resource_id, &entry);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    if ((entry.offset > g_pet_resource_jieli_state.size) ||
        (entry.size > g_pet_resource_jieli_state.size - entry.offset)) {
        return PET_RESULT_BUFFER_TOO_SMALL;
    }
    if (pet_resource_jieli_crc32(&g_pet_resource_jieli_state.base[entry.offset],
                                 entry.size) != entry.crc32) {
        return PET_RESULT_BAD_CRC;
    }

    *out_data = &g_pet_resource_jieli_state.base[entry.offset];
    *out_size = entry.size;
    return PET_RESULT_OK;
}

pet_result_t pet_resource_jieli_self_test(void)
{
    pet_u8_t corrupted_blob[272u];
    pet_resource_jieli_manifest_info_t info;
    pet_resource_entry_t entry;
    const pet_u8_t *data;
    pet_u32_t data_size;
    pet_u32_t i;
    pet_result_t ret;

    if (pet_resource_jieli_test_blob_size > (pet_u32_t)sizeof(corrupted_blob)) {
        return PET_RESULT_BUFFER_TOO_SMALL;
    }

    ret = pet_resource_jieli_open_blob(0, 0u);
    if (ret != PET_RESULT_INVALID_ARGUMENT) {
        return PET_RESULT_ERROR;
    }

    ret = pet_resource_jieli_open_blob(pet_resource_jieli_test_blob,
                                       pet_resource_jieli_test_blob_size);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    ret = pet_resource_jieli_validate_manifest();
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    ret = pet_resource_jieli_get_manifest_info(&info);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    if ((info.version != PET_RESOURCE_FORMAT_VERSION) ||
        (info.header_size != PET_RESOURCE_JIELI_HEADER_SIZE) ||
        (info.entry_size != PET_RESOURCE_JIELI_ENTRY_SIZE) ||
        (info.entry_count != PET_RESOURCE_JIELI_TEST_BLOB_ENTRY_COUNT)) {
        return PET_RESULT_ERROR;
    }

    ret = pet_resource_jieli_find_entry_by_id(PET_RESOURCE_JIELI_TEST_BACKGROUND_ID, &entry);
    if ((ret != PET_RESULT_OK) || (entry.width != 8u) || (entry.height != 8u)) {
        return PET_RESULT_ERROR;
    }
    ret = pet_resource_jieli_find_entry_by_id(PET_RESOURCE_JIELI_TEST_SPRITE_ID, &entry);
    if ((ret != PET_RESULT_OK) || (entry.width != 4u) || (entry.height != 4u)) {
        return PET_RESULT_ERROR;
    }
    ret = pet_resource_jieli_find_entry_by_id(PET_RESOURCE_JIELI_TEST_ANIM_TABLE_ID, &entry);
    if ((ret != PET_RESULT_OK) ||
        (entry.resource_type != PET_RESOURCE_TYPE_ANIMATION) ||
        (entry.format != PET_RESOURCE_FORMAT_ANIMATION_TABLE)) {
        return PET_RESULT_ERROR;
    }
    ret = pet_resource_jieli_find_entry_by_id(9999u, &entry);
    if (ret != PET_RESULT_NOT_FOUND) {
        return PET_RESULT_ERROR;
    }
    ret = pet_resource_jieli_find_entry_by_type_index(PET_RESOURCE_TYPE_SPRITE, 1u, &entry);
    if ((ret != PET_RESULT_OK) || (entry.resource_id != PET_RESOURCE_JIELI_TEST_SPRITE_ID)) {
        return PET_RESULT_ERROR;
    }

    ret = pet_resource_jieli_read_entry(PET_RESOURCE_JIELI_TEST_BACKGROUND_ID, &data, &data_size);
    if ((ret != PET_RESULT_OK) || (data == 0) || (data_size != 128u)) {
        return PET_RESULT_ERROR;
    }

    for (i = 0u; i < pet_resource_jieli_test_blob_size; ++i) {
        corrupted_blob[i] = pet_resource_jieli_test_blob[i];
    }
    corrupted_blob[0] ^= 0xffu;
    ret = pet_resource_jieli_open_blob(corrupted_blob, pet_resource_jieli_test_blob_size);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    ret = pet_resource_jieli_validate_manifest();
    if (ret != PET_RESULT_BAD_VERSION) {
        return PET_RESULT_ERROR;
    }

    for (i = 0u; i < pet_resource_jieli_test_blob_size; ++i) {
        corrupted_blob[i] = pet_resource_jieli_test_blob[i];
    }
    corrupted_blob[100u] ^= 0x01u;
    ret = pet_resource_jieli_open_blob(corrupted_blob, pet_resource_jieli_test_blob_size);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    ret = pet_resource_jieli_validate_manifest();
    if (ret != PET_RESULT_BAD_CRC) {
        return PET_RESULT_ERROR;
    }

    return pet_resource_jieli_open_blob(pet_resource_jieli_test_blob,
                                        pet_resource_jieli_test_blob_size);
}
