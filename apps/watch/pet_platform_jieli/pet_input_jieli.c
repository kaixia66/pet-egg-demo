#include "pet_platform_jieli_internal.h"

#define PET_INPUT_JIELI_QUEUE_CAPACITY 8u

typedef struct {
    pet_key_event_t events[PET_INPUT_JIELI_QUEUE_CAPACITY];
    pet_u8_t head;
    pet_u8_t tail;
    pet_u8_t count;
} pet_input_jieli_queue_t;

static pet_input_snapshot_t g_pet_input_jieli_snapshot;
static pet_input_jieli_queue_t g_pet_input_jieli_queue;

static void pet_input_jieli_clear_queue(void)
{
    g_pet_input_jieli_queue.head = 0u;
    g_pet_input_jieli_queue.tail = 0u;
    g_pet_input_jieli_queue.count = 0u;
}

static pet_result_t pet_input_jieli_queue_push(const pet_key_event_t *event)
{
    if (event == 0) {
        return PET_RESULT_INVALID_ARGUMENT;
    }
    if (g_pet_input_jieli_queue.count >= PET_INPUT_JIELI_QUEUE_CAPACITY) {
        return PET_RESULT_FULL;
    }

    g_pet_input_jieli_queue.events[g_pet_input_jieli_queue.tail] = *event;
    g_pet_input_jieli_queue.tail++;
    if (g_pet_input_jieli_queue.tail >= PET_INPUT_JIELI_QUEUE_CAPACITY) {
        g_pet_input_jieli_queue.tail = 0u;
    }
    g_pet_input_jieli_queue.count++;
    return PET_RESULT_OK;
}

static pet_result_t pet_input_jieli_queue_pop(pet_key_event_t *event)
{
    if (event == 0) {
        return PET_RESULT_INVALID_ARGUMENT;
    }
    if (g_pet_input_jieli_queue.count == 0u) {
        return PET_RESULT_AGAIN;
    }

    *event = g_pet_input_jieli_queue.events[g_pet_input_jieli_queue.head];
    g_pet_input_jieli_queue.head++;
    if (g_pet_input_jieli_queue.head >= PET_INPUT_JIELI_QUEUE_CAPACITY) {
        g_pet_input_jieli_queue.head = 0u;
    }
    g_pet_input_jieli_queue.count--;
    return PET_RESULT_OK;
}

static void pet_input_jieli_update_snapshot(const pet_key_event_t *event)
{
    pet_u8_t key_index;

    if ((event == 0) || (event->key >= PET_KEY_MAX)) {
        return;
    }

    key_index = (pet_u8_t)event->key;
    g_pet_input_jieli_snapshot.timestamp_ms = event->timestamp_ms;
    g_pet_input_jieli_snapshot.last_key = event->key;
    g_pet_input_jieli_snapshot.last_action = event->type;
    g_pet_input_jieli_snapshot.hold_ms[key_index] = event->hold_ms;
    g_pet_input_jieli_snapshot.repeat_count[key_index] = event->repeat_count;
    g_pet_input_jieli_snapshot.raw_code[key_index] = event->raw_code;

    if ((event->type == PET_KEY_EVENT_DOWN) ||
        (event->type == PET_KEY_EVENT_LONG_PRESS) ||
        (event->type == PET_KEY_EVENT_REPEAT)) {
        g_pet_input_jieli_snapshot.pressed_mask |= PET_KEY_MASK(event->key);
    } else if (event->type == PET_KEY_EVENT_UP) {
        g_pet_input_jieli_snapshot.pressed_mask &= (pet_u8_t)~PET_KEY_MASK(event->key);
    }
}

static pet_result_t pet_input_jieli_raw_code_to_key(pet_u16_t raw_code, pet_key_t *key)
{
    if (key == 0) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    switch (raw_code) {
    case PET_JIELI_RAW_KEY_LEFT_UP:
        *key = PET_KEY_LEFT_UP;
        return PET_RESULT_OK;
    case PET_JIELI_RAW_KEY_RIGHT_DOWN:
        *key = PET_KEY_RIGHT_DOWN;
        return PET_RESULT_OK;
    case PET_JIELI_RAW_KEY_OK:
        *key = PET_KEY_OK;
        return PET_RESULT_OK;
    case PET_JIELI_RAW_KEY_CANCEL:
        *key = PET_KEY_CANCEL;
        return PET_RESULT_OK;
    default:
        break;
    }

    return PET_RESULT_UNSUPPORTED;
}

static pet_result_t pet_input_jieli_raw_event_to_type(pet_u8_t raw_event,
                                                      pet_key_action_t *type,
                                                      pet_u32_t *hold_ms,
                                                      pet_u16_t *repeat_count)
{
    if ((type == 0) || (hold_ms == 0) || (repeat_count == 0)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    *hold_ms = 0u;
    *repeat_count = 0u;
    switch (raw_event) {
    case PET_JIELI_RAW_EVENT_DOWN:
        *type = PET_KEY_EVENT_DOWN;
        return PET_RESULT_OK;
    case PET_JIELI_RAW_EVENT_UP:
        *type = PET_KEY_EVENT_UP;
        return PET_RESULT_OK;
    case PET_JIELI_RAW_EVENT_CLICK:
        *type = PET_KEY_EVENT_CLICK;
        return PET_RESULT_OK;
    case PET_JIELI_RAW_EVENT_LONG:
        *type = PET_KEY_EVENT_LONG_PRESS;
        *hold_ms = 750u;
        return PET_RESULT_OK;
    case PET_JIELI_RAW_EVENT_REPEAT:
        *type = PET_KEY_EVENT_REPEAT;
        *hold_ms = 900u;
        *repeat_count = 1u;
        return PET_RESULT_OK;
    default:
        break;
    }

    return PET_RESULT_UNSUPPORTED;
}

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
    pet_input_jieli_clear_queue();
}

pet_result_t pet_input_jieli_map_raw_event(pet_u16_t raw_code, pet_u8_t raw_event,
                                           pet_u32_t timestamp_ms,
                                           pet_key_event_t *out_event)
{
    pet_key_t key;
    pet_key_action_t type;
    pet_u32_t hold_ms;
    pet_u16_t repeat_count;
    pet_result_t ret;

    if (out_event == 0) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    ret = pet_input_jieli_raw_code_to_key(raw_code, &key);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    ret = pet_input_jieli_raw_event_to_type(raw_event, &type, &hold_ms, &repeat_count);
    if (ret != PET_RESULT_OK) {
        return ret;
    }

    out_event->key = key;
    out_event->type = type;
    out_event->timestamp_ms = timestamp_ms;
    out_event->hold_ms = hold_ms;
    out_event->repeat_count = repeat_count;
    out_event->raw_code = raw_code;
    return PET_RESULT_OK;
}

pet_result_t pet_input_jieli_poll_key_event(void *ctx, pet_key_event_t *event)
{
    (void)ctx;

    if (event == 0) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    /*
     * P3 does not consume the real Jieli key queue. Only the private POC queue is polled here;
     * an empty queue uses PET_RESULT_AGAIN as the shared ABI's "no event yet" result.
     */
    return pet_input_jieli_queue_pop(event);
}

pet_result_t pet_input_jieli_get_snapshot(pet_input_snapshot_t *snapshot)
{
    if (snapshot == 0) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    *snapshot = g_pet_input_jieli_snapshot;
    return PET_RESULT_OK;
}

static pet_result_t pet_input_jieli_push_raw_for_self_test(pet_u16_t raw_code, pet_u8_t raw_event,
                                                           pet_u32_t timestamp_ms)
{
    pet_key_event_t event;
    pet_result_t ret;

    ret = pet_input_jieli_map_raw_event(raw_code, raw_event, timestamp_ms, &event);
    if (ret != PET_RESULT_OK) {
        return ret;
    }

    ret = pet_input_jieli_queue_push(&event);
    if (ret == PET_RESULT_OK) {
        pet_input_jieli_update_snapshot(&event);
    }
    return ret;
}

#if defined(PET_PLATFORM_JIELI_TEST)
pet_result_t pet_input_jieli_test_push_raw(pet_u16_t raw_code, pet_u8_t raw_event,
                                           pet_u32_t timestamp_ms)
{
    return pet_input_jieli_push_raw_for_self_test(raw_code, raw_event, timestamp_ms);
}
#endif

static pet_result_t pet_input_jieli_expect_event(const pet_key_event_t *event,
                                                 pet_key_t key,
                                                 pet_key_action_t type,
                                                 pet_u32_t timestamp_ms,
                                                 pet_u16_t raw_code)
{
    if (event == 0) {
        return PET_RESULT_INVALID_ARGUMENT;
    }
    if ((event->key != key) || (event->type != type) ||
        (event->timestamp_ms != timestamp_ms) || (event->raw_code != raw_code)) {
        return PET_RESULT_ERROR;
    }
    return PET_RESULT_OK;
}

pet_result_t pet_platform_jieli_input_self_test(void)
{
    const pet_platform_t *platform;
    pet_key_event_t event;
    pet_result_t ret;

    platform = pet_platform_jieli_get();
    if ((platform == 0) || (platform->poll_key_event == 0)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    pet_input_jieli_init();
    ret = pet_input_jieli_push_raw_for_self_test(PET_JIELI_RAW_KEY_LEFT_UP,
                                                 PET_JIELI_RAW_EVENT_CLICK, 100u);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    ret = pet_input_jieli_push_raw_for_self_test(PET_JIELI_RAW_KEY_RIGHT_DOWN,
                                                 PET_JIELI_RAW_EVENT_CLICK, 110u);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    ret = pet_input_jieli_push_raw_for_self_test(PET_JIELI_RAW_KEY_OK,
                                                 PET_JIELI_RAW_EVENT_LONG, 120u);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    ret = pet_input_jieli_push_raw_for_self_test(PET_JIELI_RAW_KEY_CANCEL,
                                                 PET_JIELI_RAW_EVENT_REPEAT, 130u);
    if (ret != PET_RESULT_OK) {
        return ret;
    }

    ret = platform->poll_key_event(platform->ctx, &event);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    ret = pet_input_jieli_expect_event(&event, PET_KEY_LEFT_UP, PET_KEY_EVENT_CLICK, 100u,
                                       PET_JIELI_RAW_KEY_LEFT_UP);
    if (ret != PET_RESULT_OK) {
        return ret;
    }

    ret = platform->poll_key_event(platform->ctx, &event);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    ret = pet_input_jieli_expect_event(&event, PET_KEY_RIGHT_DOWN, PET_KEY_EVENT_CLICK, 110u,
                                       PET_JIELI_RAW_KEY_RIGHT_DOWN);
    if (ret != PET_RESULT_OK) {
        return ret;
    }

    ret = platform->poll_key_event(platform->ctx, &event);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    ret = pet_input_jieli_expect_event(&event, PET_KEY_OK, PET_KEY_EVENT_LONG_PRESS, 120u,
                                       PET_JIELI_RAW_KEY_OK);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    if (event.hold_ms == 0u) {
        return PET_RESULT_ERROR;
    }

    ret = platform->poll_key_event(platform->ctx, &event);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    ret = pet_input_jieli_expect_event(&event, PET_KEY_CANCEL, PET_KEY_EVENT_REPEAT, 130u,
                                       PET_JIELI_RAW_KEY_CANCEL);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    if ((event.hold_ms == 0u) || (event.repeat_count == 0u)) {
        return PET_RESULT_ERROR;
    }

    ret = pet_input_jieli_map_raw_event(0xffffu, PET_JIELI_RAW_EVENT_CLICK, 140u, &event);
    if (ret != PET_RESULT_UNSUPPORTED) {
        return PET_RESULT_ERROR;
    }

    ret = platform->poll_key_event(platform->ctx, &event);
    if (ret != PET_RESULT_AGAIN) {
        return PET_RESULT_ERROR;
    }

    return PET_RESULT_OK;
}
