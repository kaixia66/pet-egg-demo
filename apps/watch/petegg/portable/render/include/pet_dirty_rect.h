#ifndef PETEGG_PORTABLE_PET_DIRTY_RECT_H_
#define PETEGG_PORTABLE_PET_DIRTY_RECT_H_

#include "pet_render_types.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PET_DIRTY_RECT_MAX_RECTS 4u

typedef struct pet_dirty_rect_accumulator_t {
  pet_rect_t rects[PET_DIRTY_RECT_MAX_RECTS];
  uint8_t count;
  pet_rect_t bounding_rect;
  uint16_t flags;
} pet_dirty_rect_accumulator_t;

typedef pet_dirty_rect_accumulator_t PetDirtyRectAccumulator;

pet_result_t pet_dirty_rect_init(pet_dirty_rect_accumulator_t* dirty);
pet_result_t pet_dirty_rect_add(pet_dirty_rect_accumulator_t* dirty, const pet_rect_t* rect);
pet_result_t pet_dirty_rect_get_bounds(const pet_dirty_rect_accumulator_t* dirty,
                                       pet_rect_t* out_bounds);
pet_result_t pet_dirty_rect_clear(pet_dirty_rect_accumulator_t* dirty);

#ifdef __cplusplus
}
#endif

#endif  /* PETEGG_PORTABLE_PET_DIRTY_RECT_H_ */
