#include "pet_platform_jieli_internal.h"

#if PET_JIELI_TEST_MODE_ENABLED
typedef struct {
    pet_bool_t fake_millis_enabled;
    pet_bool_t fake_now_sec_enabled;
    pet_bool_t fake_battery_enabled;
    pet_u32_t fake_millis;
    pet_u32_t fake_now_sec;
    pet_u8_t fake_battery_percent;
    pet_u16_t fake_battery_voltage_mv;
} pet_debug_jieli_state_t;

static pet_debug_jieli_state_t g_pet_debug_jieli_state;
#endif

void pet_debug_jieli_init(void)
{
}

#if PET_JIELI_TEST_MODE_ENABLED
pet_bool_t pet_debug_jieli_get_fake_millis(pet_u32_t *out_millis)
{
    if ((out_millis == 0) || (g_pet_debug_jieli_state.fake_millis_enabled != PET_TRUE)) {
        return PET_FALSE;
    }
    *out_millis = g_pet_debug_jieli_state.fake_millis;
    return PET_TRUE;
}

pet_bool_t pet_debug_jieli_get_fake_now_sec(pet_u32_t *out_now_sec)
{
    if ((out_now_sec == 0) || (g_pet_debug_jieli_state.fake_now_sec_enabled != PET_TRUE)) {
        return PET_FALSE;
    }
    *out_now_sec = g_pet_debug_jieli_state.fake_now_sec;
    return PET_TRUE;
}

pet_bool_t pet_debug_jieli_get_fake_battery(pet_u8_t *out_percent, pet_u16_t *out_voltage_mv)
{
    if ((out_percent == 0) || (out_voltage_mv == 0) ||
        (g_pet_debug_jieli_state.fake_battery_enabled != PET_TRUE)) {
        return PET_FALSE;
    }
    *out_percent = g_pet_debug_jieli_state.fake_battery_percent;
    *out_voltage_mv = g_pet_debug_jieli_state.fake_battery_voltage_mv;
    return PET_TRUE;
}

pet_result_t pet_debug_jieli_set_fake_millis(pet_u32_t millis)
{
    g_pet_debug_jieli_state.fake_millis = millis;
    g_pet_debug_jieli_state.fake_millis_enabled = PET_TRUE;
    return PET_RESULT_OK;
}

pet_result_t pet_debug_jieli_advance_fake_millis(pet_u32_t delta_ms)
{
    if (g_pet_debug_jieli_state.fake_millis_enabled != PET_TRUE) {
        g_pet_debug_jieli_state.fake_millis = 0u;
        g_pet_debug_jieli_state.fake_millis_enabled = PET_TRUE;
    }
    g_pet_debug_jieli_state.fake_millis += delta_ms;
    return PET_RESULT_OK;
}

pet_result_t pet_debug_jieli_set_fake_now_sec(pet_u32_t now_sec)
{
    g_pet_debug_jieli_state.fake_now_sec = now_sec;
    g_pet_debug_jieli_state.fake_now_sec_enabled = PET_TRUE;
    return PET_RESULT_OK;
}

pet_result_t pet_debug_jieli_set_fake_battery(pet_u8_t percent, pet_u16_t voltage_mv)
{
    if (percent > 100u) {
        return PET_RESULT_INVALID_ARGUMENT;
    }
    g_pet_debug_jieli_state.fake_battery_percent = percent;
    g_pet_debug_jieli_state.fake_battery_voltage_mv = voltage_mv;
    g_pet_debug_jieli_state.fake_battery_enabled = PET_TRUE;
    return PET_RESULT_OK;
}

pet_result_t pet_debug_jieli_enable_ble_loopback(pet_bool_t enabled)
{
    return pet_ble_jieli_test_set_loopback_enabled(enabled);
}

pet_result_t pet_debug_jieli_inject_ble_packet(const pet_packet_t *packet)
{
    pet_result_t ret;

    ret = pet_ble_jieli_test_set_loopback_enabled(PET_TRUE);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    return pet_ble_jieli_send_packet(0, packet);
}

pet_result_t pet_debug_jieli_inject_nfc_card(const pet_nfc_card_t *card)
{
    pet_result_t ret;

    ret = pet_nfc_jieli_test_set_fake_enabled(PET_TRUE);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    return pet_nfc_jieli_test_inject_card(card);
}

pet_result_t pet_debug_jieli_inject_nfc_pair_payload(const pet_nfc_pair_payload_t *payload)
{
    pet_result_t ret;

    ret = pet_nfc_jieli_test_set_fake_enabled(PET_TRUE);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    return pet_nfc_jieli_test_inject_pair_payload(payload);
}

pet_result_t pet_debug_jieli_clear_all(void)
{
    g_pet_debug_jieli_state.fake_millis_enabled = PET_FALSE;
    g_pet_debug_jieli_state.fake_now_sec_enabled = PET_FALSE;
    g_pet_debug_jieli_state.fake_battery_enabled = PET_FALSE;
    g_pet_debug_jieli_state.fake_millis = 0u;
    g_pet_debug_jieli_state.fake_now_sec = 0u;
    g_pet_debug_jieli_state.fake_battery_percent = 0u;
    g_pet_debug_jieli_state.fake_battery_voltage_mv = 0u;
    pet_ble_jieli_test_set_loopback_enabled(PET_FALSE);
    pet_ble_jieli_test_clear_loopback();
    pet_nfc_jieli_test_set_fake_enabled(PET_FALSE);
    pet_nfc_jieli_test_clear();
    return PET_RESULT_OK;
}
#endif

#if defined(PET_DEBUG)
pet_result_t pet_debug_inject_key_event(const pet_platform_t *platform, const pet_key_event_t *event)
{
    (void)platform;
    (void)event;
    return PET_RESULT_UNSUPPORTED;
}

pet_result_t pet_debug_inject_packet(const pet_platform_t *platform, const pet_packet_t *packet)
{
    (void)platform;
    return pet_debug_jieli_inject_ble_packet(packet);
}

pet_result_t pet_debug_inject_nfc_card(const pet_platform_t *platform, const pet_nfc_card_t *card)
{
    (void)platform;
    return pet_debug_jieli_inject_nfc_card(card);
}
#endif

pet_result_t pet_debug_jieli_self_test(void)
{
#if PET_JIELI_TEST_MODE_ENABLED
    pet_u8_t percent = 0u;
    pet_u16_t voltage = 0u;
    pet_u32_t millis = 0u;
    pet_u32_t now_sec = 0u;
    pet_u8_t payload[1] = {0x77u};
    pet_packet_t packet;
    pet_packet_t polled;
    pet_nfc_card_t card;
    pet_nfc_card_t polled_card;
    pet_u32_t i;

    pet_debug_jieli_clear_all();
    if (pet_debug_jieli_set_fake_battery(66u, 3800u) != PET_RESULT_OK) {
        return PET_RESULT_ERROR;
    }
    if (pet_debug_jieli_get_fake_battery(&percent, &voltage) != PET_TRUE) {
        return PET_RESULT_ERROR;
    }
    if ((percent != 66u) || (voltage != 3800u)) {
        return PET_RESULT_ERROR;
    }

    pet_debug_jieli_set_fake_millis(100u);
    pet_debug_jieli_advance_fake_millis(23u);
    if (pet_debug_jieli_get_fake_millis(&millis) != PET_TRUE) {
        return PET_RESULT_ERROR;
    }
    if (millis != 123u) {
        return PET_RESULT_ERROR;
    }
    pet_debug_jieli_set_fake_now_sec(456u);
    if (pet_debug_jieli_get_fake_now_sec(&now_sec) != PET_TRUE) {
        return PET_RESULT_ERROR;
    }
    if (now_sec != 456u) {
        return PET_RESULT_ERROR;
    }

    if (pet_protocol_jieli_build_packet(PET_PACKET_PONG, 2u, 1u, payload, sizeof(payload),
                                        PET_PACKET_FLAG_IS_ACK, &packet) != PET_RESULT_OK) {
        return PET_RESULT_ERROR;
    }
    if (pet_debug_jieli_inject_ble_packet(&packet) != PET_RESULT_OK) {
        return PET_RESULT_ERROR;
    }
    if (pet_ble_jieli_poll_packet(0, &polled) != PET_RESULT_OK) {
        return PET_RESULT_ERROR;
    }
    if (polled.seq != packet.seq) {
        return PET_RESULT_ERROR;
    }

    for (i = 0u; i < sizeof(card); ++i) {
        ((pet_u8_t *)&card)[i] = 0u;
    }
    card.uid_len = 4u;
    card.uid[0] = 0xaau;
    card.uid[1] = 0xbbu;
    card.uid[2] = 0xccu;
    card.uid[3] = 0xddu;
    card.type = 2u;
    card.card_id = 0x2222u;
    if (pet_debug_jieli_inject_nfc_card(&card) != PET_RESULT_OK) {
        return PET_RESULT_ERROR;
    }
    if (pet_nfc_jieli_poll_card(0, &polled_card) != PET_RESULT_OK) {
        return PET_RESULT_ERROR;
    }
    if (polled_card.card_id != card.card_id) {
        return PET_RESULT_ERROR;
    }

    pet_debug_jieli_clear_all();
    if (pet_debug_jieli_get_fake_battery(&percent, &voltage) != PET_FALSE) {
        return PET_RESULT_ERROR;
    }
    return PET_RESULT_OK;
#else
    return PET_RESULT_UNSUPPORTED;
#endif
}
