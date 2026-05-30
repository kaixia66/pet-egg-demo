#ifndef PET_DISPLAY_PROFILE_H
#define PET_DISPLAY_PROFILE_H

#include "pet_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PET_RGB565_BYTES_PER_PIXEL 2u
#define PET_RGB565_RED_MASK        0xF800u
#define PET_RGB565_GREEN_MASK      0x07E0u
#define PET_RGB565_BLUE_MASK       0x001Fu

typedef enum {
    PET_DISPLAY_OWNER_NONE = 0,
    PET_DISPLAY_OWNER_NATIVE_WATCH_UI,
    PET_DISPLAY_OWNER_LVGL_SYSTEM_UI,
    PET_DISPLAY_OWNER_PET2D,
    PET_DISPLAY_OWNER_DEBUG
} pet_display_owner_t;

typedef enum {
    PET_DISPLAY_FLUSH_MODE_RGB565_FULL = 0,
    PET_DISPLAY_FLUSH_MODE_RGB565_RECT,
    PET_DISPLAY_FLUSH_MODE_RGB565_RECT_ASYNC
} pet_display_flush_mode_t;

typedef enum {
    PET_DISPLAY_ROTATION_0 = 0,
    PET_DISPLAY_ROTATION_90 = 90,
    PET_DISPLAY_ROTATION_180 = 180,
    PET_DISPLAY_ROTATION_270 = 270
} pet_display_rotation_t;

typedef enum {
    PET_RGB565_ORDER_RGB = 0,
    PET_RGB565_ORDER_BGR
} pet_rgb565_order_t;

typedef PetScreenShape pet_screen_shape_t;

typedef struct PetScreenProfile {
    int32_t width;
    int32_t height;
    PetScreenShape shape;
    int32_t safe_margin_percent;
    int32_t default_scale;
} PetScreenProfile;

typedef struct {
    pet_u16_t left;
    pet_u16_t top;
    pet_u16_t right;
    pet_u16_t bottom;
} pet_safe_area_t;

typedef struct {
    pet_u16_t width;
    pet_u16_t height;
    PetScreenShape shape;
    int32_t safe_margin_percent;
    int32_t default_scale;
    pet_display_flush_mode_t flush_mode;
    pet_display_rotation_t rotation;
    pet_rgb565_order_t rgb565_order;
    pet_safe_area_t safe_area;
    pet_u16_t stride_align_pixels;
    pet_u16_t min_flush_width;
    pet_u16_t min_flush_height;
    pet_u8_t requires_even_x;
    pet_u8_t requires_even_width;
    pet_u8_t reserved[2];
} pet_display_profile_t;

#ifdef __cplusplus
}
#endif

#endif
