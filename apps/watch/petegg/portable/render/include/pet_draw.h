#ifndef PETEGG_PORTABLE_PET_DRAW_H_
#define PETEGG_PORTABLE_PET_DRAW_H_

#include "pet_framebuffer.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PET_DRAW_CMD_FILL_RECT 1u
#define PET_DRAW_CMD_SPRITE_BLIT 2u
#define PET_DRAW_CMD_CLEAR 3u
#define PET_DRAW_CMD_SET_CLIP 4u
#define PET_DRAW_CMD_RESET_CLIP 5u

typedef struct pet_draw_command_t {
  uint8_t type;
  pet_rect_t dst_rect;
  pet_rect_t src_rect;
  uint16_t resource_id;
  uint16_t color;
  uint16_t color_key;
  uint16_t flags;
} pet_draw_command_t;

typedef pet_draw_command_t PetDrawCommand;

pet_result_t pet_draw_fill_rect(pet_framebuffer_t* fb,
                                const pet_rect_t* rect,
                                uint16_t color);
pet_result_t pet_draw_blit_rgb565(pet_framebuffer_t* fb,
                                  int16_t dst_x,
                                  int16_t dst_y,
                                  uint16_t src_w,
                                  uint16_t src_h,
                                  uint16_t src_stride_pixels,
                                  const uint16_t* src_pixels,
                                  uint16_t color_key,
                                  uint8_t use_color_key);
pet_result_t pet_draw_execute(pet_framebuffer_t* fb, const pet_draw_command_t* cmd);

#ifdef __cplusplus
}
#endif

#endif  /* PETEGG_PORTABLE_PET_DRAW_H_ */
