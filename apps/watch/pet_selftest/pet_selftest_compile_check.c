#include "pet_selftest.h"

#include "pet_platform.h"

PET_STATIC_ASSERT(pet_selftest_case_count_fits_mask, PET_SELFTEST_MAX <= 32);

pet_result_t pet_selftest_compile_check_run_all(void)
{
    pet_selftest_summary_t summary;

    return pet_selftest_run_all(&summary);
}

pet_result_t pet_selftest_compile_check_snapshot(void)
{
    pet_platform_capability_snapshot_t snapshot;

    if (pet_selftest_get_capability_snapshot(&snapshot) != PET_RESULT_OK) {
        return PET_RESULT_ERROR;
    }
    if ((snapshot.has_pet2d_minimal_visual_probe_gate == 0u) ||
        (snapshot.has_dirty_rect_poc_gate == 0u) ||
        (snapshot.has_resource_sprite_surface_probe_gate == 0u) ||
        (snapshot.has_real_resource_read_probe == 0u) ||
        (snapshot.has_key_calibration == 0u) ||
        (snapshot.has_minimal_sprite_movement_probe_gate == 0u) ||
        (snapshot.has_movement_stats == 0u) ||
        (snapshot.has_key_latency_probe_gate == 0u) ||
        (snapshot.has_pet2d_scene_handoff == 0u) ||
        (snapshot.has_pet2d_perf_poc == 0u) ||
        (snapshot.has_mvp_a_scene_skeleton == 0u) ||
        (snapshot.mvp_a_scene_skeleton_debug_entry == 0u) ||
        (snapshot.mvp_a_scene_skeleton_real_board_verified != 0u) ||
        (snapshot.has_mvp_a_scene_action_loop == 0u) ||
        (snapshot.mvp_a_scene_action_loop_selftest == 0u) ||
        (snapshot.mvp_a_scene_action_loop_debug_entry == 0u) ||
        (snapshot.home_observe_enabled != 0u) ||
        (snapshot.full_pet2d_runtime_enabled != 0u) ||
        (snapshot.has_internal_save_syscfg_backend == 0u) ||
        (snapshot.internal_save_ab_supported == 0u) ||
        (snapshot.internal_save_crc_supported == 0u) ||
        (snapshot.internal_save_rollback_supported == 0u) ||
        (snapshot.internal_save_real_write_verified != 0u) ||
        (snapshot.internal_save_low_battery_guard_supported != 0u) ||
        (snapshot.internal_save_low_battery_guard_planned == 0u) ||
        (snapshot.real_lcd_flush_enabled != 0u) ||
        (snapshot.external_flash_resource_enabled != 0u) ||
        (snapshot.pet2d_runtime_enabled != 0u)) {
        return PET_RESULT_ERROR;
    }
    return PET_RESULT_OK;
}

const char *pet_selftest_compile_check_name(void)
{
    return pet_selftest_case_name(PET_SELFTEST_MVP_A_SCENE_ACTION_LOOP);
}
