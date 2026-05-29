#include "pet_dirty_rect.h"

#include <string.h>

static pet_rect_t pet_dirty_rect_union(const pet_rect_t* a, const pet_rect_t* b) {
  pet_rect_t out;
  int32_t left;
  int32_t top;
  int32_t right;
  int32_t bottom;
  if (pet_render_rect_is_empty(a) != 0u) {
    return *b;
  }
  if (pet_render_rect_is_empty(b) != 0u) {
    return *a;
  }
  left = a->x < b->x ? a->x : b->x;
  top = a->y < b->y ? a->y : b->y;
  right = (int32_t)a->x + a->width > (int32_t)b->x + b->width
              ? (int32_t)a->x + a->width
              : (int32_t)b->x + b->width;
  bottom = (int32_t)a->y + a->height > (int32_t)b->y + b->height
               ? (int32_t)a->y + a->height
               : (int32_t)b->y + b->height;
  out.x = (int16_t)left;
  out.y = (int16_t)top;
  out.width = (uint16_t)(right - left);
  out.height = (uint16_t)(bottom - top);
  return out;
}

pet_result_t pet_dirty_rect_init(pet_dirty_rect_accumulator_t* dirty) {
  if (dirty == 0) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  memset(dirty, 0, sizeof(*dirty));
  return PET_RESULT_OK;
}

pet_result_t pet_dirty_rect_clear(pet_dirty_rect_accumulator_t* dirty) {
  return pet_dirty_rect_init(dirty);
}

pet_result_t pet_dirty_rect_add(pet_dirty_rect_accumulator_t* dirty, const pet_rect_t* rect) {
  if (dirty == 0 || rect == 0) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  if (pet_render_rect_is_empty(rect) != 0u) {
    return PET_RESULT_OK;
  }
  if (dirty->count < PET_DIRTY_RECT_MAX_RECTS) {
    dirty->rects[dirty->count] = *rect;
    dirty->count += 1u;
  }
  dirty->bounding_rect = dirty->count == 1u ? *rect : pet_dirty_rect_union(&dirty->bounding_rect, rect);
  return PET_RESULT_OK;
}

pet_result_t pet_dirty_rect_get_bounds(const pet_dirty_rect_accumulator_t* dirty,
                                       pet_rect_t* out_bounds) {
  if (dirty == 0 || out_bounds == 0) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  *out_bounds = dirty->bounding_rect;
  return PET_RESULT_OK;
}
