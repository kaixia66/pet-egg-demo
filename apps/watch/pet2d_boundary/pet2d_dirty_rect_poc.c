#include "pet2d_dirty_rect_poc.h"

#include "pet_platform_jieli_internal.h"

#define PET2D_COLOR_BLACK 0x0000u
#define PET2D_COLOR_WHITE 0xFFFFu
#define PET2D_COLOR_RED   0xF800u
#define PET2D_COLOR_GREEN 0x07E0u
#define PET2D_COLOR_BLUE  0x001Fu
#define PET2D_COLOR_YELLOW 0xFFE0u
#define PET2D_COLOR_MAGENTA 0xF81Fu
#define PET2D_COLOR_CYAN 0x07FFu

pet_u16_t pet2d_dirty_rect_poc_pattern_size(pet2d_dirty_rect_pattern_t pattern)
{
    if (pattern == PET2D_DIRTY_RECT_PATTERN_16) {
        return PET2D_DIRTY_RECT_SIZE_16;
    }
    if (pattern == PET2D_DIRTY_RECT_PATTERN_32) {
        return PET2D_DIRTY_RECT_SIZE_32;
    }
    if (pattern == PET2D_DIRTY_RECT_PATTERN_64) {
        return PET2D_DIRTY_RECT_SIZE_64;
    }
    return 0u;
}

static pet_u16_t pet2d_dirty_rect_pattern_pixel(pet2d_dirty_rect_pattern_t pattern,
                                                pet_u16_t x, pet_u16_t y,
                                                pet_u16_t size)
{
    if ((x == 0u) || (y == 0u) || (x == (pet_u16_t)(size - 1u)) ||
        (y == (pet_u16_t)(size - 1u))) {
        return PET2D_COLOR_WHITE;
    }

    if (pattern == PET2D_DIRTY_RECT_PATTERN_16) {
        if (x == y) {
            return PET2D_COLOR_YELLOW;
        }
        if (x == (pet_u16_t)((size - 1u) - y)) {
            return PET2D_COLOR_MAGENTA;
        }
        if ((x < (size / 2u)) && (y < (size / 2u))) {
            return PET2D_COLOR_RED;
        }
        if ((x >= (size / 2u)) && (y < (size / 2u))) {
            return PET2D_COLOR_GREEN;
        }
        if ((x < (size / 2u)) && (y >= (size / 2u))) {
            return PET2D_COLOR_BLUE;
        }
        return PET2D_COLOR_BLACK;
    }

    if (pattern == PET2D_DIRTY_RECT_PATTERN_32) {
        pet_u16_t tile = (pet_u16_t)(((x / 4u) + (y / 4u)) & 3u);
        if (tile == 0u) {
            return PET2D_COLOR_RED;
        }
        if (tile == 1u) {
            return PET2D_COLOR_GREEN;
        }
        if (tile == 2u) {
            return PET2D_COLOR_BLUE;
        }
        return PET2D_COLOR_CYAN;
    }

    if ((x == (size / 2u)) || (y == (size / 2u))) {
        return PET2D_COLOR_YELLOW;
    }
    if ((y & 7u) < 4u) {
        return PET2D_COLOR_RED;
    }
    if ((x & 7u) < 4u) {
        return PET2D_COLOR_BLUE;
    }
    return PET2D_COLOR_GREEN;
}

pet_result_t pet2d_dirty_rect_poc_fill_pattern(pet2d_dirty_rect_pattern_t pattern,
                                               pet2d_minimal_surface_t *surface)
{
    pet_u16_t size = pet2d_dirty_rect_poc_pattern_size(pattern);
    pet_u16_t x;
    pet_u16_t y;

    if ((surface == 0) || (surface->pixels == 0) || (size == 0u)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }
    if ((surface->width != size) || (surface->height != size) ||
        (surface->pitch_pixels < size)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    for (y = 0u; y < size; y++) {
        for (x = 0u; x < size; x++) {
            surface->pixels[((pet_u32_t)y * surface->pitch_pixels) + x] =
                pet2d_dirty_rect_pattern_pixel(pattern, x, y, size);
        }
    }

    return PET_RESULT_OK;
}

pet_result_t pet2d_dirty_rect_poc_rect_for_case(pet2d_dirty_rect_pattern_t pattern,
                                                pet_u8_t case_index,
                                                pet2d_dirty_rect_case_t *out_rect)
{
    pet_u16_t size = pet2d_dirty_rect_poc_pattern_size(pattern);

    if ((out_rect == 0) || (size == 0u)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    out_rect->width = size;
    out_rect->height = size;
    if (case_index == 0u) {
        out_rect->x = (pet_i16_t)((PET_JIELI_DISPLAY_WIDTH - size) / 2u);
        out_rect->y = (pet_i16_t)((PET_JIELI_DISPLAY_HEIGHT - size) / 2u);
        return PET_RESULT_OK;
    }
    if (case_index == 1u) {
        out_rect->x = 101;
        out_rect->y = 103;
        return PET_RESULT_OK;
    }
    if (case_index == 2u) {
        out_rect->x = (pet_i16_t)(PET_JIELI_DISPLAY_WIDTH - size - 1u);
        out_rect->y = (pet_i16_t)(PET_JIELI_DISPLAY_HEIGHT - size - 1u);
        return PET_RESULT_OK;
    }
    if (case_index == 3u) {
        out_rect->x = (pet_i16_t)(PET_JIELI_DISPLAY_WIDTH - (size / 2u));
        out_rect->y = (pet_i16_t)(PET_JIELI_DISPLAY_HEIGHT - (size / 2u));
        return PET_RESULT_OK;
    }

    return PET_RESULT_NOT_FOUND;
}

pet_result_t pet2d_dirty_rect_poc_rect_in_bounds(const pet2d_dirty_rect_case_t *rect)
{
    pet_i32_t right;
    pet_i32_t bottom;

    if ((rect == 0) || (rect->width == 0u) || (rect->height == 0u) ||
        (rect->x < 0) || (rect->y < 0)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    right = (pet_i32_t)rect->x + (pet_i32_t)rect->width;
    bottom = (pet_i32_t)rect->y + (pet_i32_t)rect->height;
    if ((right > (pet_i32_t)PET_JIELI_DISPLAY_WIDTH) ||
        (bottom > (pet_i32_t)PET_JIELI_DISPLAY_HEIGHT)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    return PET_RESULT_OK;
}

pet_result_t pet2d_dirty_rect_poc_self_test(void)
{
    static pet_u16_t buffer[PET2D_DIRTY_RECT_SIZE_64 * PET2D_DIRTY_RECT_SIZE_64];
    pet2d_minimal_surface_t surface;
    pet2d_dirty_rect_case_t rect;
    pet_result_t ret;
    pet_u16_t size;

    if (pet2d_dirty_rect_poc_fill_pattern(PET2D_DIRTY_RECT_PATTERN_MAX, 0) !=
        PET_RESULT_INVALID_ARGUMENT) {
        return PET_RESULT_ERROR;
    }

    size = pet2d_dirty_rect_poc_pattern_size(PET2D_DIRTY_RECT_PATTERN_16);
    surface.width = size;
    surface.height = size;
    surface.pitch_pixels = size;
    surface.pixels = buffer;
    ret = pet2d_dirty_rect_poc_fill_pattern(PET2D_DIRTY_RECT_PATTERN_16, &surface);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    if ((buffer[0] != PET2D_COLOR_WHITE) ||
        (buffer[(1u * size) + 1u] != PET2D_COLOR_YELLOW)) {
        return PET_RESULT_ERROR;
    }

    size = pet2d_dirty_rect_poc_pattern_size(PET2D_DIRTY_RECT_PATTERN_32);
    surface.width = size;
    surface.height = size;
    surface.pitch_pixels = size;
    ret = pet2d_dirty_rect_poc_fill_pattern(PET2D_DIRTY_RECT_PATTERN_32, &surface);
    if (ret != PET_RESULT_OK) {
        return ret;
    }

    size = pet2d_dirty_rect_poc_pattern_size(PET2D_DIRTY_RECT_PATTERN_64);
    surface.width = size;
    surface.height = size;
    surface.pitch_pixels = size;
    ret = pet2d_dirty_rect_poc_fill_pattern(PET2D_DIRTY_RECT_PATTERN_64, &surface);
    if (ret != PET_RESULT_OK) {
        return ret;
    }

    ret = pet2d_dirty_rect_poc_rect_for_case(PET2D_DIRTY_RECT_PATTERN_64, 0u, &rect);
    if ((ret != PET_RESULT_OK) || (pet2d_dirty_rect_poc_rect_in_bounds(&rect) != PET_RESULT_OK)) {
        return PET_RESULT_ERROR;
    }
    ret = pet2d_dirty_rect_poc_rect_for_case(PET2D_DIRTY_RECT_PATTERN_32, 1u, &rect);
    if ((ret != PET_RESULT_OK) || (pet2d_dirty_rect_poc_rect_in_bounds(&rect) != PET_RESULT_OK)) {
        return PET_RESULT_ERROR;
    }
    ret = pet2d_dirty_rect_poc_rect_for_case(PET2D_DIRTY_RECT_PATTERN_16, 2u, &rect);
    if ((ret != PET_RESULT_OK) || (pet2d_dirty_rect_poc_rect_in_bounds(&rect) != PET_RESULT_OK)) {
        return PET_RESULT_ERROR;
    }
    ret = pet2d_dirty_rect_poc_rect_for_case(PET2D_DIRTY_RECT_PATTERN_64, 3u, &rect);
    if ((ret != PET_RESULT_OK) ||
        (pet2d_dirty_rect_poc_rect_in_bounds(&rect) != PET_RESULT_INVALID_ARGUMENT)) {
        return PET_RESULT_ERROR;
    }
    if (pet2d_dirty_rect_poc_rect_for_case(PET2D_DIRTY_RECT_PATTERN_16, 4u, &rect) !=
        PET_RESULT_NOT_FOUND) {
        return PET_RESULT_ERROR;
    }

    return PET_RESULT_OK;
}
