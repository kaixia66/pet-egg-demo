#ifndef PETEGG_PORTABLE_PET_FRAMEBUFFER_H_
#define PETEGG_PORTABLE_PET_FRAMEBUFFER_H_

#include "pet_render_types.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PET_FRAMEBUFFER_FLAG_NONE 0u

typedef struct pet_framebuffer_t {
  uint16_t width;
  uint16_t height;
  uint8_t pixel_format;
  uint16_t stride_pixels;
  uint16_t* pixels;
  uint32_t capacity_bytes;
  pet_rect_t clip_rect;
  uint16_t flags;
} pet_framebuffer_t;

typedef pet_framebuffer_t PetFramebuffer;

pet_result_t pet_framebuffer_init_rgb565(pet_framebuffer_t* fb,
                                         uint16_t width,
                                         uint16_t height,
                                         uint16_t stride_pixels,
                                         uint16_t* pixels,
                                         uint32_t capacity_bytes);
pet_result_t pet_framebuffer_clear(pet_framebuffer_t* fb, uint16_t color);
pet_result_t pet_framebuffer_fill_rect(pet_framebuffer_t* fb,
                                       const pet_rect_t* rect,
                                       uint16_t color);
pet_result_t pet_framebuffer_set_clip(pet_framebuffer_t* fb, const pet_rect_t* clip);
pet_result_t pet_framebuffer_reset_clip(pet_framebuffer_t* fb);

#ifdef __cplusplus
}
#endif

#endif  /* PETEGG_PORTABLE_PET_FRAMEBUFFER_H_ */
