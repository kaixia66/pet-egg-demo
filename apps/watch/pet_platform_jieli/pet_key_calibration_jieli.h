#ifndef PET_KEY_CALIBRATION_JIELI_H
#define PET_KEY_CALIBRATION_JIELI_H

#include "pet_platform.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PET_KEY_CALIBRATION_JIELI_RECENT_MAX 16u

#define PET_KEY_CAL_JIELI_SDK_KEY_UI_HOME     0x40u
#define PET_KEY_CAL_JIELI_SDK_KEY_UI_SHORTCUT 0x41u
#define PET_KEY_CAL_JIELI_SDK_KEY_UI_MINUS    0x43u
#define PET_KEY_CAL_JIELI_SDK_KEY_UI_PLUS     0x44u

typedef struct {
    pet_u16_t raw_code;
    pet_u8_t raw_event;
    pet_u16_t sdk_key_value;
    pet_key_t mapped_key;
    pet_key_action_t mapped_event;
    pet_u32_t timestamp_ms;
    pet_u32_t hold_ms;
    pet_u16_t repeat_count;
    pet_result_t map_result;
} pet_key_calibration_jieli_record_t;

pet_result_t pet_key_calibration_jieli_sdk_value_to_raw(pet_u16_t sdk_key_value,
                                                        pet_u16_t *out_raw_code);
pet_result_t pet_key_calibration_jieli_raw_to_sdk_value(pet_u16_t raw_code,
                                                        pet_u16_t *out_sdk_key_value);
pet_result_t pet_key_calibration_jieli_record_raw(pet_u16_t raw_code,
                                                  pet_u8_t raw_event,
                                                  pet_u16_t sdk_key_value,
                                                  pet_u32_t timestamp_ms);
pet_result_t pet_key_calibration_jieli_record_sdk_key(pet_u16_t sdk_key_value,
                                                      pet_u8_t raw_event,
                                                      pet_u32_t timestamp_ms);
pet_result_t pet_key_calibration_jieli_get_recent(
    pet_key_calibration_jieli_record_t *out_records,
    pet_u8_t capacity,
    pet_u8_t *out_count);
pet_result_t pet_key_calibration_jieli_clear(void);
pet_result_t pet_key_calibration_jieli_self_test(void);

#ifdef __cplusplus
}
#endif

#endif
