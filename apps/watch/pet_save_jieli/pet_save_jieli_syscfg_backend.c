#include "pet_save_jieli_syscfg_backend.h"

#if defined(PET_PLATFORM_JIELI_TEST)
static pet_u8_t g_pet_save_jieli_fake_slot_a[PET_SAVE_JIELI_SYSCFG_SLOT_CAPACITY];
static pet_u8_t g_pet_save_jieli_fake_slot_b[PET_SAVE_JIELI_SYSCFG_SLOT_CAPACITY];
static pet_bool_t g_pet_save_jieli_fake_slot_a_valid;
static pet_bool_t g_pet_save_jieli_fake_slot_b_valid;
#else
extern int syscfg_read(unsigned short item_id, void *buf, unsigned short len);
extern int syscfg_write(unsigned short item_id, void *buf, unsigned short len);
#endif

static pet_u8_t g_pet_save_jieli_syscfg_slot_a[PET_SAVE_JIELI_SYSCFG_SLOT_CAPACITY];
static pet_u8_t g_pet_save_jieli_syscfg_slot_b[PET_SAVE_JIELI_SYSCFG_SLOT_CAPACITY];
static pet_save_jieli_memory_backend_t g_pet_save_jieli_syscfg_backend;
static pet_save_jieli_syscfg_stats_t g_pet_save_jieli_syscfg_stats;

static void pet_save_jieli_syscfg_zero(pet_u8_t *bytes, pet_u32_t len)
{
    pet_u32_t i;

    for (i = 0u; i < len; ++i) {
        bytes[i] = 0u;
    }
}

static void pet_save_jieli_syscfg_copy(pet_u8_t *dst, const pet_u8_t *src, pet_u32_t len)
{
    pet_u32_t i;

    for (i = 0u; i < len; ++i) {
        dst[i] = src[i];
    }
}

static pet_bool_t pet_save_jieli_syscfg_equal(const pet_u8_t *a, const pet_u8_t *b, pet_u32_t len)
{
    pet_u32_t i;

    for (i = 0u; i < len; ++i) {
        if (a[i] != b[i]) {
            return PET_FALSE;
        }
    }
    return PET_TRUE;
}

static void pet_save_jieli_syscfg_reset_stats(void)
{
    g_pet_save_jieli_syscfg_stats.slot_a_item_id = PET_SAVE_JIELI_SYSCFG_SLOT_A_ITEM_ID;
    g_pet_save_jieli_syscfg_stats.slot_b_item_id = PET_SAVE_JIELI_SYSCFG_SLOT_B_ITEM_ID;
    g_pet_save_jieli_syscfg_stats.slot_capacity = PET_SAVE_JIELI_SYSCFG_SLOT_CAPACITY;
#if defined(PET_PLATFORM_JIELI_TEST)
    g_pet_save_jieli_syscfg_stats.backend_type = (pet_u8_t)PET_SAVE_JIELI_SYSCFG_BACKEND_FAKE;
#else
    g_pet_save_jieli_syscfg_stats.backend_type = (pet_u8_t)PET_SAVE_JIELI_SYSCFG_BACKEND_SYSCFG;
#endif
    g_pet_save_jieli_syscfg_stats.selected_slot = (pet_u8_t)PET_SAVE_SLOT_NONE;
    g_pet_save_jieli_syscfg_stats.slot_a_read_result = PET_RESULT_NOT_FOUND;
    g_pet_save_jieli_syscfg_stats.slot_b_read_result = PET_RESULT_NOT_FOUND;
    g_pet_save_jieli_syscfg_stats.last_write_result = PET_RESULT_NOT_READY;
    g_pet_save_jieli_syscfg_stats.last_readback_result = PET_RESULT_NOT_READY;
    g_pet_save_jieli_syscfg_stats.last_fallback_result = PET_RESULT_NOT_READY;
    g_pet_save_jieli_syscfg_stats.selected_counter = 0u;
    g_pet_save_jieli_syscfg_stats.selected_payload_len = 0u;
    g_pet_save_jieli_syscfg_stats.selected_crc32 = 0u;
    g_pet_save_jieli_syscfg_stats.write_case_pass_count = 0u;
    g_pet_save_jieli_syscfg_stats.write_case_fail_count = 0u;
    g_pet_save_jieli_syscfg_stats.readback_case_pass_count = 0u;
    g_pet_save_jieli_syscfg_stats.readback_case_fail_count = 0u;
    g_pet_save_jieli_syscfg_stats.fallback_case_pass_count = 0u;
    g_pet_save_jieli_syscfg_stats.fallback_case_fail_count = 0u;
    g_pet_save_jieli_syscfg_stats.real_write_verified = 0u;
    g_pet_save_jieli_syscfg_stats.non_destructive_namespace = 1u;
}

static pet_u16_t pet_save_jieli_syscfg_item_for_slot(PetSaveSlot slot)
{
    return (slot == PET_SAVE_SLOT_B) ? PET_SAVE_JIELI_SYSCFG_SLOT_B_ITEM_ID :
                                      PET_SAVE_JIELI_SYSCFG_SLOT_A_ITEM_ID;
}

#if defined(PET_PLATFORM_JIELI_TEST)
static int pet_save_jieli_syscfg_read_item(pet_u16_t item_id, pet_u8_t *buf, pet_u16_t len)
{
    if ((buf == 0) || (len != PET_SAVE_JIELI_SYSCFG_SLOT_CAPACITY)) {
        return -1;
    }

    if (item_id == PET_SAVE_JIELI_SYSCFG_SLOT_A_ITEM_ID) {
        if (g_pet_save_jieli_fake_slot_a_valid != PET_TRUE) {
            return -1;
        }
        pet_save_jieli_syscfg_copy(buf, g_pet_save_jieli_fake_slot_a, len);
        return (int)len;
    }

    if (item_id == PET_SAVE_JIELI_SYSCFG_SLOT_B_ITEM_ID) {
        if (g_pet_save_jieli_fake_slot_b_valid != PET_TRUE) {
            return -1;
        }
        pet_save_jieli_syscfg_copy(buf, g_pet_save_jieli_fake_slot_b, len);
        return (int)len;
    }

    return -1;
}

static int pet_save_jieli_syscfg_write_item(pet_u16_t item_id, const pet_u8_t *buf, pet_u16_t len)
{
    if ((buf == 0) || (len != PET_SAVE_JIELI_SYSCFG_SLOT_CAPACITY)) {
        return -1;
    }

    if (item_id == PET_SAVE_JIELI_SYSCFG_SLOT_A_ITEM_ID) {
        pet_save_jieli_syscfg_copy(g_pet_save_jieli_fake_slot_a, buf, len);
        g_pet_save_jieli_fake_slot_a_valid = PET_TRUE;
        return (int)len;
    }

    if (item_id == PET_SAVE_JIELI_SYSCFG_SLOT_B_ITEM_ID) {
        pet_save_jieli_syscfg_copy(g_pet_save_jieli_fake_slot_b, buf, len);
        g_pet_save_jieli_fake_slot_b_valid = PET_TRUE;
        return (int)len;
    }

    return -1;
}
#else
static int pet_save_jieli_syscfg_read_item(pet_u16_t item_id, pet_u8_t *buf, pet_u16_t len)
{
    return syscfg_read((unsigned short)item_id, buf, (unsigned short)len);
}

static int pet_save_jieli_syscfg_write_item(pet_u16_t item_id, const pet_u8_t *buf, pet_u16_t len)
{
    return syscfg_write((unsigned short)item_id, (void *)buf, (unsigned short)len);
}
#endif

static pet_result_t pet_save_jieli_syscfg_read_slot(PetSaveSlot slot,
                                                    pet_u8_t *buf,
                                                    pet_result_t *out_read_result)
{
    int ret;
    pet_u16_t item_id;

    if ((buf == 0) || (out_read_result == 0)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    item_id = pet_save_jieli_syscfg_item_for_slot(slot);
    ret = pet_save_jieli_syscfg_read_item(item_id, buf, PET_SAVE_JIELI_SYSCFG_SLOT_CAPACITY);
    if (ret == (int)PET_SAVE_JIELI_SYSCFG_SLOT_CAPACITY) {
        *out_read_result = PET_RESULT_OK;
        return PET_RESULT_OK;
    }

    pet_save_jieli_syscfg_zero(buf, PET_SAVE_JIELI_SYSCFG_SLOT_CAPACITY);
    *out_read_result = PET_RESULT_NOT_FOUND;
    return *out_read_result;
}

static pet_result_t pet_save_jieli_syscfg_sync_from_storage(void)
{
    pet_result_t ret_a;
    pet_result_t ret_b;

    ret_a = pet_save_jieli_syscfg_read_slot(PET_SAVE_SLOT_A,
                                            g_pet_save_jieli_syscfg_slot_a,
                                            &g_pet_save_jieli_syscfg_stats.slot_a_read_result);
    ret_b = pet_save_jieli_syscfg_read_slot(PET_SAVE_SLOT_B,
                                            g_pet_save_jieli_syscfg_slot_b,
                                            &g_pet_save_jieli_syscfg_stats.slot_b_read_result);

    if (((ret_a == PET_RESULT_OK) || (ret_a == PET_RESULT_NOT_FOUND)) &&
        ((ret_b == PET_RESULT_OK) || (ret_b == PET_RESULT_NOT_FOUND))) {
        return PET_RESULT_OK;
    }

    return PET_RESULT_STORAGE_ERROR;
}

static pet_result_t pet_save_jieli_syscfg_open_memory_view(void)
{
    pet_result_t ret;

    ret = pet_save_jieli_memory_backend_init(&g_pet_save_jieli_syscfg_backend,
                                             g_pet_save_jieli_syscfg_slot_a,
                                             PET_SAVE_JIELI_SYSCFG_SLOT_CAPACITY,
                                             g_pet_save_jieli_syscfg_slot_b,
                                             PET_SAVE_JIELI_SYSCFG_SLOT_CAPACITY);
    if (ret != PET_RESULT_OK) {
        return ret;
    }

    return pet_save_jieli_open_memory_backend(&g_pet_save_jieli_syscfg_backend);
}

static pet_result_t pet_save_jieli_syscfg_write_slot(PetSaveSlot slot)
{
    const pet_u8_t *slot_bytes;
    pet_u16_t item_id;
    int ret;

    if (slot == PET_SAVE_SLOT_A) {
        slot_bytes = g_pet_save_jieli_syscfg_slot_a;
    } else if (slot == PET_SAVE_SLOT_B) {
        slot_bytes = g_pet_save_jieli_syscfg_slot_b;
    } else {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    item_id = pet_save_jieli_syscfg_item_for_slot(slot);
    ret = pet_save_jieli_syscfg_write_item(item_id, slot_bytes, PET_SAVE_JIELI_SYSCFG_SLOT_CAPACITY);
    return (ret == (int)PET_SAVE_JIELI_SYSCFG_SLOT_CAPACITY) ? PET_RESULT_OK :
                                                              PET_RESULT_STORAGE_ERROR;
}

static void pet_save_jieli_syscfg_record_selected(PetSaveSlot slot,
                                                  const pet_save_slot_header_t *header)
{
    g_pet_save_jieli_syscfg_stats.selected_slot = (pet_u8_t)slot;
    if (header != 0) {
        g_pet_save_jieli_syscfg_stats.selected_counter = header->counter;
        g_pet_save_jieli_syscfg_stats.selected_payload_len = header->payload_len;
        g_pet_save_jieli_syscfg_stats.selected_crc32 = header->crc32;
    }
}

pet_result_t pet_save_jieli_syscfg_load_latest(pet_u8_t *out_payload,
                                               pet_u32_t out_capacity,
                                               pet_u32_t *out_payload_size,
                                               pet_u64_t *out_counter)
{
    PetSaveSlot slot = PET_SAVE_SLOT_NONE;
    pet_save_slot_header_t header;
    pet_result_t ret;

    pet_save_jieli_syscfg_reset_stats();
    ret = pet_save_jieli_syscfg_sync_from_storage();
    if (ret != PET_RESULT_OK) {
        return ret;
    }

    ret = pet_save_jieli_syscfg_open_memory_view();
    if (ret != PET_RESULT_OK) {
        return ret;
    }

    ret = pet_save_jieli_choose_latest_slot(&slot, &header);
    if (ret == PET_RESULT_OK) {
        pet_save_jieli_syscfg_record_selected(slot, &header);
        ret = pet_save_jieli_load_latest(out_payload, out_capacity, out_payload_size, out_counter);
    }

    pet_save_jieli_close();
    return ret;
}

pet_result_t pet_save_jieli_syscfg_write_transaction(const pet_u8_t *payload,
                                                     pet_u32_t payload_size,
                                                     pet_u64_t *out_counter)
{
    PetSaveSlot latest = PET_SAVE_SLOT_NONE;
    pet_save_slot_header_t header;
    pet_u8_t readback[PET_SAVE_JIELI_SYSCFG_MAX_PAYLOAD];
    pet_u32_t readback_size = 0u;
    pet_u64_t readback_counter = 0u;
    pet_u64_t written_counter = 0u;
    pet_result_t ret;

    pet_save_jieli_syscfg_reset_stats();
    if ((payload == 0) && (payload_size != 0u)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }
    if (payload_size > PET_SAVE_JIELI_SYSCFG_MAX_PAYLOAD) {
        return PET_RESULT_BUFFER_TOO_SMALL;
    }

    ret = pet_save_jieli_syscfg_sync_from_storage();
    if (ret != PET_RESULT_OK) {
        g_pet_save_jieli_syscfg_stats.last_write_result = ret;
        return ret;
    }

    ret = pet_save_jieli_syscfg_open_memory_view();
    if (ret != PET_RESULT_OK) {
        g_pet_save_jieli_syscfg_stats.last_write_result = ret;
        return ret;
    }

    ret = pet_save_jieli_write_transaction(payload, payload_size, &written_counter);
    if (ret == PET_RESULT_OK) {
        ret = pet_save_jieli_choose_latest_slot(&latest, &header);
    }
    if (ret == PET_RESULT_OK) {
        pet_save_jieli_syscfg_record_selected(latest, &header);
        ret = pet_save_jieli_syscfg_write_slot(latest);
    }
    pet_save_jieli_close();

    g_pet_save_jieli_syscfg_stats.last_write_result = ret;
    if (ret != PET_RESULT_OK) {
        return ret;
    }

    ret = pet_save_jieli_syscfg_sync_from_storage();
    if (ret == PET_RESULT_OK) {
        ret = pet_save_jieli_syscfg_open_memory_view();
    }
    if (ret == PET_RESULT_OK) {
        ret = pet_save_jieli_load_latest(readback, sizeof(readback),
                                         &readback_size, &readback_counter);
    }
    pet_save_jieli_close();
    g_pet_save_jieli_syscfg_stats.last_readback_result = ret;
    if (ret != PET_RESULT_OK) {
        return ret;
    }

    if ((readback_counter != written_counter) ||
        (readback_size != payload_size) ||
        (pet_save_jieli_syscfg_equal(readback, payload, payload_size) != PET_TRUE)) {
        return PET_RESULT_STORAGE_ERROR;
    }

    g_pet_save_jieli_syscfg_stats.real_write_verified = 1u;
    if (out_counter != 0) {
        *out_counter = written_counter;
    }
    return PET_RESULT_OK;
}

pet_result_t pet_save_jieli_syscfg_get_last_stats(pet_save_jieli_syscfg_stats_t *out_stats)
{
    if (out_stats == 0) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    *out_stats = g_pet_save_jieli_syscfg_stats;
    return PET_RESULT_OK;
}

static pet_result_t pet_save_jieli_syscfg_corrupt_latest_for_test(void)
{
    PetSaveSlot latest = PET_SAVE_SLOT_NONE;
    pet_save_slot_header_t header;
    pet_result_t ret;

    ret = pet_save_jieli_syscfg_sync_from_storage();
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    ret = pet_save_jieli_syscfg_open_memory_view();
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    ret = pet_save_jieli_choose_latest_slot(&latest, &header);
    if (ret != PET_RESULT_OK) {
        pet_save_jieli_close();
        return ret;
    }

    if (latest == PET_SAVE_SLOT_A) {
        g_pet_save_jieli_syscfg_slot_a[PET_SAVE_SLOT_HEADER_SIZE] ^= 0x5au;
    } else {
        g_pet_save_jieli_syscfg_slot_b[PET_SAVE_SLOT_HEADER_SIZE] ^= 0x5au;
    }
    pet_save_jieli_close();

    return pet_save_jieli_syscfg_write_slot(latest);
}

pet_result_t pet_save_jieli_syscfg_self_test(void)
{
    pet_u8_t payload1[32];
    pet_u8_t payload2[32];
    pet_u8_t payload3[32];
    pet_u8_t out[PET_SAVE_JIELI_SYSCFG_MAX_PAYLOAD];
    pet_u32_t out_size = 0u;
    pet_u64_t counter1 = 0u;
    pet_u64_t counter2 = 0u;
    pet_u64_t counter3 = 0u;
    pet_result_t ret;
    pet_u16_t write_pass = 0u;
    pet_u16_t write_fail = 0u;
    pet_u16_t readback_pass = 0u;
    pet_u16_t readback_fail = 0u;
    pet_u16_t fallback_pass = 0u;
    pet_u16_t fallback_fail = 0u;
    pet_u32_t i;

    for (i = 0u; i < sizeof(payload1); ++i) {
        payload1[i] = (pet_u8_t)(0x21u + i);
        payload2[i] = (pet_u8_t)(0x61u + i);
        payload3[i] = (pet_u8_t)(0xa1u + i);
    }

    ret = pet_save_jieli_syscfg_write_transaction(payload1, sizeof(payload1), &counter1);
    if (ret != PET_RESULT_OK) {
        g_pet_save_jieli_syscfg_stats.write_case_fail_count = 1u;
        return ret;
    }
    write_pass++;
    ret = pet_save_jieli_syscfg_load_latest(out, sizeof(out), &out_size, &counter1);
    if ((ret != PET_RESULT_OK) || (out_size != sizeof(payload1)) ||
        (pet_save_jieli_syscfg_equal(out, payload1, sizeof(payload1)) != PET_TRUE)) {
        g_pet_save_jieli_syscfg_stats.readback_case_fail_count = 1u;
        return PET_RESULT_ERROR;
    }
    readback_pass++;

    ret = pet_save_jieli_syscfg_write_transaction(payload2, sizeof(payload2), &counter2);
    if (ret != PET_RESULT_OK) {
        g_pet_save_jieli_syscfg_stats.write_case_pass_count = write_pass;
        g_pet_save_jieli_syscfg_stats.write_case_fail_count = 1u;
        return ret;
    }
    write_pass++;
    if (counter2 <= counter1) {
        g_pet_save_jieli_syscfg_stats.write_case_pass_count = write_pass;
        g_pet_save_jieli_syscfg_stats.readback_case_pass_count = readback_pass;
        return PET_RESULT_ERROR;
    }

    ret = pet_save_jieli_syscfg_corrupt_latest_for_test();
    if (ret != PET_RESULT_OK) {
        g_pet_save_jieli_syscfg_stats.write_case_pass_count = write_pass;
        g_pet_save_jieli_syscfg_stats.readback_case_pass_count = readback_pass;
        g_pet_save_jieli_syscfg_stats.fallback_case_fail_count = 1u;
        return ret;
    }
    ret = pet_save_jieli_syscfg_load_latest(out, sizeof(out), &out_size, &counter3);
    g_pet_save_jieli_syscfg_stats.last_fallback_result = ret;
    if ((ret != PET_RESULT_OK) || (counter3 != counter1) ||
        (out_size != sizeof(payload1)) ||
        (pet_save_jieli_syscfg_equal(out, payload1, sizeof(payload1)) != PET_TRUE)) {
        g_pet_save_jieli_syscfg_stats.write_case_pass_count = write_pass;
        g_pet_save_jieli_syscfg_stats.readback_case_pass_count = readback_pass;
        g_pet_save_jieli_syscfg_stats.fallback_case_fail_count = 1u;
        return PET_RESULT_ERROR;
    }
    fallback_pass++;

    ret = pet_save_jieli_syscfg_write_transaction(payload3, sizeof(payload3), &counter3);
    if (ret != PET_RESULT_OK) {
        g_pet_save_jieli_syscfg_stats.write_case_pass_count = write_pass;
        g_pet_save_jieli_syscfg_stats.readback_case_pass_count = readback_pass;
        g_pet_save_jieli_syscfg_stats.fallback_case_pass_count = fallback_pass;
        g_pet_save_jieli_syscfg_stats.write_case_fail_count = 1u;
        return ret;
    }
    write_pass++;
    ret = pet_save_jieli_syscfg_load_latest(out, sizeof(out), &out_size, &counter3);
    if ((ret != PET_RESULT_OK) || (out_size != sizeof(payload3)) ||
        (pet_save_jieli_syscfg_equal(out, payload3, sizeof(payload3)) != PET_TRUE)) {
        g_pet_save_jieli_syscfg_stats.write_case_pass_count = write_pass;
        g_pet_save_jieli_syscfg_stats.readback_case_pass_count = readback_pass;
        g_pet_save_jieli_syscfg_stats.fallback_case_pass_count = fallback_pass;
        g_pet_save_jieli_syscfg_stats.readback_case_fail_count = 1u;
        return PET_RESULT_ERROR;
    }
    readback_pass++;

    g_pet_save_jieli_syscfg_stats.write_case_pass_count = write_pass;
    g_pet_save_jieli_syscfg_stats.write_case_fail_count = write_fail;
    g_pet_save_jieli_syscfg_stats.readback_case_pass_count = readback_pass;
    g_pet_save_jieli_syscfg_stats.readback_case_fail_count = readback_fail;
    g_pet_save_jieli_syscfg_stats.fallback_case_pass_count = fallback_pass;
    g_pet_save_jieli_syscfg_stats.fallback_case_fail_count = fallback_fail;
    g_pet_save_jieli_syscfg_stats.last_write_result = PET_RESULT_OK;
    g_pet_save_jieli_syscfg_stats.last_readback_result = PET_RESULT_OK;
    g_pet_save_jieli_syscfg_stats.last_fallback_result = PET_RESULT_OK;
    g_pet_save_jieli_syscfg_stats.real_write_verified = 1u;

    return PET_RESULT_OK;
}
