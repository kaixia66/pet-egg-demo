#include "pet2d_minimal_visual.h"

#define PET2D_RGB565_RED     0xF800u
#define PET2D_RGB565_GREEN   0x07E0u
#define PET2D_RGB565_BLUE    0x001Fu
#define PET2D_RGB565_WHITE   0xFFFFu
#define PET2D_RGB565_YELLOW  0xFFE0u
#define PET2D_RGB565_MAGENTA 0xF81Fu
#define PET2D_RGB565_BLACK   0x0000u

static pet_u16_t pet2d_minimal_visual_expected_pixel(pet_u16_t x, pet_u16_t y,
                                                     pet_u16_t width, pet_u16_t height)
{
    if ((x == 0u) || (y == 0u) || (x == (pet_u16_t)(width - 1u)) ||
        (y == (pet_u16_t)(height - 1u))) {
        return PET2D_RGB565_WHITE;
    }
    if (x == y) {
        return PET2D_RGB565_YELLOW;
    }
    if ((pet_u16_t)(x + y) == (pet_u16_t)(width - 1u)) {
        return PET2D_RGB565_MAGENTA;
    }
    if ((x < (width / 2u)) && (y < (height / 2u))) {
        return PET2D_RGB565_RED;
    }
    if ((x >= (width / 2u)) && (y < (height / 2u))) {
        return PET2D_RGB565_GREEN;
    }
    if ((x < (width / 2u)) && (y >= (height / 2u))) {
        return PET2D_RGB565_BLUE;
    }
    return PET2D_RGB565_BLACK;
}

pet_result_t pet2d_minimal_visual_fill_test_pattern(pet2d_minimal_surface_t *surface)
{
    pet_u16_t x;
    pet_u16_t y;

    if ((surface == 0) || (surface->pixels == 0)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }
    if ((surface->width == 0u) || (surface->height == 0u) ||
        (surface->pitch_pixels < surface->width)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    for (y = 0u; y < surface->height; y++) {
        for (x = 0u; x < surface->width; x++) {
            surface->pixels[((pet_u32_t)y * surface->pitch_pixels) + x] =
                pet2d_minimal_visual_expected_pixel(x, y, surface->width, surface->height);
        }
    }

    return PET_RESULT_OK;
}

pet_result_t pet2d_minimal_visual_self_test(void)
{
    pet_u16_t pixels[PET2D_MINIMAL_VISUAL_WIDTH * PET2D_MINIMAL_VISUAL_HEIGHT];
    pet2d_minimal_surface_t surface;
    pet_result_t ret;

    if (pet2d_minimal_visual_fill_test_pattern(0) != PET_RESULT_INVALID_ARGUMENT) {
        return PET_RESULT_ERROR;
    }

    surface.width = PET2D_MINIMAL_VISUAL_WIDTH;
    surface.height = PET2D_MINIMAL_VISUAL_HEIGHT;
    surface.pitch_pixels = PET2D_MINIMAL_VISUAL_WIDTH;
    surface.pixels = pixels;

    ret = pet2d_minimal_visual_fill_test_pattern(&surface);
    if (ret != PET_RESULT_OK) {
        return ret;
    }

    if ((pixels[0u] != PET2D_RGB565_WHITE) ||
        (pixels[1u] != PET2D_RGB565_WHITE) ||
        (pixels[(7u * PET2D_MINIMAL_VISUAL_WIDTH) + 7u] != PET2D_RGB565_YELLOW) ||
        (pixels[(7u * PET2D_MINIMAL_VISUAL_WIDTH) + 8u] != PET2D_RGB565_MAGENTA) ||
        (pixels[(4u * PET2D_MINIMAL_VISUAL_WIDTH) + 5u] != PET2D_RGB565_RED) ||
        (pixels[(4u * PET2D_MINIMAL_VISUAL_WIDTH) + 10u] != PET2D_RGB565_GREEN) ||
        (pixels[(10u * PET2D_MINIMAL_VISUAL_WIDTH) + 4u] != PET2D_RGB565_BLUE) ||
        (pixels[(10u * PET2D_MINIMAL_VISUAL_WIDTH) + 11u] != PET2D_RGB565_BLACK)) {
        return PET_RESULT_ERROR;
    }

    surface.pitch_pixels = (pet_u16_t)(PET2D_MINIMAL_VISUAL_WIDTH - 1u);
    if (pet2d_minimal_visual_fill_test_pattern(&surface) != PET_RESULT_INVALID_ARGUMENT) {
        return PET_RESULT_ERROR;
    }

    return PET_RESULT_OK;
}
