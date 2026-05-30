#include "pet_platform_jieli_internal.h"

static pet_input_snapshot_t g_pet_input_jieli_snapshot;

void pet_input_jieli_init(void)
{
    pet_u8_t i;

    g_pet_input_jieli_snapshot.timestamp_ms = 0u;
    g_pet_input_jieli_snapshot.pressed_mask = 0u;
    g_pet_input_jieli_snapshot.last_key = PET_KEY_CANCEL;
    g_pet_input_jieli_snapshot.last_action = PET_KEY_EVENT_UP;
    for (i = 0u; i < PET_KEY_MAX; ++i) {
        g_pet_input_jieli_snapshot.hold_ms[i] = 0u;
        g_pet_input_jieli_snapshot.repeat_count[i] = 0u;
        g_pet_input_jieli_snapshot.raw_code[i] = 0u;
    }
}

pet_result_t pet_input_jieli_poll_key_event(void *ctx, pet_key_event_t *event)
{
    (void)ctx;

    if (event == 0) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    event->key = PET_KEY_CANCEL;
    event->type = PET_KEY_EVENT_UP;
    event->timestamp_ms = 0u;
    event->hold_ms = 0u;
    event->repeat_count = 0u;
    event->raw_code = 0u;
    /* TODO(P3): bridge raw Jieli key messages to PetKeyEvent without changing MVP-A LVGL input. */
    return PET_RESULT_NOT_READY;
}

pet_result_t pet_input_jieli_get_snapshot(pet_input_snapshot_t *snapshot)
{
    if (snapshot == 0) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    *snapshot = g_pet_input_jieli_snapshot;
    return PET_RESULT_OK;
}
