#include "pet_draw.h"

static uint8_t pet_draw_framebuffer_valid(const pet_framebuffer_t* fb) {
  uint32_t required;
  if (fb == 0 || fb->pixels == 0 || fb->pixel_format != PET_PIXEL_FORMAT_RGB565 ||
      fb->width == 0u || fb->height == 0u || fb->stride_pixels < fb->width) {
    return 0u;
  }
  required = (uint32_t)fb->stride_pixels * (uint32_t)fb->height * 2u;
  return fb->capacity_bytes >= required ? 1u : 0u;
}

pet_result_t pet_draw_fill_rect(pet_framebuffer_t* fb,
                                const pet_rect_t* rect,
                                uint16_t color) {
  return pet_framebuffer_fill_rect(fb, rect, color);
}

pet_result_t pet_draw_blit_rgb565(pet_framebuffer_t* fb,
                                  int16_t dst_x,
                                  int16_t dst_y,
                                  uint16_t src_w,
                                  uint16_t src_h,
                                  uint16_t src_stride_pixels,
                                  const uint16_t* src_pixels,
                                  uint16_t color_key,
                                  uint8_t use_color_key) {
  pet_rect_t dst_rect;
  pet_rect_t clipped;
  uint16_t y;
  uint16_t x;
  if (pet_draw_framebuffer_valid(fb) == 0u || src_pixels == 0 || src_w == 0u ||
      src_h == 0u || src_stride_pixels < src_w) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  dst_rect.x = dst_x;
  dst_rect.y = dst_y;
  dst_rect.width = src_w;
  dst_rect.height = src_h;
  if (pet_render_rect_intersect(&dst_rect, &fb->clip_rect, &clipped) != PET_RESULT_OK) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  if (pet_render_rect_is_empty(&clipped) != 0u) {
    return PET_RESULT_OK;
  }
  for (y = 0u; y < clipped.height; ++y) {
    const int32_t src_y = (int32_t)clipped.y - (int32_t)dst_y + (int32_t)y;
    uint16_t* dst_row =
        fb->pixels + (uint32_t)(clipped.y + (int16_t)y) * fb->stride_pixels + clipped.x;
    const uint16_t* src_row = src_pixels + (uint32_t)src_y * src_stride_pixels;
    for (x = 0u; x < clipped.width; ++x) {
      const int32_t src_x = (int32_t)clipped.x - (int32_t)dst_x + (int32_t)x;
      const uint16_t color = src_row[(uint32_t)src_x];
      if (use_color_key == 0u || color != color_key) {
        dst_row[x] = color;
      }
    }
  }
  return PET_RESULT_OK;
}

pet_result_t pet_draw_execute(pet_framebuffer_t* fb, const pet_draw_command_t* cmd) {
  if (fb == 0 || cmd == 0) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  switch (cmd->type) {
    case PET_DRAW_CMD_FILL_RECT:
      return pet_draw_fill_rect(fb, &cmd->dst_rect, cmd->color);
    case PET_DRAW_CMD_CLEAR:
      return pet_framebuffer_clear(fb, cmd->color);
    case PET_DRAW_CMD_SET_CLIP:
      return pet_framebuffer_set_clip(fb, &cmd->dst_rect);
    case PET_DRAW_CMD_RESET_CLIP:
      return pet_framebuffer_reset_clip(fb);
    case PET_DRAW_CMD_SPRITE_BLIT:
      return PET_RESULT_UNSUPPORTED;
    default:
      return PET_RESULT_UNSUPPORTED;
  }
}
