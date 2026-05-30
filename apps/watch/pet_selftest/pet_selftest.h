#ifndef PET_SELFTEST_H
#define PET_SELFTEST_H

#include "pet_platform.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    PET_SELFTEST_SHARED_INTERFACE = 0,
    PET_SELFTEST_PLATFORM_HAL,
    PET_SELFTEST_DISPLAY_PROFILE,
    PET_SELFTEST_DISPLAY_OWNER,
    PET_SELFTEST_DISPLAY_FLUSH_OWNER,
    PET_SELFTEST_DISPLAY_TINY_FLUSH_POC,
    PET_SELFTEST_INPUT_MAPPING,
    PET_SELFTEST_RENDER_OWNER_BOUNDARY,
    PET_SELFTEST_RESOURCE_MANIFEST,
    PET_SELFTEST_PET2D_RESOURCE_PROBE,
    PET_SELFTEST_SAVE_TRANSACTION,
    PET_SELFTEST_PROTOCOL_PACKET,
    PET_SELFTEST_BLE_LOOPBACK,
    PET_SELFTEST_NFC_FAKE,
    PET_SELFTEST_DEBUG_INJECTION,
    PET_SELFTEST_MAX
} pet_selftest_case_t;

typedef struct {
    pet_u16_t total;
    pet_u16_t passed;
    pet_u16_t failed;
    pet_u16_t skipped;
    pet_u32_t failed_mask;
    pet_u32_t skipped_mask;
} pet_selftest_summary_t;

typedef struct {
    pet_u8_t has_shared_interface;
    pet_u8_t has_platform_hal;
    pet_u8_t has_display_profile;
    pet_u8_t has_display_owner;
    pet_u8_t has_display_flush_owner_guard;
    pet_u8_t has_tiny_lcd_flush_poc_gate;
    pet_u8_t has_input_mapping;
    pet_u8_t has_render_owner_boundary;
    pet_u8_t has_resource_manifest_adapter;
    pet_u8_t has_save_transaction_adapter;
    pet_u8_t has_protocol_helper;
    pet_u8_t has_ble_loopback_test;
    pet_u8_t has_nfc_fake_test;
    pet_u8_t has_debug_injection_test;

    pet_u8_t real_lcd_flush_enabled;
    pet_u8_t real_key_queue_enabled;
    pet_u8_t real_flash_storage_enabled;
    pet_u8_t real_ble_enabled;
    pet_u8_t real_nfc_enabled;
    pet_u8_t pet2d_runtime_enabled;
} pet_platform_capability_snapshot_t;

pet_result_t pet_selftest_run_case(pet_selftest_case_t test_case);
pet_result_t pet_selftest_run_all(pet_selftest_summary_t *out_summary);
const char *pet_selftest_case_name(pet_selftest_case_t test_case);
pet_result_t pet_selftest_get_capability_snapshot(pet_platform_capability_snapshot_t *out_snapshot);

#ifdef __cplusplus
}
#endif

#endif
