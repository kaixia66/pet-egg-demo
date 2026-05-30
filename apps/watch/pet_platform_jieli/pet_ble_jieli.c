#include "pet_platform_jieli_internal.h"

void pet_ble_jieli_init(void)
{
}

pet_result_t pet_ble_jieli_send_packet(void *ctx, const pet_packet_t *packet)
{
    (void)ctx;

    if (packet == 0) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    /* TODO(P7): bridge pet_packet_t to the selected Jieli BLE transport without loopback. */
    return PET_RESULT_NOT_READY;
}

pet_result_t pet_ble_jieli_poll_packet(void *ctx, pet_packet_t *packet)
{
    (void)ctx;

    if (packet == 0) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    return PET_RESULT_NOT_READY;
}
