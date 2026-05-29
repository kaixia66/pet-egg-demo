#ifndef PETEGG_PORTABLE_PET_RENDER_PERF_H_
#define PETEGG_PORTABLE_PET_RENDER_PERF_H_

#include "pet_result.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct pet_render_perf_t {
  uint32_t frame_count;
  uint32_t draw_call_count;
  uint32_t dirty_rect_count;
  uint32_t last_frame_time_ms;
  uint32_t last_draw_time_ms;
  uint32_t last_flush_time_ms;
  uint32_t max_frame_time_ms;
  uint16_t flags;
} pet_render_perf_t;

typedef pet_render_perf_t PetRenderPerf;

pet_result_t pet_render_perf_reset(pet_render_perf_t* perf);
pet_result_t pet_render_perf_record_frame(pet_render_perf_t* perf,
                                          uint32_t frame_time_ms,
                                          uint32_t draw_call_count,
                                          uint32_t dirty_rect_count);

#ifdef __cplusplus
}
#endif

#endif  /* PETEGG_PORTABLE_PET_RENDER_PERF_H_ */
