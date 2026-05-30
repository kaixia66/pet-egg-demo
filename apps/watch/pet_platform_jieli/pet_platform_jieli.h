#ifndef PET_PLATFORM_JIELI_H
#define PET_PLATFORM_JIELI_H

#include "pet_platform.h"

#ifdef __cplusplus
extern "C" {
#endif

void pet_platform_jieli_init(void);
const pet_platform_t *pet_platform_jieli_get(void);

typedef struct {
    pet_u32_t flush_call_count;
    pet_u32_t rejected_count;
    pet_u32_t busy_count;
    pet_u32_t total_requested_pixels;
    pet_i16_t last_x;
    pet_i16_t last_y;
    pet_u16_t last_w;
    pet_u16_t last_h;
    pet_u16_t last_pitch_pixels;
    pet_u8_t last_mode;
    pet_u8_t last_owner;
    pet_u8_t real_flush_enabled;
    pet_u8_t busy;
} pet_display_jieli_flush_stats_t;

pet_display_owner_t pet_display_jieli_get_owner(void);
pet_result_t pet_display_jieli_get_flush_stats(pet_display_jieli_flush_stats_t *out_stats);
pet_result_t pet_display_jieli_reset_flush_stats(void);
pet_result_t pet_display_jieli_owner_self_test(void);
pet_result_t pet_display_jieli_flush_self_test(void);
pet_result_t pet_display_jieli_tiny_flush_poc(void);
pet_result_t pet_platform_jieli_display_self_test(void);
pet_result_t pet_platform_jieli_input_self_test(void);
pet_result_t pet_ble_jieli_self_test(void);
pet_result_t pet_nfc_jieli_self_test(void);
pet_result_t pet_debug_jieli_self_test(void);

#if defined(PET_PLATFORM_JIELI_TEST) || defined(PET_DEBUG)
pet_result_t pet_debug_jieli_set_fake_millis(pet_u32_t millis);
pet_result_t pet_debug_jieli_advance_fake_millis(pet_u32_t delta_ms);
pet_result_t pet_debug_jieli_set_fake_now_sec(pet_u32_t now_sec);
pet_result_t pet_debug_jieli_set_fake_battery(pet_u8_t percent, pet_u16_t voltage_mv);
pet_result_t pet_debug_jieli_enable_ble_loopback(pet_bool_t enabled);
pet_result_t pet_debug_jieli_inject_ble_packet(const pet_packet_t *packet);
pet_result_t pet_debug_jieli_inject_nfc_card(const pet_nfc_card_t *card);
pet_result_t pet_debug_jieli_inject_nfc_pair_payload(const pet_nfc_pair_payload_t *payload);
pet_result_t pet_debug_jieli_clear_all(void);
#endif

#ifdef __cplusplus
}
#endif

#endif
