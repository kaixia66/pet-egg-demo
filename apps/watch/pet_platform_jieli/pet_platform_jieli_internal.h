#ifndef PET_PLATFORM_JIELI_INTERNAL_H
#define PET_PLATFORM_JIELI_INTERNAL_H

#include "pet_platform_jieli.h"

#define PET_JIELI_HARDWARE_REV_701N_DEV PET_VERSION_MAKE(7u, 1u, 0u)
#define PET_JIELI_FIRMWARE_VERSION_P2_STUB PET_VERSION_MAKE(0u, 2u, 0u)

void pet_display_jieli_init(void);
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

void pet_nfc_jieli_init(void);
pet_result_t pet_nfc_jieli_start_card_scan(void *ctx);
pet_result_t pet_nfc_jieli_start_pair_scan(void *ctx);
pet_result_t pet_nfc_jieli_poll_card(void *ctx, pet_nfc_card_t *card);
pet_result_t pet_nfc_jieli_poll_pair(void *ctx, pet_nfc_pair_t *pair);

void pet_power_jieli_init(void);
pet_result_t pet_power_jieli_get_battery_percent(void *ctx, pet_u8_t *percent);
pet_result_t pet_power_jieli_get_battery_voltage_mv(void *ctx, pet_u16_t *voltage_mv);
pet_bool_t pet_power_jieli_is_low_power(void);

void pet_debug_jieli_init(void);

#endif
