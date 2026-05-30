#include "pet_selftest.h"

#include "pet2d_boundary.h"
#include "pet2d_dirty_rect_poc.h"
#include "pet2d_minimal_visual.h"
#include "pet2d_movement_poc.h"
#include "pet2d_resource_sprite_poc.h"
#include "pet_protocol_jieli.h"
#include "pet_key_calibration_jieli.h"
#include "pet_resource_jieli.h"
#include "pet_resource_jieli_real.h"
#include "pet_save_jieli.h"
#include "pet_platform_jieli.h"

typedef pet_result_t (*pet_selftest_fn_t)(void);

static pet_result_t pet_selftest_shared_interface(void)
{
    if ((PET_PROTOCOL_VERSION_MAJOR != 1u) ||
        (PET_PACKET_MAGIC != 0xE6u) ||
        (PET_PACKET_MAX_PAYLOAD != 64u) ||
        (PET_PACKET_SERIALIZED_HEADER_SIZE != 10u) ||
        (sizeof(pet_packet_t) != PET_PACKET_MAX_SERIALIZED_SIZE) ||
        (sizeof(pet_nfc_pair_payload_t) != PET_NFC_PAIR_PAYLOAD_SERIALIZED_SIZE)) {
        return PET_RESULT_ERROR;
    }

    return PET_RESULT_OK;
}

static pet_result_t pet_selftest_platform_hal(void)
{
    const pet_platform_t *platform = pet_platform_jieli_get();

    if (platform == 0) {
        return PET_RESULT_INVALID_ARGUMENT;
    }
    if ((platform->millis == 0) ||
        (platform->now_sec == 0) ||
        (platform->get_device_identity == 0) ||
        (platform->get_display_profile == 0) ||
        (platform->poll_key_event == 0) ||
        (platform->display_acquire == 0) ||
        (platform->display_release == 0) ||
        (platform->display_flush == 0) ||
        (platform->display_wait == 0) ||
        (platform->display_set_brightness == 0) ||
        (platform->display_sleep == 0) ||
        (platform->display_wakeup == 0) ||
        (platform->storage_read == 0) ||
        (platform->storage_write_atomic == 0) ||
        (platform->audio_play_sfx == 0) ||
        (platform->audio_stop == 0) ||
        (platform->ble_send_packet == 0) ||
        (platform->ble_poll_packet == 0) ||
        (platform->nfc_start_card_scan == 0) ||
        (platform->nfc_start_pair_scan == 0) ||
        (platform->nfc_poll_card == 0) ||
        (platform->nfc_poll_pair == 0) ||
        (platform->power_get_battery_percent == 0) ||
        (platform->power_get_battery_voltage_mv == 0)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    return PET_RESULT_OK;
}

static pet_result_t pet_selftest_display_tiny_flush_poc_gate(void)
{
    /*
     * The real tiny LCD probe is manual-only. run_all records this case as skipped
     * so PET_PLATFORM_JIELI_TEST cannot unexpectedly write the panel.
     */
    return PET_RESULT_UNSUPPORTED;
}

static pet_result_t pet_selftest_pet2d_minimal_real_flush_gate(void)
{
    pet_result_t ret = pet2d_minimal_visual_self_test();

    if (ret != PET_RESULT_OK) {
        return ret;
    }
    /*
     * The minimal real-flush probe is a manual board-test entry. The
     * aggregator validates the 16x16 pattern helper, then records the real
     * panel path as skipped so run_all cannot write the LCD.
     */
    return PET_RESULT_UNSUPPORTED;
}

static pet_result_t pet_selftest_repeated_flush_gate(void)
{
    pet_result_t ret = pet2d_dirty_rect_poc_self_test();

    if (ret != PET_RESULT_OK) {
        return ret;
    }
    return pet2d_boundary_repeated_flush_gate_self_test();
}

static pet_result_t pet_selftest_resource_sprite_surface_gate(void)
{
    pet_result_t ret = pet2d_resource_sprite_poc_self_test();

    if (ret != PET_RESULT_OK) {
        return ret;
    }
    return pet2d_boundary_resource_sprite_gate_self_test();
}

static pet_result_t pet_selftest_resource_package_probe(void)
{
    pet_result_t ret = pet_resource_jieli_real_self_test();

    if ((ret == PET_RESULT_NOT_FOUND) || (ret == PET_RESULT_NOT_READY)) {
        return PET_RESULT_UNSUPPORTED;
    }
    return ret;
}

static pet_result_t pet_selftest_minimal_movement_gate(void)
{
    pet_result_t ret = pet2d_movement_poc_self_test();

    if (ret != PET_RESULT_OK) {
        return ret;
    }
    return PET_RESULT_UNSUPPORTED;
}

static pet_result_t pet_selftest_key_latency_movement_gate(void)
{
    pet2d_movement_poc_stats_t stats;
    pet_result_t ret = pet2d_movement_poc_self_test();

    if (ret != PET_RESULT_OK) {
        return ret;
    }
    ret = pet2d_movement_poc_get_stats(&stats);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    if ((stats.key_event_count == 0u) ||
        (stats.movement_step_count == 0u) ||
        (stats.last_dirty_w < PET2D_MOVEMENT_POC_SURFACE_SIZE) ||
        (stats.last_dirty_h < PET2D_MOVEMENT_POC_SURFACE_SIZE)) {
        return PET_RESULT_ERROR;
    }
    return pet2d_boundary_movement_repeated_probe(PET2D_MOVEMENT_POC_DEFAULT_REPEAT, 0u);
}

static pet_result_t pet_selftest_call(pet_selftest_case_t test_case)
{
    static const pet_selftest_fn_t k_tests[PET_SELFTEST_MAX] = {
        pet_selftest_shared_interface,
        pet_selftest_platform_hal,
        pet_platform_jieli_display_self_test,
        pet_display_jieli_owner_self_test,
        pet_display_jieli_flush_self_test,
        pet_selftest_display_tiny_flush_poc_gate,
        pet_platform_jieli_input_self_test,
        pet2d_boundary_self_test,
        pet_selftest_pet2d_minimal_real_flush_gate,
        pet_selftest_repeated_flush_gate,
        pet_resource_jieli_self_test,
        pet2d_boundary_resource_probe_self_test,
        pet_selftest_resource_sprite_surface_gate,
        pet_selftest_resource_package_probe,
        pet_key_calibration_jieli_self_test,
        pet_selftest_minimal_movement_gate,
        pet_selftest_key_latency_movement_gate,
        pet_save_jieli_self_test,
        pet_protocol_jieli_self_test,
        pet_ble_jieli_self_test,
        pet_nfc_jieli_self_test,
        pet_debug_jieli_self_test
    };

    if ((test_case < PET_SELFTEST_SHARED_INTERFACE) || (test_case >= PET_SELFTEST_MAX)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    return k_tests[test_case]();
}

const char *pet_selftest_case_name(pet_selftest_case_t test_case)
{
    static const char *const k_names[PET_SELFTEST_MAX] = {
        "shared_interface",
        "platform_hal",
        "display_profile",
        "display_owner",
        "display_flush_owner",
        "display_tiny_flush_poc",
        "input_mapping",
        "render_owner_boundary",
        "pet2d_minimal_real_flush_gate",
        "repeated_flush_gate",
        "resource_manifest",
        "pet2d_resource_probe",
        "resource_sprite_surface",
        "resource_package_probe",
        "key_calibration",
        "minimal_movement_poc",
        "key_latency_movement_gate",
        "save_transaction",
        "protocol_packet",
        "ble_loopback",
        "nfc_fake",
        "debug_injection"
    };

    if ((test_case < PET_SELFTEST_SHARED_INTERFACE) || (test_case >= PET_SELFTEST_MAX)) {
        return "unknown";
    }

    return k_names[test_case];
}

pet_result_t pet_selftest_run_case(pet_selftest_case_t test_case)
{
    return pet_selftest_call(test_case);
}

pet_result_t pet_selftest_run_all(pet_selftest_summary_t *out_summary)
{
    pet_selftest_summary_t summary;
    pet_u16_t index;
    pet_result_t ret;

    if (out_summary == 0) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    summary.total = (pet_u16_t)PET_SELFTEST_MAX;
    summary.passed = 0u;
    summary.failed = 0u;
    summary.skipped = 0u;
    summary.failed_mask = 0u;
    summary.skipped_mask = 0u;

    for (index = 0u; index < (pet_u16_t)PET_SELFTEST_MAX; index++) {
        ret = pet_selftest_run_case((pet_selftest_case_t)index);
        if (ret == PET_RESULT_OK) {
            summary.passed++;
        } else if (ret == PET_RESULT_UNSUPPORTED) {
            summary.skipped++;
            summary.skipped_mask |= (1u << index);
        } else {
            summary.failed++;
            summary.failed_mask |= (1u << index);
        }
    }

    *out_summary = summary;
    if (summary.failed != 0u) {
        return PET_RESULT_ERROR;
    }
    if (summary.skipped != 0u) {
        return PET_RESULT_UNSUPPORTED;
    }

    return PET_RESULT_OK;
}

pet_result_t pet_selftest_get_capability_snapshot(pet_platform_capability_snapshot_t *out_snapshot)
{
    if (out_snapshot == 0) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    out_snapshot->has_shared_interface = 1u;
    out_snapshot->has_platform_hal = 1u;
    out_snapshot->has_display_profile = 1u;
    out_snapshot->has_display_owner = 1u;
    out_snapshot->has_display_flush_owner_guard = 1u;
    out_snapshot->has_tiny_lcd_flush_poc_gate = 1u;
    out_snapshot->has_input_mapping = 1u;
    out_snapshot->has_render_owner_boundary = 1u;
    out_snapshot->has_pet2d_minimal_visual_probe_gate = 1u;
    out_snapshot->has_dirty_rect_poc_gate = 1u;
    out_snapshot->has_resource_sprite_surface_probe_gate = 1u;
    out_snapshot->has_real_resource_read_probe = 1u;
    out_snapshot->has_key_calibration = 1u;
    out_snapshot->has_minimal_sprite_movement_probe_gate = 1u;
    out_snapshot->has_movement_stats = 1u;
    out_snapshot->has_key_latency_probe_gate = 1u;
    out_snapshot->has_resource_manifest_adapter = 1u;
    out_snapshot->has_save_transaction_adapter = 1u;
    out_snapshot->has_protocol_helper = 1u;
    out_snapshot->has_ble_loopback_test = 1u;
    out_snapshot->has_nfc_fake_test = 1u;
    out_snapshot->has_debug_injection_test = 1u;

    out_snapshot->real_lcd_flush_enabled = 0u;
    out_snapshot->real_key_queue_enabled = 0u;
    out_snapshot->real_flash_storage_enabled = 0u;
    out_snapshot->real_resource_package_available =
        (pet_resource_jieli_real_package_probe() == PET_RESULT_OK) ? 1u : 0u;
    out_snapshot->external_flash_resource_enabled = 0u;
    out_snapshot->real_ble_enabled = 0u;
    out_snapshot->real_nfc_enabled = 0u;
    out_snapshot->pet2d_runtime_enabled = 0u;

    return PET_RESULT_OK;
}
