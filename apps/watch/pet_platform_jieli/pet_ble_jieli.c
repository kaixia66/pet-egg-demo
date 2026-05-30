#include "pet_platform_jieli_internal.h"

#define PET_BLE_JIELI_LOOPBACK_CAPACITY 4u

#if PET_JIELI_TEST_MODE_ENABLED
typedef struct {
    pet_packet_t queue[PET_BLE_JIELI_LOOPBACK_CAPACITY];
    pet_u8_t head;
    pet_u8_t tail;
    pet_u8_t count;
    pet_bool_t loopback_enabled;
} pet_ble_jieli_state_t;

static pet_ble_jieli_state_t g_pet_ble_jieli_state;

static pet_result_t pet_ble_jieli_enqueue_packet(const pet_packet_t *packet)
{
    if (packet == 0) {
        return PET_RESULT_INVALID_ARGUMENT;
    }
    if (g_pet_ble_jieli_state.count >= PET_BLE_JIELI_LOOPBACK_CAPACITY) {
        return PET_RESULT_BUSY;
    }

    g_pet_ble_jieli_state.queue[g_pet_ble_jieli_state.tail] = *packet;
    g_pet_ble_jieli_state.tail =
        (pet_u8_t)((g_pet_ble_jieli_state.tail + 1u) % PET_BLE_JIELI_LOOPBACK_CAPACITY);
    g_pet_ble_jieli_state.count++;
    return PET_RESULT_OK;
}

static pet_result_t pet_ble_jieli_dequeue_packet(pet_packet_t *packet)
{
    if (packet == 0) {
        return PET_RESULT_INVALID_ARGUMENT;
    }
    if (g_pet_ble_jieli_state.count == 0u) {
        return PET_RESULT_AGAIN;
    }

    *packet = g_pet_ble_jieli_state.queue[g_pet_ble_jieli_state.head];
    g_pet_ble_jieli_state.head =
        (pet_u8_t)((g_pet_ble_jieli_state.head + 1u) % PET_BLE_JIELI_LOOPBACK_CAPACITY);
    g_pet_ble_jieli_state.count--;
    return PET_RESULT_OK;
}
#endif

void pet_ble_jieli_init(void)
{
}

pet_result_t pet_ble_jieli_send_packet(void *ctx, const pet_packet_t *packet)
{
    (void)ctx;

    if (packet == 0) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

#if PET_JIELI_TEST_MODE_ENABLED
    if (g_pet_ble_jieli_state.loopback_enabled == PET_TRUE) {
        if (pet_protocol_jieli_validate_packet(packet) != PET_PACKET_STATUS_OK) {
            return PET_RESULT_BAD_CRC;
        }
        return pet_ble_jieli_enqueue_packet(packet);
    }
#endif

    /* TODO(P7): bridge pet_packet_t to the selected Jieli BLE transport without loopback. */
    return PET_RESULT_NOT_READY;
}

pet_result_t pet_ble_jieli_poll_packet(void *ctx, pet_packet_t *packet)
{
    (void)ctx;

    if (packet == 0) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

#if PET_JIELI_TEST_MODE_ENABLED
    if (g_pet_ble_jieli_state.loopback_enabled == PET_TRUE) {
        return pet_ble_jieli_dequeue_packet(packet);
    }
#endif

    return PET_RESULT_NOT_READY;
}

#if PET_JIELI_TEST_MODE_ENABLED
pet_result_t pet_ble_jieli_test_set_loopback_enabled(pet_bool_t enabled)
{
    g_pet_ble_jieli_state.loopback_enabled = enabled ? PET_TRUE : PET_FALSE;
    return PET_RESULT_OK;
}

pet_result_t pet_ble_jieli_test_clear_loopback(void)
{
    g_pet_ble_jieli_state.head = 0u;
    g_pet_ble_jieli_state.tail = 0u;
    g_pet_ble_jieli_state.count = 0u;
    return PET_RESULT_OK;
}
#endif

pet_result_t pet_ble_jieli_self_test(void)
{
#if PET_JIELI_TEST_MODE_ENABLED
    pet_u8_t payload[2] = {0x42u, 0x24u};
    pet_packet_t packet;
    pet_packet_t polled;
    pet_result_t ret;
    pet_u8_t i;

    pet_ble_jieli_test_set_loopback_enabled(PET_FALSE);
    pet_ble_jieli_test_clear_loopback();
    ret = pet_protocol_jieli_build_packet(PET_PACKET_PING, 9u, 5u, payload, sizeof(payload),
                                          PET_PACKET_FLAG_REQUIRES_ACK, &packet);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    if (pet_ble_jieli_send_packet(0, &packet) != PET_RESULT_NOT_READY) {
        return PET_RESULT_ERROR;
    }

    pet_ble_jieli_test_set_loopback_enabled(PET_TRUE);
    if (pet_ble_jieli_send_packet(0, &packet) != PET_RESULT_OK) {
        return PET_RESULT_ERROR;
    }
    if (pet_ble_jieli_poll_packet(0, &polled) != PET_RESULT_OK) {
        return PET_RESULT_ERROR;
    }
    if ((polled.seq != packet.seq) || (polled.ack != packet.ack) ||
        (polled.crc16 != packet.crc16) || (polled.payload[0] != payload[0])) {
        return PET_RESULT_ERROR;
    }
    if (pet_ble_jieli_poll_packet(0, &polled) != PET_RESULT_AGAIN) {
        return PET_RESULT_ERROR;
    }

    pet_ble_jieli_test_clear_loopback();
    for (i = 0u; i < PET_BLE_JIELI_LOOPBACK_CAPACITY; ++i) {
        if (pet_ble_jieli_send_packet(0, &packet) != PET_RESULT_OK) {
            return PET_RESULT_ERROR;
        }
    }
    if (pet_ble_jieli_send_packet(0, &packet) != PET_RESULT_BUSY) {
        return PET_RESULT_ERROR;
    }
    pet_ble_jieli_test_clear_loopback();
    packet.crc16 ^= 1u;
    if (pet_ble_jieli_send_packet(0, &packet) != PET_RESULT_BAD_CRC) {
        return PET_RESULT_ERROR;
    }
    if (pet_ble_jieli_poll_packet(0, &polled) != PET_RESULT_AGAIN) {
        return PET_RESULT_ERROR;
    }

    pet_ble_jieli_test_set_loopback_enabled(PET_FALSE);
    pet_ble_jieli_test_clear_loopback();
    return PET_RESULT_OK;
#else
    return PET_RESULT_UNSUPPORTED;
#endif
}
