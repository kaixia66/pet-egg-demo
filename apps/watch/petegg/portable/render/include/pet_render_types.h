#ifndef PETEGG_PORTABLE_PET_RENDER_TYPES_H_
#define PETEGG_PORTABLE_PET_RENDER_TYPES_H_

#include "pet_result.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Portable Render Core basic types. Coordinates start at the top-left corner in pixels.
   Rectangles use x/y/width/height and are clipped to half-open bounds. */
#define PET_PIXEL_FORMAT_NONE 0u
#define PET_PIXEL_FORMAT_RGB565 1u

#define PET_CLIP_MODE_RECT 1u
#define PET_CLIP_MODE_SCREEN_PROFILE_RESERVED 2u

#define PET_BLEND_MODE_OPAQUE 0u
#define PET_BLEND_MODE_COLOR_KEY 1u

#define PET_RENDER_STATUS_OK 0u
#define PET_RENDER_STATUS_INVALID_ARG 1u
#define PET_RENDER_STATUS_EMPTY 2u
#define PET_RENDER_STATUS_UNSUPPORTED 3u
#define PET_RENDER_STATUS_BUFFER_TOO_SMALL 4u

typedef uint16_t pet_color_rgb565_t;

typedef struct pet_point_t {
  int16_t x;
  int16_t y;
} pet_point_t;

typedef struct pet_size_t {
  uint16_t width;
  uint16_t height;
} pet_size_t;

typedef struct pet_rect_t {
  int16_t x;
  int16_t y;
  uint16_t width;
  uint16_t height;
} pet_rect_t;

typedef pet_color_rgb565_t PetColorRgb565;
typedef pet_point_t PetPoint;
typedef pet_size_t PetSize;
typedef pet_rect_t PetRect;

uint16_t pet_render_rgb565(uint8_t r, uint8_t g, uint8_t b);
uint8_t pet_render_rect_is_empty(const pet_rect_t* rect);
pet_result_t pet_render_rect_intersect(const pet_rect_t* a,
                                       const pet_rect_t* b,
                                       pet_rect_t* out_rect);
const char* pet_render_status_name(uint8_t status);

#ifdef __cplusplus
}
#endif

#endif  /* PETEGG_PORTABLE_PET_RENDER_TYPES_H_ */
