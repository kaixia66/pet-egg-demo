#include "pet_framebuffer.h"

#include <string.h>

static pet_rect_t pet_framebuffer_full_rect(const pet_framebuffer_t* fb) {
  pet_rect_t rect;
  rect.x = 0;
  rect.y = 0;
  rect.width = fb->width;
  rect.height = fb->height;
  return rect;
}

static uint8_t pet_framebuffer_valid(const pet_framebuffer_t* fb) {
  return fb != 0 && fb->pixel_format == PET_PIXEL_FORMAT_RGB565 && fb->pixels != 0 &&
         fb->width != 0u && fb->height != 0u && fb->stride_pixels >= fb->width;
}

pet_result_t pet_framebuffer_init_rgb565(pet_framebuffer_t* fb,
                                         uint16_t width,
                                         uint16_t height,
                                         uint16_t stride_pixels,
                                         uint16_t* pixels,
                                         uint32_t capacity_bytes) {
  uint32_t required;
  if (fb == 0 || pixels == 0 || width == 0u || height == 0u || stride_pixels < width) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  required = (uint32_t)stride_pixels * (uint32_t)height * 2u;
  if (capacity_bytes < required) {
    return PET_RESULT_BUFFER_TOO_SMALL;
  }
  memset(fb, 0, sizeof(*fb));
  fb->width = width;
  fb->height = height;
  fb->pixel_format = PET_PIXEL_FORMAT_RGB565;
  fb->stride_pixels = stride_pixels;
  fb->pixels = pixels;
  fb->capacity_bytes = capacity_bytes;
  return pet_framebuffer_reset_clip(fb);
}

pet_result_t pet_framebuffer_reset_clip(pet_framebuffer_t* fb) {
  if (pet_framebuffer_valid(fb) == 0u) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  fb->clip_rect = pet_framebuffer_full_rect(fb);
  return PET_RESULT_OK;
}

pet_result_t pet_framebuffer_set_clip(pet_framebuffer_t* fb, const pet_rect_t* clip) {
  pet_rect_t full_rect;
  pet_rect_t clipped;
  if (pet_framebuffer_valid(fb) == 0u || clip == 0) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  full_rect = pet_framebuffer_full_rect(fb);
  if (pet_render_rect_intersect(&full_rect, clip, &clipped) != PET_RESULT_OK) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  fb->clip_rect = clipped;
  return PET_RESULT_OK;
}

pet_result_t pet_framebuffer_clear(pet_framebuffer_t* fb, uint16_t color) {
  uint16_t y;
  uint16_t x;
  if (pet_framebuffer_valid(fb) == 0u) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  for (y = 0u; y < fb->height; ++y) {
    uint16_t* row = fb->pixels + (uint32_t)y * fb->stride_pixels;
    for (x = 0u; x < fb->width; ++x) {
      row[x] = color;
    }
  }
  return PET_RESULT_OK;
}

pet_result_t pet_framebuffer_fill_rect(pet_framebuffer_t* fb,
                                       const pet_rect_t* rect,
                                       uint16_t color) {
  pet_rect_t clipped;
  uint16_t y;
  uint16_t x;
  if (pet_framebuffer_valid(fb) == 0u || rect == 0) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  if (pet_render_rect_intersect(rect, &fb->clip_rect, &clipped) != PET_RESULT_OK) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  if (pet_render_rect_is_empty(&clipped) != 0u) {
    return PET_RESULT_OK;
  }
  for (y = 0u; y < clipped.height; ++y) {
    uint16_t* row =
        fb->pixels + (uint32_t)(clipped.y + (int16_t)y) * fb->stride_pixels + clipped.x;
    for (x = 0u; x < clipped.width; ++x) {
      row[x] = color;
    }
  }
  return PET_RESULT_OK;
}
