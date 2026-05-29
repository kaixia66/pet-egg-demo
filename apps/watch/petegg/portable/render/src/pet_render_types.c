#include "pet_render_types.h"

static int32_t pet_render_rect_right(const pet_rect_t* rect) {
  return (int32_t)rect->x + (int32_t)rect->width;
}

static int32_t pet_render_rect_bottom(const pet_rect_t* rect) {
  return (int32_t)rect->y + (int32_t)rect->height;
}

uint16_t pet_render_rgb565(uint8_t r, uint8_t g, uint8_t b) {
  return (uint16_t)((((uint16_t)r & 0xF8u) << 8u) | (((uint16_t)g & 0xFCu) << 3u) |
                    ((uint16_t)b >> 3u));
}

uint8_t pet_render_rect_is_empty(const pet_rect_t* rect) {
  return rect == 0 || rect->width == 0u || rect->height == 0u ? 1u : 0u;
}

pet_result_t pet_render_rect_intersect(const pet_rect_t* a,
                                       const pet_rect_t* b,
                                       pet_rect_t* out_rect) {
  int32_t left;
  int32_t top;
  int32_t right;
  int32_t bottom;
  if (a == 0 || b == 0 || out_rect == 0) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  out_rect->x = 0;
  out_rect->y = 0;
  out_rect->width = 0u;
  out_rect->height = 0u;
  if (pet_render_rect_is_empty(a) != 0u || pet_render_rect_is_empty(b) != 0u) {
    return PET_RESULT_OK;
  }
  left = a->x > b->x ? a->x : b->x;
  top = a->y > b->y ? a->y : b->y;
  right = pet_render_rect_right(a) < pet_render_rect_right(b) ? pet_render_rect_right(a)
                                                              : pet_render_rect_right(b);
  bottom = pet_render_rect_bottom(a) < pet_render_rect_bottom(b) ? pet_render_rect_bottom(a)
                                                                 : pet_render_rect_bottom(b);
  if (right <= left || bottom <= top) {
    return PET_RESULT_OK;
  }
  out_rect->x = (int16_t)left;
  out_rect->y = (int16_t)top;
  out_rect->width = (uint16_t)(right - left);
  out_rect->height = (uint16_t)(bottom - top);
  return PET_RESULT_OK;
}

const char* pet_render_status_name(uint8_t status) {
  switch (status) {
    case PET_RENDER_STATUS_OK:
      return "ok";
    case PET_RENDER_STATUS_INVALID_ARG:
      return "invalid_arg";
    case PET_RENDER_STATUS_EMPTY:
      return "empty";
    case PET_RENDER_STATUS_UNSUPPORTED:
      return "unsupported";
    case PET_RENDER_STATUS_BUFFER_TOO_SMALL:
      return "buffer_too_small";
    default:
      return "unknown";
  }
}
