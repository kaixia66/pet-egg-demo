#ifndef PETEGG_PORTABLE_PET_SPRITE_H_
#define PETEGG_PORTABLE_PET_SPRITE_H_

#include "pet_config.h"
#include "pet_render_types.h"
#include "pet_result.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PET_SPRITE_FLAG_NONE 0u
#define PET_SPRITE_FLAG_HAS_COLOR_KEY 1u
#define PET_SPRITE_FLAG_RGB565 2u

typedef struct pet_sprite_frame_t {
  uint16_t resource_id;
  uint16_t x;
  uint16_t y;
  uint16_t width;
  uint16_t height;
  int16_t anchor_x;
  int16_t anchor_y;
  uint16_t color_key;
  uint16_t flags;
  uint32_t reserved;
} pet_sprite_frame_t;

typedef struct pet_sprite_sheet_ref_t {
  uint16_t resource_id;
  uint16_t frame_count;
  uint16_t first_frame_id;
  uint16_t flags;
} pet_sprite_sheet_ref_t;

typedef pet_sprite_frame_t PetSpriteFrame;
typedef pet_sprite_sheet_ref_t PetSpriteSheetRef;

pet_result_t pet_sprite_frame_validate(const pet_sprite_frame_t* frame);

#ifdef __cplusplus
}
#endif

#endif  /* PETEGG_PORTABLE_PET_SPRITE_H_ */
