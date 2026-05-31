#ifndef PET2D_PERF_POC_H
#define PET2D_PERF_POC_H

#include "pet_platform.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PET2D_PERF_POC_FRAME_MAX 120u
#define PET2D_PERF_POC_DEFAULT_FRAMES 60u
#define PET2D_PERF_POC_MAX_SURFACE_SIZE 128u
#define PET2D_PERF_POC_STEP_PIXELS 8u
#define PET2D_PERF_POC_RUN_TIMEOUT_MS 6000u

typedef enum {
    PET2D_PERF_MODE_RECT_32 = 0,
    PET2D_PERF_MODE_RECT_64,
    PET2D_PERF_MODE_RECT_96,
    PET2D_PERF_MODE_RECT_128,
    PET2D_PERF_MODE_MAX
} pet2d_perf_mode_t;

typedef struct {
    pet_u32_t run_count;
    pet_u32_t frame_attempt_count;
    pet_u32_t frame_success_count;
    pet_u32_t frame_fail_count;

    pet_u32_t logic_total_ms;
    pet_u32_t render_total_ms;
    pet_u32_t flush_total_ms;
    pet_u32_t frame_total_ms;

    pet_u32_t logic_max_ms;
    pet_u32_t render_max_ms;
    pet_u32_t flush_max_ms;
    pet_u32_t frame_max_ms;

    pet_u32_t logic_min_ms;
    pet_u32_t render_min_ms;
    pet_u32_t flush_min_ms;
    pet_u32_t frame_min_ms;

    pet_u32_t logic_avg_ms;
    pet_u32_t render_avg_ms;
    pet_u32_t flush_avg_ms;
    pet_u32_t frame_avg_ms;
    pet_u32_t approx_fps_x100;

    pet_u16_t last_rect_w;
    pet_u16_t last_rect_h;
    pet_u16_t last_dirty_w;
    pet_u16_t last_dirty_h;
    pet_u16_t last_frame_count;
    pet_u8_t last_mode;
    pet_u8_t last_result;
} pet2d_perf_stats_t;

pet_result_t pet2d_perf_poc_run_mode(pet2d_perf_mode_t mode,
                                     pet_u16_t frame_count,
                                     pet_u16_t frame_delay_ms);
pet_result_t pet2d_perf_poc_get_stats(pet2d_perf_stats_t *out_stats);
pet_result_t pet2d_perf_poc_reset_stats(void);
pet_result_t pet2d_perf_poc_self_test(void);

#ifdef __cplusplus
}
#endif

#endif
