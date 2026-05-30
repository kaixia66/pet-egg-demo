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
        (snapshot.real_lcd_flush_enabled != 0u) ||
        (snapshot.pet2d_runtime_enabled != 0u)) {
        return PET_RESULT_ERROR;
    }
    return PET_RESULT_OK;
}

const char *pet_selftest_compile_check_name(void)
{
    return pet_selftest_case_name(PET_SELFTEST_REPEATED_FLUSH_GATE);
}
