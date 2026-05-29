#include "pet_sprite.h"

pet_result_t pet_sprite_frame_validate(const pet_sprite_frame_t* frame) {
  if (frame == 0 || frame->resource_id == 0u || frame->width == 0u || frame->height == 0u) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  return PET_RESULT_OK;
}
