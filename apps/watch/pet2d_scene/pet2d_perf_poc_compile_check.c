#include "pet2d_perf_poc.h"

PET_STATIC_ASSERT(pet2d_perf_stats_result_width,
                  sizeof(((pet2d_perf_stats_t *)0)->last_result) == sizeof(pet_u8_t));
PET_STATIC_ASSERT(pet2d_perf_buffer_bound,
                  PET2D_PERF_POC_MAX_SURFACE_SIZE == 128u);
PET_STATIC_ASSERT(pet2d_perf_frame_bound,
                  PET2D_PERF_POC_FRAME_MAX <= 120u);

pet_result_t pet2d_perf_poc_compile_check_self_test(void)
{
    return pet2d_perf_poc_self_test();
}

pet_result_t pet2d_perf_poc_compile_check_stats(void)
{
    pet2d_perf_stats_t stats;

    if (pet2d_perf_poc_get_stats(&stats) != PET_RESULT_OK) {
        return PET_RESULT_ERROR;
    }
    if (pet2d_perf_poc_run_mode(PET2D_PERF_MODE_MAX, 1u, 0u) !=
        PET_RESULT_INVALID_ARGUMENT) {
        return PET_RESULT_ERROR;
    }
    return PET_RESULT_OK;
}
