#include "pet_resource_jieli_real.h"

#ifndef PET_RESOURCE_JIELI_REAL_MANIFEST_PATH
#define PET_RESOURCE_JIELI_REAL_MANIFEST_PATH "storage/virfat_flash/C/petegg/manifest.bin"
#endif

#define PET_RESOURCE_JIELI_REAL_OUT_OF_RANGE PET_RESULT_INVALID_ARGUMENT

#ifndef SEEK_SET
#define SEEK_SET 0
#endif

typedef struct resfile RESFILE;
extern RESFILE *res_fopen(const char *path, const char *mode);
extern int res_fread(RESFILE *file, void *buf, pet_u32_t len);
extern int res_fseek(RESFILE *file, int offset, int fromwhere);
extern int res_flen(RESFILE *file);
extern int res_fclose(RESFILE *file);

typedef struct {
    pet_bool_t probed;
    pet_result_t last_result;
    pet_resource_jieli_real_info_t info;
    const char *path;
} pet_resource_jieli_real_state_t;

static pet_resource_jieli_real_state_t g_pet_resource_jieli_real_state;

static pet_u16_t pet_resource_jieli_real_read_u16(const pet_u8_t *p)
{
    return (pet_u16_t)(((pet_u16_t)p[0]) | (((pet_u16_t)p[1]) << 8));
}

static pet_u32_t pet_resource_jieli_real_read_u32(const pet_u8_t *p)
{
    return ((pet_u32_t)p[0]) |
           (((pet_u32_t)p[1]) << 8) |
           (((pet_u32_t)p[2]) << 16) |
           (((pet_u32_t)p[3]) << 24);
}

static void pet_resource_jieli_real_decode_header(const pet_u8_t *p,
                                                  pet_resource_manifest_header_t *out_header)
{
    out_header->magic = pet_resource_jieli_real_read_u32(&p[0]);
    out_header->version = pet_resource_jieli_real_read_u16(&p[4]);
    out_header->entry_count = pet_resource_jieli_real_read_u16(&p[6]);
    out_header->table_crc32 = pet_resource_jieli_real_read_u32(&p[8]);
    out_header->reserved = pet_resource_jieli_real_read_u32(&p[12]);
}

static pet_result_t pet_resource_jieli_real_try_open(RESFILE **out_file, pet_u32_t *out_size)
{
    RESFILE *file;
    int file_len;

    if ((out_file == 0) || (out_size == 0)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    file = res_fopen(PET_RESOURCE_JIELI_REAL_MANIFEST_PATH, "r");
    if (file == 0) {
        return PET_RESULT_NOT_FOUND;
    }

    file_len = res_flen(file);
    if (file_len <= 0) {
        (void)res_fclose(file);
        return PET_RESULT_NOT_FOUND;
    }

    *out_file = file;
    *out_size = (pet_u32_t)file_len;
    return PET_RESULT_OK;
}

static pet_result_t pet_resource_jieli_real_read_manifest_header(
    pet_resource_manifest_header_t *out_header,
    pet_resource_jieli_real_info_t *out_info)
{
    RESFILE *file = 0;
    pet_u8_t header_bytes[PET_RESOURCE_JIELI_REAL_HEADER_PROBE_SIZE];
    pet_u32_t file_size = 0u;
    pet_u32_t crc32;
    int read_len;
    pet_result_t ret;

    if (out_header == 0) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    ret = pet_resource_jieli_real_try_open(&file, &file_size);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    if (file_size < PET_RESOURCE_JIELI_REAL_HEADER_PROBE_SIZE) {
        (void)res_fclose(file);
        return PET_RESULT_BUFFER_TOO_SMALL;
    }

    (void)res_fseek(file, 0, SEEK_SET);
    read_len = res_fread(file, header_bytes, PET_RESOURCE_JIELI_REAL_HEADER_PROBE_SIZE);
    (void)res_fclose(file);
    if (read_len != (int)PET_RESOURCE_JIELI_REAL_HEADER_PROBE_SIZE) {
        return PET_RESULT_BUFFER_TOO_SMALL;
    }

    pet_resource_jieli_real_decode_header(header_bytes, out_header);
    crc32 = pet_resource_jieli_crc32(header_bytes, PET_RESOURCE_JIELI_REAL_HEADER_PROBE_SIZE);
    if (out_info != 0) {
        out_info->base_addr = 0u;
        out_info->size = file_size;
        out_info->crc32 = crc32;
        out_info->source_type = PET_RESOURCE_JIELI_REAL_SOURCE_RES_FILE;
    }
    return PET_RESULT_OK;
}

pet_result_t pet_resource_jieli_real_probe_info(pet_resource_jieli_real_info_t *out)
{
    pet_resource_manifest_header_t header;
    pet_resource_jieli_real_info_t info;
    pet_result_t ret;

    if (out == 0) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    ret = pet_resource_jieli_real_read_manifest_header(&header, &info);
    if (ret != PET_RESULT_OK) {
        g_pet_resource_jieli_real_state.probed = PET_TRUE;
        g_pet_resource_jieli_real_state.last_result = ret;
        g_pet_resource_jieli_real_state.info.base_addr = 0u;
        g_pet_resource_jieli_real_state.info.size = 0u;
        g_pet_resource_jieli_real_state.info.crc32 = 0u;
        g_pet_resource_jieli_real_state.info.source_type =
            PET_RESOURCE_JIELI_REAL_SOURCE_NONE;
        g_pet_resource_jieli_real_state.path = PET_RESOURCE_JIELI_REAL_MANIFEST_PATH;
        return ret;
    }
    (void)header;

    g_pet_resource_jieli_real_state.probed = PET_TRUE;
    g_pet_resource_jieli_real_state.last_result = PET_RESULT_OK;
    g_pet_resource_jieli_real_state.info = info;
    g_pet_resource_jieli_real_state.path = PET_RESOURCE_JIELI_REAL_MANIFEST_PATH;
    *out = info;
    return PET_RESULT_OK;
}

pet_result_t pet_resource_jieli_real_read(pet_u32_t offset, pet_u8_t *out, pet_u32_t len)
{
    RESFILE *file = 0;
    pet_u32_t file_size = 0u;
    int read_len;
    pet_result_t ret;

    if ((out == 0) && (len != 0u)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }
    if (len == 0u) {
        return PET_RESULT_OK;
    }

    ret = pet_resource_jieli_real_try_open(&file, &file_size);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    if ((offset > file_size) || (len > file_size - offset) ||
        (offset > (pet_u32_t)0x7fffffffu) || (len > (pet_u32_t)0x7fffffffu)) {
        (void)res_fclose(file);
        return PET_RESOURCE_JIELI_REAL_OUT_OF_RANGE;
    }

    (void)res_fseek(file, (int)offset, SEEK_SET);
    read_len = res_fread(file, out, len);
    (void)res_fclose(file);
    if (read_len != (int)len) {
        return PET_RESULT_BUFFER_TOO_SMALL;
    }

    return PET_RESULT_OK;
}

pet_result_t pet_resource_jieli_real_open_manifest(void)
{
    pet_resource_jieli_real_info_t info;

    return pet_resource_jieli_real_probe_info(&info);
}

pet_result_t pet_resource_jieli_real_package_probe(void)
{
    pet_resource_manifest_header_t header;
    pet_resource_jieli_real_info_t info;
    pet_u32_t table_size;
    pet_result_t ret;

    ret = pet_resource_jieli_real_read_manifest_header(&header, &info);
    if (ret != PET_RESULT_OK) {
        g_pet_resource_jieli_real_state.probed = PET_TRUE;
        g_pet_resource_jieli_real_state.last_result = ret;
        g_pet_resource_jieli_real_state.path = PET_RESOURCE_JIELI_REAL_MANIFEST_PATH;
        return ret;
    }

    g_pet_resource_jieli_real_state.probed = PET_TRUE;
    g_pet_resource_jieli_real_state.info = info;
    g_pet_resource_jieli_real_state.path = PET_RESOURCE_JIELI_REAL_MANIFEST_PATH;

    if ((header.magic != PET_RESOURCE_MANIFEST_MAGIC) ||
        (header.version != PET_RESOURCE_FORMAT_VERSION)) {
        g_pet_resource_jieli_real_state.last_result = PET_RESULT_BAD_VERSION;
        return PET_RESULT_BAD_VERSION;
    }
    if (header.entry_count == 0u) {
        g_pet_resource_jieli_real_state.last_result = PET_RESULT_NOT_FOUND;
        return PET_RESULT_NOT_FOUND;
    }

    table_size = (pet_u32_t)header.entry_count * PET_RESOURCE_JIELI_ENTRY_SIZE;
    if ((table_size / PET_RESOURCE_JIELI_ENTRY_SIZE != header.entry_count) ||
        (table_size > info.size - PET_RESOURCE_JIELI_HEADER_SIZE)) {
        g_pet_resource_jieli_real_state.last_result = PET_RESULT_BUFFER_TOO_SMALL;
        return PET_RESULT_BUFFER_TOO_SMALL;
    }

    g_pet_resource_jieli_real_state.last_result = PET_RESULT_OK;
    return PET_RESULT_OK;
}

pet_result_t pet_resource_jieli_real_self_test(void)
{
    pet_resource_jieli_real_info_t info;
    pet_u8_t byte;
    pet_result_t ret;

    if (pet_resource_jieli_real_probe_info(0) != PET_RESULT_INVALID_ARGUMENT) {
        return PET_RESULT_ERROR;
    }
    if (pet_resource_jieli_real_read(0u, 0, 1u) != PET_RESULT_INVALID_ARGUMENT) {
        return PET_RESULT_ERROR;
    }
    if (pet_resource_jieli_real_read(0u, &byte, 0u) != PET_RESULT_OK) {
        return PET_RESULT_ERROR;
    }

    ret = pet_resource_jieli_real_probe_info(&info);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    if ((info.size < PET_RESOURCE_JIELI_HEADER_SIZE) ||
        (info.source_type != PET_RESOURCE_JIELI_REAL_SOURCE_RES_FILE)) {
        return PET_RESULT_ERROR;
    }

    ret = pet_resource_jieli_real_read(info.size, &byte, 1u);
    if (ret != PET_RESOURCE_JIELI_REAL_OUT_OF_RANGE) {
        return PET_RESULT_ERROR;
    }

    return pet_resource_jieli_real_package_probe();
}
