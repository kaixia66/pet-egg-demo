#include "pet_render_perf.h"

#include <string.h>

pet_result_t pet_render_perf_reset(pet_render_perf_t* perf) {
  if (perf == 0) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  memset(perf, 0, sizeof(*perf));
  return PET_RESULT_OK;
}

pet_result_t pet_render_perf_record_frame(pet_render_perf_t* perf,
                                          uint32_t frame_time_ms,
                                          uint32_t draw_call_count,
                                          uint32_t dirty_rect_count) {
  if (perf == 0) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  perf->frame_count += 1u;
  perf->draw_call_count += draw_call_count;
  perf->dirty_rect_count += dirty_rect_count;
  perf->last_frame_time_ms = frame_time_ms;
  perf->last_draw_time_ms = frame_time_ms;
  if (frame_time_ms > perf->max_frame_time_ms) {
    perf->max_frame_time_ms = frame_time_ms;
  }
  return PET_RESULT_OK;
}
