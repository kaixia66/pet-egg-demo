#include "pet_save_jieli.h"

#define PET_SAVE_JIELI_TEST_SLOT_CAPACITY 160u

static pet_save_jieli_memory_backend_t *g_pet_save_jieli_backend;

static pet_u16_t pet_save_jieli_read_u16(const pet_u8_t *bytes)
{
    return (pet_u16_t)(((pet_u16_t)bytes[0]) | ((pet_u16_t)bytes[1] << 8));
}

static pet_u32_t pet_save_jieli_read_u32(const pet_u8_t *bytes)
{
    return ((pet_u32_t)bytes[0]) |
           ((pet_u32_t)bytes[1] << 8) |
           ((pet_u32_t)bytes[2] << 16) |
           ((pet_u32_t)bytes[3] << 24);
}

static pet_u64_t pet_save_jieli_read_u64(const pet_u8_t *bytes)
{
    return ((pet_u64_t)pet_save_jieli_read_u32(bytes)) |
           ((pet_u64_t)pet_save_jieli_read_u32(bytes + 4u) << 32);
}

static void pet_save_jieli_write_u16(pet_u8_t *bytes, pet_u16_t value)
{
    bytes[0] = (pet_u8_t)(value & 0xffu);
    bytes[1] = (pet_u8_t)((value >> 8) & 0xffu);
}

static void pet_save_jieli_write_u32(pet_u8_t *bytes, pet_u32_t value)
{
    bytes[0] = (pet_u8_t)(value & 0xffu);
    bytes[1] = (pet_u8_t)((value >> 8) & 0xffu);
    bytes[2] = (pet_u8_t)((value >> 16) & 0xffu);
    bytes[3] = (pet_u8_t)((value >> 24) & 0xffu);
}

static void pet_save_jieli_write_u64(pet_u8_t *bytes, pet_u64_t value)
{
    pet_save_jieli_write_u32(bytes, (pet_u32_t)(value & 0xffffffffu));
    pet_save_jieli_write_u32(bytes + 4u, (pet_u32_t)((value >> 32) & 0xffffffffu));
}

static void pet_save_jieli_copy_bytes(pet_u8_t *dst, const pet_u8_t *src, pet_u32_t len)
{
    pet_u32_t i;

    for (i = 0u; i < len; ++i) {
        dst[i] = src[i];
    }
}

static void pet_save_jieli_zero_bytes(pet_u8_t *dst, pet_u32_t len)
{
    pet_u32_t i;

    for (i = 0u; i < len; ++i) {
        dst[i] = 0u;
    }
}

static pet_bool_t pet_save_jieli_slot_is_empty(const pet_u8_t *slot, pet_u32_t slot_capacity)
{
    pet_u32_t i;
    pet_bool_t all_zero = PET_TRUE;
    pet_bool_t all_ff = PET_TRUE;

    if ((slot == 0) || (slot_capacity == 0u)) {
        return PET_FALSE;
    }

    for (i = 0u; i < slot_capacity; ++i) {
        if (slot[i] != 0u) {
            all_zero = PET_FALSE;
        }
        if (slot[i] != 0xffu) {
            all_ff = PET_FALSE;
        }
    }

    return (all_zero || all_ff) ? PET_TRUE : PET_FALSE;
}

static void pet_save_jieli_decode_header(const pet_u8_t *slot, pet_save_slot_header_t *header)
{
    pet_u32_t i;

    header->magic = pet_save_jieli_read_u32(slot + 0u);
    header->version = pet_save_jieli_read_u16(slot + 4u);
    header->schema_version = pet_save_jieli_read_u16(slot + 6u);
    header->payload_type = pet_save_jieli_read_u16(slot + 8u);
    header->reserved0 = pet_save_jieli_read_u16(slot + 10u);
    header->payload_len = pet_save_jieli_read_u32(slot + 12u);
    header->counter = pet_save_jieli_read_u64(slot + 16u);
    header->timestamp_sec = pet_save_jieli_read_u64(slot + 24u);
    header->crc32 = pet_save_jieli_read_u32(slot + 32u);
    for (i = 0u; i < sizeof(header->reserved1); ++i) {
        header->reserved1[i] = slot[36u + i];
    }
}

static void pet_save_jieli_encode_header(pet_u8_t *slot, const pet_save_slot_header_t *header)
{
    pet_u32_t i;

    pet_save_jieli_write_u32(slot + 0u, header->magic);
    pet_save_jieli_write_u16(slot + 4u, header->version);
    pet_save_jieli_write_u16(slot + 6u, header->schema_version);
    pet_save_jieli_write_u16(slot + 8u, header->payload_type);
    pet_save_jieli_write_u16(slot + 10u, header->reserved0);
    pet_save_jieli_write_u32(slot + 12u, header->payload_len);
    pet_save_jieli_write_u64(slot + 16u, header->counter);
    pet_save_jieli_write_u64(slot + 24u, header->timestamp_sec);
    pet_save_jieli_write_u32(slot + 32u, header->crc32);
    for (i = 0u; i < sizeof(header->reserved1); ++i) {
        slot[36u + i] = header->reserved1[i];
    }
}

static void pet_save_jieli_make_header(pet_save_slot_header_t *header,
                                       pet_u32_t payload_len,
                                       pet_u64_t counter,
                                       pet_u64_t timestamp_sec,
                                       pet_u32_t crc32)
{
    pet_u32_t i;

    header->magic = PET_SAVE_MAGIC;
    header->version = PET_SAVE_VERSION;
    header->schema_version = PET_SAVE_SCHEMA_VERSION;
    header->payload_type = PET_SAVE_PAYLOAD_DEVICE;
    header->reserved0 = 0u;
    header->payload_len = payload_len;
    header->counter = counter;
    header->timestamp_sec = timestamp_sec;
    header->crc32 = crc32;
    for (i = 0u; i < sizeof(header->reserved1); ++i) {
        header->reserved1[i] = 0u;
    }
}

static pet_result_t pet_save_jieli_get_slot(PetSaveSlot slot_id,
                                            pet_u8_t **out_slot,
                                            pet_u32_t *out_capacity)
{
    if ((g_pet_save_jieli_backend == 0) || (out_slot == 0) || (out_capacity == 0)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    if (slot_id == PET_SAVE_SLOT_A) {
        *out_slot = g_pet_save_jieli_backend->slot_a;
        *out_capacity = g_pet_save_jieli_backend->slot_a_capacity;
        return PET_RESULT_OK;
    }

    if (slot_id == PET_SAVE_SLOT_B) {
        *out_slot = g_pet_save_jieli_backend->slot_b;
        *out_capacity = g_pet_save_jieli_backend->slot_b_capacity;
        return PET_RESULT_OK;
    }

    return PET_RESULT_INVALID_ARGUMENT;
}

static pet_result_t pet_save_jieli_choose_target_slot(PetSaveSlot latest_slot,
                                                      PetSaveSlot *out_slot)
{
    if (out_slot == 0) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    if (latest_slot == PET_SAVE_SLOT_A) {
        *out_slot = PET_SAVE_SLOT_B;
    } else {
        *out_slot = PET_SAVE_SLOT_A;
    }

    return PET_RESULT_OK;
}

static pet_result_t pet_save_jieli_expect(pet_bool_t condition, pet_result_t result)
{
    return condition ? PET_RESULT_OK : result;
}

pet_u32_t pet_save_jieli_crc32(const pet_u8_t *data, pet_u32_t len)
{
    pet_u32_t crc = 0xffffffffu;
    pet_u32_t i;
    pet_u8_t bit;

    if ((data == 0) && (len != 0u)) {
        return 0u;
    }

    for (i = 0u; i < len; ++i) {
        crc ^= data[i];
        for (bit = 0u; bit < 8u; ++bit) {
            if ((crc & 1u) != 0u) {
                crc = (crc >> 1) ^ 0xedb88320u;
            } else {
                crc >>= 1;
            }
        }
    }

    return crc ^ 0xffffffffu;
}

pet_result_t pet_save_jieli_open_memory_backend(pet_save_jieli_memory_backend_t *backend)
{
    if ((backend == 0) || (backend->slot_a == 0) || (backend->slot_b == 0) ||
        (backend->slot_a_capacity < PET_SAVE_SLOT_HEADER_SIZE) ||
        (backend->slot_b_capacity < PET_SAVE_SLOT_HEADER_SIZE)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    g_pet_save_jieli_backend = backend;
    return PET_RESULT_OK;
}

void pet_save_jieli_close(void)
{
    g_pet_save_jieli_backend = 0;
}

pet_result_t pet_save_jieli_validate_slot(const pet_u8_t *slot,
                                          pet_u32_t slot_capacity,
                                          pet_save_slot_header_t *out_header)
{
    pet_save_slot_header_t header;
    pet_u32_t expected_crc;

    if ((slot == 0) || (slot_capacity < PET_SAVE_SLOT_HEADER_SIZE)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    if (pet_save_jieli_slot_is_empty(slot, slot_capacity) == PET_TRUE) {
        return PET_RESULT_NOT_FOUND;
    }

    pet_save_jieli_decode_header(slot, &header);

    if ((header.magic != PET_SAVE_MAGIC) ||
        (header.version != PET_SAVE_VERSION) ||
        (header.schema_version < PET_SAVE_SCHEMA_VERSION_MIN_READ) ||
        (header.schema_version > PET_SAVE_SCHEMA_VERSION) ||
        (header.payload_type != PET_SAVE_PAYLOAD_DEVICE)) {
        return PET_RESULT_BAD_VERSION;
    }

    if (header.payload_len > (slot_capacity - PET_SAVE_SLOT_HEADER_SIZE)) {
        return PET_RESULT_BUFFER_TOO_SMALL;
    }

    expected_crc = pet_save_jieli_crc32(slot + PET_SAVE_SLOT_HEADER_SIZE, header.payload_len);
    if (expected_crc != header.crc32) {
        return PET_RESULT_BAD_CRC;
    }

    if (out_header != 0) {
        *out_header = header;
    }

    return PET_RESULT_OK;
}

pet_result_t pet_save_jieli_choose_latest_slot(PetSaveSlot *out_slot,
                                               pet_save_slot_header_t *out_header)
{
    pet_result_t ret_a;
    pet_result_t ret_b;
    pet_save_slot_header_t header_a;
    pet_save_slot_header_t header_b;

    if ((out_slot == 0) || (g_pet_save_jieli_backend == 0)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    ret_a = pet_save_jieli_validate_slot(g_pet_save_jieli_backend->slot_a,
                                         g_pet_save_jieli_backend->slot_a_capacity,
                                         &header_a);
    ret_b = pet_save_jieli_validate_slot(g_pet_save_jieli_backend->slot_b,
                                         g_pet_save_jieli_backend->slot_b_capacity,
                                         &header_b);

    if ((ret_a == PET_RESULT_OK) && (ret_b == PET_RESULT_OK)) {
        if (header_b.counter > header_a.counter) {
            *out_slot = PET_SAVE_SLOT_B;
            if (out_header != 0) {
                *out_header = header_b;
            }
        } else {
            *out_slot = PET_SAVE_SLOT_A;
            if (out_header != 0) {
                *out_header = header_a;
            }
        }
        return PET_RESULT_OK;
    }

    if (ret_a == PET_RESULT_OK) {
        *out_slot = PET_SAVE_SLOT_A;
        if (out_header != 0) {
            *out_header = header_a;
        }
        return PET_RESULT_OK;
    }

    if (ret_b == PET_RESULT_OK) {
        *out_slot = PET_SAVE_SLOT_B;
        if (out_header != 0) {
            *out_header = header_b;
        }
        return PET_RESULT_OK;
    }

    *out_slot = PET_SAVE_SLOT_NONE;
    return PET_RESULT_NOT_FOUND;
}

pet_result_t pet_save_jieli_load_latest(pet_u8_t *out_payload,
                                        pet_u32_t out_capacity,
                                        pet_u32_t *out_payload_size,
                                        pet_u64_t *out_counter)
{
    PetSaveSlot slot_id;
    pet_save_slot_header_t header;
    pet_u8_t *slot;
    pet_u32_t slot_capacity;
    pet_result_t ret;

    if ((out_payload_size == 0) || (out_counter == 0) ||
        ((out_payload == 0) && (out_capacity != 0u))) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    ret = pet_save_jieli_choose_latest_slot(&slot_id, &header);
    if (ret != PET_RESULT_OK) {
        return ret;
    }

    if (header.payload_len > out_capacity) {
        return PET_RESULT_BUFFER_TOO_SMALL;
    }

    ret = pet_save_jieli_get_slot(slot_id, &slot, &slot_capacity);
    if (ret != PET_RESULT_OK) {
        return ret;
    }

    (void)slot_capacity;
    pet_save_jieli_copy_bytes(out_payload, slot + PET_SAVE_SLOT_HEADER_SIZE, header.payload_len);
    *out_payload_size = header.payload_len;
    *out_counter = header.counter;
    return PET_RESULT_OK;
}

pet_result_t pet_save_jieli_write_transaction(const pet_u8_t *payload,
                                              pet_u32_t payload_size,
                                              pet_u64_t *out_counter)
{
    PetSaveSlot latest_slot = PET_SAVE_SLOT_NONE;
    PetSaveSlot target_slot = PET_SAVE_SLOT_NONE;
    pet_save_slot_header_t latest_header;
    pet_save_slot_header_t new_header;
    pet_save_slot_header_t staged_header;
    pet_result_t latest_ret;
    pet_result_t ret;
    pet_u8_t *slot;
    pet_u32_t slot_capacity;
    pet_u64_t next_counter;

    if ((g_pet_save_jieli_backend == 0) || ((payload == 0) && (payload_size != 0u))) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    if (g_pet_save_jieli_backend->low_battery_block == PET_TRUE) {
        return PET_RESULT_NOT_READY;
    }

    latest_ret = pet_save_jieli_choose_latest_slot(&latest_slot, &latest_header);
    if ((latest_ret != PET_RESULT_OK) && (latest_ret != PET_RESULT_NOT_FOUND)) {
        return latest_ret;
    }

    ret = pet_save_jieli_choose_target_slot(latest_slot, &target_slot);
    if (ret != PET_RESULT_OK) {
        return ret;
    }

    ret = pet_save_jieli_get_slot(target_slot, &slot, &slot_capacity);
    if (ret != PET_RESULT_OK) {
        return ret;
    }

    if (payload_size > (slot_capacity - PET_SAVE_SLOT_HEADER_SIZE)) {
        return PET_RESULT_BUFFER_TOO_SMALL;
    }

    if (g_pet_save_jieli_backend->fault == PET_SAVE_JIELI_FAULT_BEFORE_WRITE) {
        return PET_RESULT_STORAGE_ERROR;
    }

    next_counter = (latest_ret == PET_RESULT_OK) ? (latest_header.counter + 1u) : 1u;
    pet_save_jieli_make_header(&new_header, payload_size, next_counter, 0u,
                               pet_save_jieli_crc32(payload, payload_size));

    staged_header = new_header;
    staged_header.crc32 ^= 0xffffffffu;
    pet_save_jieli_zero_bytes(slot, slot_capacity);
    pet_save_jieli_encode_header(slot, &staged_header);
    if (g_pet_save_jieli_backend->fault == PET_SAVE_JIELI_FAULT_AFTER_HEADER) {
        return PET_RESULT_STORAGE_ERROR;
    }

    pet_save_jieli_copy_bytes(slot + PET_SAVE_SLOT_HEADER_SIZE, payload, payload_size);
    if (g_pet_save_jieli_backend->fault == PET_SAVE_JIELI_FAULT_AFTER_PAYLOAD) {
        return PET_RESULT_STORAGE_ERROR;
    }

    pet_save_jieli_encode_header(slot, &new_header);
    if (g_pet_save_jieli_backend->fault == PET_SAVE_JIELI_FAULT_CORRUPT_AFTER_WRITE) {
        if (payload_size != 0u) {
            slot[PET_SAVE_SLOT_HEADER_SIZE] ^= 0x5au;
        } else {
            slot[4u] ^= 0x5au;
        }
    }

    ret = pet_save_jieli_validate_slot(slot, slot_capacity, 0);
    if (ret != PET_RESULT_OK) {
        return ret;
    }

    if (out_counter != 0) {
        *out_counter = next_counter;
    }

    return PET_RESULT_OK;
}

static pet_result_t pet_save_jieli_self_test_sequence_write_load(void)
{
    pet_u8_t slot_a[PET_SAVE_JIELI_TEST_SLOT_CAPACITY];
    pet_u8_t slot_b[PET_SAVE_JIELI_TEST_SLOT_CAPACITY];
    pet_u8_t payload1[5] = {1u, 2u, 3u, 4u, 5u};
    pet_u8_t payload2[6] = {9u, 8u, 7u, 6u, 5u, 4u};
    pet_u8_t out[8];
    pet_u32_t out_size = 0u;
    pet_u64_t counter = 0u;
    PetSaveSlot latest = PET_SAVE_SLOT_NONE;
    pet_save_slot_header_t header;
    pet_save_jieli_memory_backend_t backend;
    pet_result_t ret;

    ret = pet_save_jieli_memory_backend_init(&backend, slot_a, sizeof(slot_a), slot_b, sizeof(slot_b));
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    pet_save_jieli_memory_backend_clear(&backend);

    ret = pet_save_jieli_open_memory_backend(0);
    if (ret != PET_RESULT_INVALID_ARGUMENT) {
        return PET_RESULT_ERROR;
    }
    ret = pet_save_jieli_open_memory_backend(&backend);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    ret = pet_save_jieli_load_latest(out, sizeof(out), &out_size, &counter);
    if (ret != PET_RESULT_NOT_FOUND) {
        return PET_RESULT_ERROR;
    }

    ret = pet_save_jieli_write_transaction(payload1, sizeof(payload1), &counter);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    ret = pet_save_jieli_expect(counter == 1u, PET_RESULT_ERROR);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    ret = pet_save_jieli_load_latest(out, sizeof(out), &out_size, &counter);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    ret = pet_save_jieli_expect((counter == 1u) && (out_size == sizeof(payload1)) &&
                                (out[0] == payload1[0]) && (out[4] == payload1[4]),
                                PET_RESULT_ERROR);
    if (ret != PET_RESULT_OK) {
        return ret;
    }

    ret = pet_save_jieli_write_transaction(payload2, sizeof(payload2), &counter);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    ret = pet_save_jieli_expect(counter == 2u, PET_RESULT_ERROR);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    ret = pet_save_jieli_choose_latest_slot(&latest, &header);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    ret = pet_save_jieli_expect((header.counter == 2u) &&
                                ((latest == PET_SAVE_SLOT_A) || (latest == PET_SAVE_SLOT_B)),
                                PET_RESULT_ERROR);
    if (ret != PET_RESULT_OK) {
        return ret;
    }

    pet_save_jieli_close();
    return PET_RESULT_OK;
}

static pet_result_t pet_save_jieli_self_test_sequence_rollback(void)
{
    pet_u8_t slot_a[PET_SAVE_JIELI_TEST_SLOT_CAPACITY];
    pet_u8_t slot_b[PET_SAVE_JIELI_TEST_SLOT_CAPACITY];
    pet_u8_t payload1[4] = {1u, 1u, 1u, 1u};
    pet_u8_t payload2[4] = {2u, 2u, 2u, 2u};
    pet_u8_t out[8];
    pet_u32_t out_size = 0u;
    pet_u64_t counter = 0u;
    PetSaveSlot latest = PET_SAVE_SLOT_NONE;
    pet_save_slot_header_t header;
    pet_u8_t *slot;
    pet_u32_t slot_capacity;
    pet_save_jieli_memory_backend_t backend;
    pet_result_t ret;

    ret = pet_save_jieli_memory_backend_init(&backend, slot_a, sizeof(slot_a), slot_b, sizeof(slot_b));
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    pet_save_jieli_memory_backend_clear(&backend);
    ret = pet_save_jieli_open_memory_backend(&backend);
    if (ret != PET_RESULT_OK) {
        return ret;
    }

    ret = pet_save_jieli_write_transaction(payload1, sizeof(payload1), &counter);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    ret = pet_save_jieli_write_transaction(payload2, sizeof(payload2), &counter);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    ret = pet_save_jieli_choose_latest_slot(&latest, &header);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    ret = pet_save_jieli_get_slot(latest, &slot, &slot_capacity);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    slot[PET_SAVE_SLOT_HEADER_SIZE] ^= 0x33u;
    ret = pet_save_jieli_validate_slot(slot, slot_capacity, 0);
    if (ret != PET_RESULT_BAD_CRC) {
        return PET_RESULT_ERROR;
    }
    ret = pet_save_jieli_load_latest(out, sizeof(out), &out_size, &counter);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    ret = pet_save_jieli_expect((counter == 1u) && (out[0] == payload1[0]), PET_RESULT_ERROR);
    if (ret != PET_RESULT_OK) {
        return ret;
    }

    pet_save_jieli_memory_backend_clear(&backend);
    ret = pet_save_jieli_write_transaction(payload1, sizeof(payload1), &counter);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    ret = pet_save_jieli_write_transaction(payload2, sizeof(payload2), &counter);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    if (latest == PET_SAVE_SLOT_A) {
        slot = slot_b;
    } else {
        slot = slot_a;
    }
    slot[PET_SAVE_SLOT_HEADER_SIZE] ^= 0x11u;
    ret = pet_save_jieli_load_latest(out, sizeof(out), &out_size, &counter);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    ret = pet_save_jieli_expect((counter == 2u) && (out[0] == payload2[0]), PET_RESULT_ERROR);
    if (ret != PET_RESULT_OK) {
        return ret;
    }

    pet_save_jieli_close();
    return PET_RESULT_OK;
}

static pet_result_t pet_save_jieli_self_test_sequence_faults(void)
{
    pet_u8_t slot_a[PET_SAVE_JIELI_TEST_SLOT_CAPACITY];
    pet_u8_t slot_b[PET_SAVE_JIELI_TEST_SLOT_CAPACITY];
    pet_u8_t payload1[4] = {1u, 2u, 3u, 4u};
    pet_u8_t payload2[4] = {5u, 6u, 7u, 8u};
    pet_u8_t out[8];
    pet_u32_t out_size = 0u;
    pet_u64_t counter = 0u;
    pet_save_jieli_memory_backend_t backend;
    pet_result_t ret;

    ret = pet_save_jieli_memory_backend_init(&backend, slot_a, sizeof(slot_a), slot_b, sizeof(slot_b));
    if (ret != PET_RESULT_OK) {
        return ret;
    }

    pet_save_jieli_memory_backend_clear(&backend);
    ret = pet_save_jieli_open_memory_backend(&backend);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    ret = pet_save_jieli_write_transaction(payload1, sizeof(payload1), &counter);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    pet_save_jieli_memory_backend_set_fault(&backend, PET_SAVE_JIELI_FAULT_AFTER_HEADER);
    ret = pet_save_jieli_write_transaction(payload2, sizeof(payload2), &counter);
    if (ret != PET_RESULT_STORAGE_ERROR) {
        return PET_RESULT_ERROR;
    }
    pet_save_jieli_memory_backend_set_fault(&backend, PET_SAVE_JIELI_FAULT_NONE);
    ret = pet_save_jieli_load_latest(out, sizeof(out), &out_size, &counter);
    if ((ret != PET_RESULT_OK) || (counter != 1u) || (out[0] != payload1[0])) {
        return PET_RESULT_ERROR;
    }

    pet_save_jieli_memory_backend_clear(&backend);
    ret = pet_save_jieli_write_transaction(payload1, sizeof(payload1), &counter);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    pet_save_jieli_memory_backend_set_fault(&backend, PET_SAVE_JIELI_FAULT_AFTER_PAYLOAD);
    ret = pet_save_jieli_write_transaction(payload2, sizeof(payload2), &counter);
    if (ret != PET_RESULT_STORAGE_ERROR) {
        return PET_RESULT_ERROR;
    }
    pet_save_jieli_memory_backend_set_fault(&backend, PET_SAVE_JIELI_FAULT_NONE);
    ret = pet_save_jieli_load_latest(out, sizeof(out), &out_size, &counter);
    if ((ret != PET_RESULT_OK) || (counter != 1u) || (out[0] != payload1[0])) {
        return PET_RESULT_ERROR;
    }

    pet_save_jieli_memory_backend_clear(&backend);
    ret = pet_save_jieli_write_transaction(payload1, sizeof(payload1), &counter);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    pet_save_jieli_memory_backend_set_fault(&backend, PET_SAVE_JIELI_FAULT_BEFORE_WRITE);
    ret = pet_save_jieli_write_transaction(payload2, sizeof(payload2), &counter);
    if (ret != PET_RESULT_STORAGE_ERROR) {
        return PET_RESULT_ERROR;
    }
    pet_save_jieli_memory_backend_set_fault(&backend, PET_SAVE_JIELI_FAULT_NONE);
    ret = pet_save_jieli_load_latest(out, sizeof(out), &out_size, &counter);
    if ((ret != PET_RESULT_OK) || (counter != 1u)) {
        return PET_RESULT_ERROR;
    }

    pet_save_jieli_memory_backend_set_low_battery_block(&backend, PET_TRUE);
    ret = pet_save_jieli_write_transaction(payload2, sizeof(payload2), &counter);
    if (ret != PET_RESULT_NOT_READY) {
        return PET_RESULT_ERROR;
    }
    pet_save_jieli_memory_backend_set_low_battery_block(&backend, PET_FALSE);

    pet_save_jieli_close();
    return PET_RESULT_OK;
}

static pet_result_t pet_save_jieli_self_test_sequence_corruption(void)
{
    pet_u8_t slot_a[PET_SAVE_JIELI_TEST_SLOT_CAPACITY];
    pet_u8_t slot_b[PET_SAVE_JIELI_TEST_SLOT_CAPACITY];
    pet_u8_t payload[4] = {7u, 7u, 7u, 7u};
    pet_u8_t oversize[PET_SAVE_JIELI_TEST_SLOT_CAPACITY];
    pet_u64_t counter = 0u;
    PetSaveSlot latest = PET_SAVE_SLOT_NONE;
    pet_save_slot_header_t header;
    pet_u8_t *slot;
    pet_u32_t slot_capacity;
    pet_save_jieli_memory_backend_t backend;
    pet_result_t ret;

    ret = pet_save_jieli_memory_backend_init(&backend, slot_a, sizeof(slot_a), slot_b, sizeof(slot_b));
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    pet_save_jieli_memory_backend_clear(&backend);
    pet_save_jieli_zero_bytes(oversize, sizeof(oversize));
    ret = pet_save_jieli_open_memory_backend(&backend);
    if (ret != PET_RESULT_OK) {
        return ret;
    }

    ret = pet_save_jieli_write_transaction(oversize, sizeof(oversize), &counter);
    if (ret != PET_RESULT_BUFFER_TOO_SMALL) {
        return PET_RESULT_ERROR;
    }

    ret = pet_save_jieli_write_transaction(payload, sizeof(payload), &counter);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    ret = pet_save_jieli_choose_latest_slot(&latest, &header);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    ret = pet_save_jieli_get_slot(latest, &slot, &slot_capacity);
    if (ret != PET_RESULT_OK) {
        return ret;
    }

    slot[4u] = 0u;
    ret = pet_save_jieli_validate_slot(slot, slot_capacity, &header);
    if (ret != PET_RESULT_BAD_VERSION) {
        return PET_RESULT_ERROR;
    }

    pet_save_jieli_memory_backend_clear(&backend);
    ret = pet_save_jieli_write_transaction(payload, sizeof(payload), &counter);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    ret = pet_save_jieli_choose_latest_slot(&latest, &header);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    ret = pet_save_jieli_get_slot(latest, &slot, &slot_capacity);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    slot[PET_SAVE_SLOT_HEADER_SIZE] ^= 0x44u;
    ret = pet_save_jieli_validate_slot(slot, slot_capacity, &header);
    if (ret != PET_RESULT_BAD_CRC) {
        return PET_RESULT_ERROR;
    }

    pet_save_jieli_memory_backend_clear(&backend);
    pet_save_jieli_memory_backend_set_fault(&backend, PET_SAVE_JIELI_FAULT_CORRUPT_AFTER_WRITE);
    ret = pet_save_jieli_write_transaction(payload, sizeof(payload), &counter);
    if (ret != PET_RESULT_BAD_CRC) {
        return PET_RESULT_ERROR;
    }
    pet_save_jieli_memory_backend_set_fault(&backend, PET_SAVE_JIELI_FAULT_NONE);

    pet_save_jieli_close();
    return PET_RESULT_OK;
}

pet_result_t pet_save_jieli_self_test(void)
{
    pet_result_t ret;

    ret = pet_save_jieli_self_test_sequence_write_load();
    if (ret != PET_RESULT_OK) {
        pet_save_jieli_close();
        return ret;
    }

    ret = pet_save_jieli_self_test_sequence_rollback();
    if (ret != PET_RESULT_OK) {
        pet_save_jieli_close();
        return ret;
    }

    ret = pet_save_jieli_self_test_sequence_faults();
    if (ret != PET_RESULT_OK) {
        pet_save_jieli_close();
        return ret;
    }

    ret = pet_save_jieli_self_test_sequence_corruption();
    if (ret != PET_RESULT_OK) {
        pet_save_jieli_close();
        return ret;
    }

    pet_save_jieli_close();
    return PET_RESULT_OK;
}
