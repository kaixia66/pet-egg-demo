#include "pet_key_calibration_jieli.h"
#include "pet_platform_jieli_internal.h"

typedef struct {
    pet_key_calibration_jieli_record_t records[PET_KEY_CALIBRATION_JIELI_RECENT_MAX];
    pet_u8_t next;
    pet_u8_t count;
} pet_key_calibration_jieli_ring_t;

static pet_key_calibration_jieli_ring_t g_pet_key_calibration_jieli_ring;

pet_result_t pet_key_calibration_jieli_raw_to_sdk_value(pet_u16_t raw_code,
                                                        pet_u16_t *out_sdk_key_value)
{
    if (out_sdk_key_value == 0) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    switch (raw_code) {
    case PET_JIELI_RAW_KEY_OK:
        *out_sdk_key_value = PET_KEY_CAL_JIELI_SDK_KEY_UI_HOME;
        return PET_RESULT_OK;
    case PET_JIELI_RAW_KEY_LEFT_UP:
        *out_sdk_key_value = PET_KEY_CAL_JIELI_SDK_KEY_UI_PLUS;
        return PET_RESULT_OK;
    case PET_JIELI_RAW_KEY_RIGHT_DOWN:
        *out_sdk_key_value = PET_KEY_CAL_JIELI_SDK_KEY_UI_MINUS;
        return PET_RESULT_OK;
    case PET_JIELI_RAW_KEY_CANCEL:
        *out_sdk_key_value = PET_KEY_CAL_JIELI_SDK_KEY_UI_SHORTCUT;
        return PET_RESULT_OK;
    default:
        break;
    }

    return PET_RESULT_UNSUPPORTED;
}

pet_result_t pet_key_calibration_jieli_sdk_value_to_raw(pet_u16_t sdk_key_value,
                                                        pet_u16_t *out_raw_code)
{
    if (out_raw_code == 0) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    switch (sdk_key_value) {
    case PET_KEY_CAL_JIELI_SDK_KEY_UI_HOME:
        *out_raw_code = PET_JIELI_RAW_KEY_OK;
        return PET_RESULT_OK;
    case PET_KEY_CAL_JIELI_SDK_KEY_UI_PLUS:
        *out_raw_code = PET_JIELI_RAW_KEY_LEFT_UP;
        return PET_RESULT_OK;
    case PET_KEY_CAL_JIELI_SDK_KEY_UI_MINUS:
        *out_raw_code = PET_JIELI_RAW_KEY_RIGHT_DOWN;
        return PET_RESULT_OK;
    case PET_KEY_CAL_JIELI_SDK_KEY_UI_SHORTCUT:
        *out_raw_code = PET_JIELI_RAW_KEY_CANCEL;
        return PET_RESULT_OK;
    default:
        break;
    }

    return PET_RESULT_UNSUPPORTED;
}

pet_result_t pet_key_calibration_jieli_clear(void)
{
    pet_u8_t i;

    g_pet_key_calibration_jieli_ring.next = 0u;
    g_pet_key_calibration_jieli_ring.count = 0u;
    for (i = 0u; i < PET_KEY_CALIBRATION_JIELI_RECENT_MAX; i++) {
        g_pet_key_calibration_jieli_ring.records[i].raw_code = 0u;
        g_pet_key_calibration_jieli_ring.records[i].raw_event = 0u;
        g_pet_key_calibration_jieli_ring.records[i].sdk_key_value = 0u;
        g_pet_key_calibration_jieli_ring.records[i].mapped_key = PET_KEY_MAX;
        g_pet_key_calibration_jieli_ring.records[i].mapped_event = PET_KEY_EVENT_UP;
        g_pet_key_calibration_jieli_ring.records[i].timestamp_ms = 0u;
        g_pet_key_calibration_jieli_ring.records[i].hold_ms = 0u;
        g_pet_key_calibration_jieli_ring.records[i].repeat_count = 0u;
        g_pet_key_calibration_jieli_ring.records[i].map_result = PET_RESULT_NOT_READY;
    }

    return PET_RESULT_OK;
}

pet_result_t pet_key_calibration_jieli_record_raw(pet_u16_t raw_code,
                                                  pet_u8_t raw_event,
                                                  pet_u16_t sdk_key_value,
                                                  pet_u32_t timestamp_ms)
{
    pet_key_calibration_jieli_record_t *record;
    pet_key_event_t mapped_event;
    pet_result_t ret;

    record = &g_pet_key_calibration_jieli_ring.records[g_pet_key_calibration_jieli_ring.next];
    record->raw_code = raw_code;
    record->raw_event = raw_event;
    record->sdk_key_value = sdk_key_value;
    record->timestamp_ms = timestamp_ms;
    record->hold_ms = 0u;
    record->repeat_count = 0u;
    record->mapped_key = PET_KEY_MAX;
    record->mapped_event = PET_KEY_EVENT_UP;

    ret = pet_input_jieli_map_raw_event(raw_code, raw_event, timestamp_ms, &mapped_event);
    record->map_result = ret;
    if (ret == PET_RESULT_OK) {
        record->mapped_key = mapped_event.key;
        record->mapped_event = mapped_event.type;
        record->hold_ms = mapped_event.hold_ms;
        record->repeat_count = mapped_event.repeat_count;
    }

    g_pet_key_calibration_jieli_ring.next++;
    if (g_pet_key_calibration_jieli_ring.next >= PET_KEY_CALIBRATION_JIELI_RECENT_MAX) {
        g_pet_key_calibration_jieli_ring.next = 0u;
    }
    if (g_pet_key_calibration_jieli_ring.count < PET_KEY_CALIBRATION_JIELI_RECENT_MAX) {
        g_pet_key_calibration_jieli_ring.count++;
    }

    return ret;
}

pet_result_t pet_key_calibration_jieli_record_sdk_key(pet_u16_t sdk_key_value,
                                                      pet_u8_t raw_event,
                                                      pet_u32_t timestamp_ms)
{
    pet_u16_t raw_code;
    pet_result_t ret;

    ret = pet_key_calibration_jieli_sdk_value_to_raw(sdk_key_value, &raw_code);
    if (ret != PET_RESULT_OK) {
        (void)pet_key_calibration_jieli_record_raw(0xffffu, raw_event,
                                                   sdk_key_value, timestamp_ms);
        return ret;
    }

    return pet_key_calibration_jieli_record_raw(raw_code, raw_event,
                                               sdk_key_value, timestamp_ms);
}

pet_result_t pet_key_calibration_jieli_get_recent(
    pet_key_calibration_jieli_record_t *out_records,
    pet_u8_t capacity,
    pet_u8_t *out_count)
{
    pet_u8_t i;
    pet_u8_t start;

    if ((out_records == 0) || (out_count == 0)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }
    if (capacity < g_pet_key_calibration_jieli_ring.count) {
        return PET_RESULT_BUFFER_TOO_SMALL;
    }

    start = (pet_u8_t)((g_pet_key_calibration_jieli_ring.next +
                        PET_KEY_CALIBRATION_JIELI_RECENT_MAX -
                        g_pet_key_calibration_jieli_ring.count) %
                       PET_KEY_CALIBRATION_JIELI_RECENT_MAX);
    for (i = 0u; i < g_pet_key_calibration_jieli_ring.count; i++) {
        pet_u8_t index = (pet_u8_t)((start + i) % PET_KEY_CALIBRATION_JIELI_RECENT_MAX);
        out_records[i] = g_pet_key_calibration_jieli_ring.records[index];
    }
    *out_count = g_pet_key_calibration_jieli_ring.count;
    return PET_RESULT_OK;
}

pet_result_t pet_key_calibration_jieli_self_test(void)
{
    pet_key_calibration_jieli_record_t records[PET_KEY_CALIBRATION_JIELI_RECENT_MAX];
    pet_u16_t raw_code;
    pet_u16_t sdk_key_value;
    pet_u8_t count;
    pet_result_t ret;

    if (pet_key_calibration_jieli_clear() != PET_RESULT_OK) {
        return PET_RESULT_ERROR;
    }
    if (pet_key_calibration_jieli_sdk_value_to_raw(PET_KEY_CAL_JIELI_SDK_KEY_UI_PLUS,
                                                   &raw_code) != PET_RESULT_OK) {
        return PET_RESULT_ERROR;
    }
    if (raw_code != PET_JIELI_RAW_KEY_LEFT_UP) {
        return PET_RESULT_ERROR;
    }
    if (pet_key_calibration_jieli_raw_to_sdk_value(PET_JIELI_RAW_KEY_RIGHT_DOWN,
                                                   &sdk_key_value) != PET_RESULT_OK) {
        return PET_RESULT_ERROR;
    }
    if (sdk_key_value != PET_KEY_CAL_JIELI_SDK_KEY_UI_MINUS) {
        return PET_RESULT_ERROR;
    }

    ret = pet_key_calibration_jieli_record_raw(PET_JIELI_RAW_KEY_LEFT_UP,
                                               PET_JIELI_RAW_EVENT_CLICK,
                                               PET_KEY_CAL_JIELI_SDK_KEY_UI_PLUS,
                                               100u);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    ret = pet_key_calibration_jieli_record_sdk_key(PET_KEY_CAL_JIELI_SDK_KEY_UI_HOME,
                                                   PET_JIELI_RAW_EVENT_LONG,
                                                   120u);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    ret = pet_key_calibration_jieli_record_sdk_key(0xeeeeu, PET_JIELI_RAW_EVENT_CLICK, 130u);
    if (ret != PET_RESULT_UNSUPPORTED) {
        return PET_RESULT_ERROR;
    }
    if (pet_key_calibration_jieli_get_recent(records, PET_KEY_CALIBRATION_JIELI_RECENT_MAX,
                                             &count) != PET_RESULT_OK) {
        return PET_RESULT_ERROR;
    }
    if (count != 3u) {
        return PET_RESULT_ERROR;
    }
    if ((records[0].mapped_key != PET_KEY_LEFT_UP) ||
        (records[0].mapped_event != PET_KEY_EVENT_CLICK) ||
        (records[0].map_result != PET_RESULT_OK)) {
        return PET_RESULT_ERROR;
    }
    if ((records[1].mapped_key != PET_KEY_OK) ||
        (records[1].mapped_event != PET_KEY_EVENT_LONG_PRESS) ||
        (records[1].hold_ms == 0u)) {
        return PET_RESULT_ERROR;
    }
    if (records[2].map_result != PET_RESULT_UNSUPPORTED) {
        return PET_RESULT_ERROR;
    }

    return PET_RESULT_OK;
}
