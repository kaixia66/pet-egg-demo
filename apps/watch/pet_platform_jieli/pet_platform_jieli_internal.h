#ifndef PET_PLATFORM_JIELI_INTERNAL_H
#define PET_PLATFORM_JIELI_INTERNAL_H

#include "pet_platform_jieli.h"
#include "pet_protocol_jieli.h"

#if defined(PET_PLATFORM_JIELI_TEST) || defined(PET_DEBUG)
#define PET_JIELI_TEST_MODE_ENABLED 1
#else
#define PET_JIELI_TEST_MODE_ENABLED 0
#endif

#define PET_JIELI_HARDWARE_REV_701N_DEV PET_VERSION_MAKE(7u, 1u, 0u)
#define PET_JIELI_FIRMWARE_VERSION_P2_STUB PET_VERSION_MAKE(0u, 2u, 0u)

#define PET_JIELI_DISPLAY_WIDTH 454u
#define PET_JIELI_DISPLAY_HEIGHT 454u
#define PET_JIELI_DISPLAY_SHAPE PET_SCREEN_SHAPE_CIRCLE
#define PET_JIELI_DISPLAY_SAFE_X 34u
#define PET_JIELI_DISPLAY_SAFE_Y 34u
#define PET_JIELI_DISPLAY_SAFE_W 386u
#define PET_JIELI_DISPLAY_SAFE_H 386u
#define PET_JIELI_DISPLAY_ROTATION PET_DISPLAY_ROTATION_0

#define PET_JIELI_RAW_KEY_OK 0u
#define PET_JIELI_RAW_KEY_LEFT_UP 1u
#define PET_JIELI_RAW_KEY_RIGHT_DOWN 2u
#define PET_JIELI_RAW_KEY_CANCEL 3u

#define PET_JIELI_RAW_EVENT_CLICK 0u
#define PET_JIELI_RAW_EVENT_LONG 1u
#define PET_JIELI_RAW_EVENT_REPEAT 2u
#define PET_JIELI_RAW_EVENT_UP 3u
#define PET_JIELI_RAW_EVENT_DOWN 0x80u

void pet_display_jieli_init(void);
pet_display_owner_t pet_display_jieli_get_owner(void);
pet_result_t pet_display_jieli_get_profile(void *ctx, pet_display_profile_t *profile);
pet_result_t pet_display_jieli_acquire(void *ctx, pet_display_owner_t owner, pet_u32_t timeout_ms);
pet_result_t pet_display_jieli_release(void *ctx, pet_display_owner_t owner);
pet_result_t pet_display_jieli_flush(void *ctx, const pet_display_rect_t *rect,
                                     const void *rgb565_pixels, pet_u32_t stride_bytes);
pet_result_t pet_display_jieli_wait(void *ctx, pet_u32_t timeout_ms);
pet_result_t pet_display_jieli_set_brightness(void *ctx, pet_u8_t percent);
pet_result_t pet_display_jieli_sleep(void *ctx);
pet_result_t pet_display_jieli_wakeup(void *ctx);

void pet_input_jieli_init(void);
pet_result_t pet_input_jieli_poll_key_event(void *ctx, pet_key_event_t *event);
pet_result_t pet_input_jieli_get_snapshot(pet_input_snapshot_t *snapshot);
pet_result_t pet_input_jieli_map_raw_event(pet_u16_t raw_code, pet_u8_t raw_event,
                                           pet_u32_t timestamp_ms,
                                           pet_key_event_t *out_event);
#if defined(PET_PLATFORM_JIELI_TEST)
pet_result_t pet_input_jieli_test_push_raw(pet_u16_t raw_code, pet_u8_t raw_event,
                                           pet_u32_t timestamp_ms);
#endif

void pet_storage_jieli_init(void);
pet_result_t pet_storage_jieli_read(void *ctx, pet_storage_area_t area, pet_u32_t offset,
                                    void *dst, pet_u32_t len);
pet_result_t pet_storage_jieli_write_atomic(void *ctx, pet_storage_area_t area, pet_u32_t offset,
                                            const void *src, pet_u32_t len);

void pet_audio_jieli_init(void);
pet_result_t pet_audio_jieli_play_sfx(void *ctx, pet_u32_t sfx_id, pet_audio_channel_t channel);
pet_result_t pet_audio_jieli_stop(void *ctx, pet_audio_channel_t channel);
pet_result_t pet_audio_jieli_set_volume(pet_u8_t percent);
pet_result_t pet_audio_jieli_set_mute(pet_bool_t muted);
pet_bool_t pet_audio_jieli_is_busy(void);

void pet_ble_jieli_init(void);
pet_result_t pet_ble_jieli_send_packet(void *ctx, const pet_packet_t *packet);
pet_result_t pet_ble_jieli_poll_packet(void *ctx, pet_packet_t *packet);
pet_result_t pet_ble_jieli_self_test(void);
#if PET_JIELI_TEST_MODE_ENABLED
pet_result_t pet_ble_jieli_test_set_loopback_enabled(pet_bool_t enabled);
pet_result_t pet_ble_jieli_test_clear_loopback(void);
#endif

void pet_nfc_jieli_init(void);
pet_result_t pet_nfc_jieli_start_card_scan(void *ctx);
pet_result_t pet_nfc_jieli_start_pair_scan(void *ctx);
pet_result_t pet_nfc_jieli_poll_card(void *ctx, pet_nfc_card_t *card);
pet_result_t pet_nfc_jieli_poll_pair(void *ctx, pet_nfc_pair_t *pair);
pet_result_t pet_nfc_jieli_self_test(void);
#if PET_JIELI_TEST_MODE_ENABLED
pet_result_t pet_nfc_jieli_test_set_fake_enabled(pet_bool_t enabled);
pet_result_t pet_nfc_jieli_test_inject_card(const pet_nfc_card_t *card);
pet_result_t pet_nfc_jieli_test_inject_pair(const pet_nfc_pair_t *pair);
pet_result_t pet_nfc_jieli_test_inject_pair_payload(const pet_nfc_pair_payload_t *payload);
pet_result_t pet_nfc_jieli_test_clear(void);
#endif

void pet_power_jieli_init(void);
pet_result_t pet_power_jieli_get_battery_percent(void *ctx, pet_u8_t *percent);
pet_result_t pet_power_jieli_get_battery_voltage_mv(void *ctx, pet_u16_t *voltage_mv);
pet_bool_t pet_power_jieli_is_low_power(void);

void pet_debug_jieli_init(void);
pet_result_t pet_debug_jieli_self_test(void);
#if PET_JIELI_TEST_MODE_ENABLED
pet_bool_t pet_debug_jieli_get_fake_millis(pet_u32_t *out_millis);
pet_bool_t pet_debug_jieli_get_fake_now_sec(pet_u32_t *out_now_sec);
pet_bool_t pet_debug_jieli_get_fake_battery(pet_u8_t *out_percent, pet_u16_t *out_voltage_mv);
#endif

#endif
