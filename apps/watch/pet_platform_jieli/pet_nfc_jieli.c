#include "pet_platform_jieli_internal.h"

#define PET_NFC_JIELI_QUEUE_CAPACITY 2u

#if PET_JIELI_TEST_MODE_ENABLED
typedef struct {
    pet_nfc_card_t card_queue[PET_NFC_JIELI_QUEUE_CAPACITY];
    pet_nfc_pair_t pair_queue[PET_NFC_JIELI_QUEUE_CAPACITY];
    pet_u8_t card_head;
    pet_u8_t card_tail;
    pet_u8_t card_count;
    pet_u8_t pair_head;
    pet_u8_t pair_tail;
    pet_u8_t pair_count;
    pet_bool_t fake_enabled;
} pet_nfc_jieli_state_t;

static pet_nfc_jieli_state_t g_pet_nfc_jieli_state;

static void pet_nfc_jieli_zero_pair(pet_nfc_pair_t *pair)
{
    pet_u32_t i;

    for (i = 0u; i < sizeof(*pair); ++i) {
        ((pet_u8_t *)pair)[i] = 0u;
    }
}

static void pet_nfc_jieli_pair_from_payload(const pet_nfc_pair_payload_t *payload,
                                            pet_nfc_pair_t *out_pair)
{
    pet_u32_t i;

    pet_nfc_jieli_zero_pair(out_pair);
    out_pair->peer_device_id[0] = (pet_u8_t)(payload->device_id & 0xffu);
    out_pair->peer_device_id[1] = (pet_u8_t)((payload->device_id >> 8u) & 0xffu);
    out_pair->peer_device_id[2] = (pet_u8_t)((payload->device_id >> 16u) & 0xffu);
    out_pair->peer_device_id[3] = (pet_u8_t)((payload->device_id >> 24u) & 0xffu);
    out_pair->peer_device_id[4] = (pet_u8_t)(payload->device_short_id & 0xffu);
    out_pair->peer_device_id[5] = (pet_u8_t)((payload->device_short_id >> 8u) & 0xffu);
    out_pair->nonce[0] = (pet_u8_t)(payload->nonce & 0xffu);
    out_pair->nonce[1] = (pet_u8_t)((payload->nonce >> 8u) & 0xffu);
    out_pair->nonce[2] = (pet_u8_t)((payload->nonce >> 16u) & 0xffu);
    out_pair->nonce[3] = (pet_u8_t)((payload->nonce >> 24u) & 0xffu);
    out_pair->nonce[4] = (pet_u8_t)(payload->session_seed & 0xffu);
    out_pair->nonce[5] = (pet_u8_t)((payload->session_seed >> 8u) & 0xffu);
    out_pair->nonce[6] = (pet_u8_t)((payload->session_seed >> 16u) & 0xffu);
    out_pair->nonce[7] = (pet_u8_t)((payload->session_seed >> 24u) & 0xffu);
    for (i = 8u; i < PET_PROTOCOL_NONCE_LEN; ++i) {
        out_pair->nonce[i] = (pet_u8_t)i;
    }
    out_pair->role = (pet_u8_t)(payload->flags & 0xffu);
}
#endif

void pet_nfc_jieli_init(void)
{
}

pet_result_t pet_nfc_jieli_start_card_scan(void *ctx)
{
    (void)ctx;
#if PET_JIELI_TEST_MODE_ENABLED
    if (g_pet_nfc_jieli_state.fake_enabled == PET_TRUE) {
        return PET_RESULT_OK;
    }
#endif
    return PET_RESULT_NOT_READY;
}

pet_result_t pet_nfc_jieli_start_pair_scan(void *ctx)
{
    (void)ctx;
#if PET_JIELI_TEST_MODE_ENABLED
    if (g_pet_nfc_jieli_state.fake_enabled == PET_TRUE) {
        return PET_RESULT_OK;
    }
#endif
    return PET_RESULT_NOT_READY;
}

pet_result_t pet_nfc_jieli_poll_card(void *ctx, pet_nfc_card_t *card)
{
    (void)ctx;

    if (card == 0) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

#if PET_JIELI_TEST_MODE_ENABLED
    if (g_pet_nfc_jieli_state.fake_enabled == PET_TRUE) {
        if (g_pet_nfc_jieli_state.card_count == 0u) {
            return PET_RESULT_AGAIN;
        }
        *card = g_pet_nfc_jieli_state.card_queue[g_pet_nfc_jieli_state.card_head];
        g_pet_nfc_jieli_state.card_head =
            (pet_u8_t)((g_pet_nfc_jieli_state.card_head + 1u) % PET_NFC_JIELI_QUEUE_CAPACITY);
        g_pet_nfc_jieli_state.card_count--;
        return PET_RESULT_OK;
    }
#endif

    return PET_RESULT_NOT_READY;
}

pet_result_t pet_nfc_jieli_poll_pair(void *ctx, pet_nfc_pair_t *pair)
{
    (void)ctx;

    if (pair == 0) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

#if PET_JIELI_TEST_MODE_ENABLED
    if (g_pet_nfc_jieli_state.fake_enabled == PET_TRUE) {
        if (g_pet_nfc_jieli_state.pair_count == 0u) {
            return PET_RESULT_AGAIN;
        }
        *pair = g_pet_nfc_jieli_state.pair_queue[g_pet_nfc_jieli_state.pair_head];
        g_pet_nfc_jieli_state.pair_head =
            (pet_u8_t)((g_pet_nfc_jieli_state.pair_head + 1u) % PET_NFC_JIELI_QUEUE_CAPACITY);
        g_pet_nfc_jieli_state.pair_count--;
        return PET_RESULT_OK;
    }
#endif

    return PET_RESULT_NOT_READY;
}

#if PET_JIELI_TEST_MODE_ENABLED
pet_result_t pet_nfc_jieli_test_set_fake_enabled(pet_bool_t enabled)
{
    g_pet_nfc_jieli_state.fake_enabled = enabled ? PET_TRUE : PET_FALSE;
    return PET_RESULT_OK;
}

pet_result_t pet_nfc_jieli_test_clear(void)
{
    g_pet_nfc_jieli_state.card_head = 0u;
    g_pet_nfc_jieli_state.card_tail = 0u;
    g_pet_nfc_jieli_state.card_count = 0u;
    g_pet_nfc_jieli_state.pair_head = 0u;
    g_pet_nfc_jieli_state.pair_tail = 0u;
    g_pet_nfc_jieli_state.pair_count = 0u;
    return PET_RESULT_OK;
}

pet_result_t pet_nfc_jieli_test_inject_card(const pet_nfc_card_t *card)
{
    if (card == 0) {
        return PET_RESULT_INVALID_ARGUMENT;
    }
    if (card->uid_len > PET_PROTOCOL_NFC_UID_MAX) {
        return PET_RESULT_INVALID_ARGUMENT;
    }
    if (g_pet_nfc_jieli_state.card_count >= PET_NFC_JIELI_QUEUE_CAPACITY) {
        return PET_RESULT_BUSY;
    }

    g_pet_nfc_jieli_state.card_queue[g_pet_nfc_jieli_state.card_tail] = *card;
    g_pet_nfc_jieli_state.card_tail =
        (pet_u8_t)((g_pet_nfc_jieli_state.card_tail + 1u) % PET_NFC_JIELI_QUEUE_CAPACITY);
    g_pet_nfc_jieli_state.card_count++;
    return PET_RESULT_OK;
}

pet_result_t pet_nfc_jieli_test_inject_pair(const pet_nfc_pair_t *pair)
{
    if (pair == 0) {
        return PET_RESULT_INVALID_ARGUMENT;
    }
    if (g_pet_nfc_jieli_state.pair_count >= PET_NFC_JIELI_QUEUE_CAPACITY) {
        return PET_RESULT_BUSY;
    }

    g_pet_nfc_jieli_state.pair_queue[g_pet_nfc_jieli_state.pair_tail] = *pair;
    g_pet_nfc_jieli_state.pair_tail =
        (pet_u8_t)((g_pet_nfc_jieli_state.pair_tail + 1u) % PET_NFC_JIELI_QUEUE_CAPACITY);
    g_pet_nfc_jieli_state.pair_count++;
    return PET_RESULT_OK;
}

pet_result_t pet_nfc_jieli_test_inject_pair_payload(const pet_nfc_pair_payload_t *payload)
{
    pet_nfc_pair_t pair;

    if (payload == 0) {
        return PET_RESULT_INVALID_ARGUMENT;
    }
    if (pet_protocol_jieli_validate_nfc_pair_payload(payload) != PET_RESULT_OK) {
        return PET_RESULT_BAD_CRC;
    }

    pet_nfc_jieli_pair_from_payload(payload, &pair);
    return pet_nfc_jieli_test_inject_pair(&pair);
}
#endif

pet_result_t pet_nfc_jieli_self_test(void)
{
#if PET_JIELI_TEST_MODE_ENABLED
    pet_nfc_card_t card;
    pet_nfc_card_t polled_card;
    pet_nfc_pair_t polled_pair;
    pet_nfc_pair_payload_t pair_payload;
    pet_result_t ret;
    pet_u8_t i;

    pet_nfc_jieli_test_set_fake_enabled(PET_FALSE);
    pet_nfc_jieli_test_clear();
    if (pet_nfc_jieli_poll_card(0, &polled_card) != PET_RESULT_NOT_READY) {
        return PET_RESULT_ERROR;
    }

    pet_nfc_jieli_test_set_fake_enabled(PET_TRUE);
    if (pet_nfc_jieli_start_card_scan(0) != PET_RESULT_OK) {
        return PET_RESULT_ERROR;
    }
    for (i = 0u; i < sizeof(card); ++i) {
        ((pet_u8_t *)&card)[i] = 0u;
    }
    card.uid_len = 4u;
    card.uid[0] = 0x01u;
    card.uid[1] = 0x23u;
    card.uid[2] = 0x45u;
    card.uid[3] = 0x67u;
    card.type = 1u;
    card.card_id = 0x1001u;
    if (pet_nfc_jieli_test_inject_card(&card) != PET_RESULT_OK) {
        return PET_RESULT_ERROR;
    }
    if (pet_nfc_jieli_poll_card(0, &polled_card) != PET_RESULT_OK) {
        return PET_RESULT_ERROR;
    }
    if ((polled_card.uid_len != card.uid_len) || (polled_card.card_id != card.card_id)) {
        return PET_RESULT_ERROR;
    }
    if (pet_nfc_jieli_poll_card(0, &polled_card) != PET_RESULT_AGAIN) {
        return PET_RESULT_ERROR;
    }

    ret = pet_protocol_jieli_build_nfc_pair_payload(0x11223344u, 0x5566u,
                                                    0x01020304u, 0x77889900u,
                                                    PET_NFC_PAIR_PROTOCOL_VERSION,
                                                    PET_NFC_PAIR_BLE_SERVICE_ID,
                                                    PET_NFC_PAIR_RESOURCE_VERSION,
                                                    1u, &pair_payload);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    if (pet_protocol_jieli_validate_nfc_pair_payload(&pair_payload) != PET_RESULT_OK) {
        return PET_RESULT_ERROR;
    }
    if (pet_nfc_jieli_test_inject_pair_payload(&pair_payload) != PET_RESULT_OK) {
        return PET_RESULT_ERROR;
    }
    if (pet_nfc_jieli_poll_pair(0, &polled_pair) != PET_RESULT_OK) {
        return PET_RESULT_ERROR;
    }
    if ((polled_pair.peer_device_id[0] != 0x44u) || (polled_pair.role != 1u)) {
        return PET_RESULT_ERROR;
    }
    if (pet_nfc_jieli_poll_pair(0, &polled_pair) != PET_RESULT_AGAIN) {
        return PET_RESULT_ERROR;
    }

    pet_nfc_jieli_test_clear();
    if (pet_nfc_jieli_poll_card(0, &polled_card) != PET_RESULT_AGAIN) {
        return PET_RESULT_ERROR;
    }
    pet_nfc_jieli_test_set_fake_enabled(PET_FALSE);
    return PET_RESULT_OK;
#else
    return PET_RESULT_UNSUPPORTED;
#endif
}
