#include "pet2d_resource_sprite_poc.h"

#include "pet_resource_jieli.h"
#include "pet_resource_jieli_test_blob.h"

static pet_result_t pet2d_resource_sprite_expected_size(pet_u32_t resource_id,
                                                        pet_u16_t *out_width,
                                                        pet_u16_t *out_height)
{
    if ((out_width == 0) || (out_height == 0)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    if (resource_id == PET2D_RESOURCE_SPRITE_POC_ID_4X4) {
        *out_width = 4u;
        *out_height = 4u;
        return PET_RESULT_OK;
    }
    if (resource_id == PET2D_RESOURCE_SPRITE_POC_ID_8X8) {
        *out_width = 8u;
        *out_height = 8u;
        return PET_RESULT_OK;
    }

    return PET_RESULT_NOT_FOUND;
}

static pet_u16_t pet2d_resource_sprite_read_pixel(const pet2d_resource_sprite_view_t *sprite,
                                                  pet_u16_t x,
                                                  pet_u16_t y)
{
    const pet_u8_t *bytes = (const pet_u8_t *)sprite->pixels;
    pet_u32_t offset = (((pet_u32_t)y * sprite->pitch_pixels) + x) * 2u;

    return (pet_u16_t)(((pet_u16_t)bytes[offset]) | (((pet_u16_t)bytes[offset + 1u]) << 8));
}

pet_result_t pet2d_resource_sprite_poc_open(pet_u32_t resource_id,
                                            pet2d_resource_sprite_view_t *out_view)
{
    pet_resource_entry_t entry;
    const pet_u8_t *data;
    pet_u32_t data_size;
    pet_u16_t expected_w;
    pet_u16_t expected_h;
    pet_result_t ret;

    if (out_view == 0) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    ret = pet2d_resource_sprite_expected_size(resource_id, &expected_w, &expected_h);
    if (ret != PET_RESULT_OK) {
        return ret;
    }

    ret = pet_resource_jieli_open_blob(pet_resource_jieli_test_blob,
                                       pet_resource_jieli_test_blob_size);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    ret = pet_resource_jieli_validate_manifest();
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    ret = pet_resource_jieli_find_entry_by_id(resource_id, &entry);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    if ((entry.resource_type != PET_RESOURCE_TYPE_SPRITE) ||
        (entry.format != PET_RESOURCE_FORMAT_RGB565) ||
        (entry.width != expected_w) ||
        (entry.height != expected_h) ||
        (entry.size != ((pet_u32_t)expected_w * expected_h * 2u))) {
        return PET_RESULT_BAD_VERSION;
    }

    ret = pet_resource_jieli_read_entry(resource_id, &data, &data_size);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    if ((data == 0) || (data_size != entry.size)) {
        return PET_RESULT_BUFFER_TOO_SMALL;
    }

    out_view->width = entry.width;
    out_view->height = entry.height;
    out_view->pitch_pixels = entry.width;
    out_view->pixels = (const pet_u16_t *)data;
    out_view->resource_id = resource_id;
    return PET_RESULT_OK;
}

pet_result_t pet2d_minimal_visual_blit_sprite(pet2d_minimal_surface_t *dst,
                                              int dst_x,
                                              int dst_y,
                                              const pet2d_resource_sprite_view_t *sprite)
{
    pet_u16_t sx;
    pet_u16_t sy;

    if ((dst == 0) || (dst->pixels == 0) || (sprite == 0) || (sprite->pixels == 0)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }
    if ((dst->width == 0u) || (dst->height == 0u) ||
        (dst->pitch_pixels < dst->width) ||
        (sprite->width == 0u) || (sprite->height == 0u) ||
        (sprite->pitch_pixels < sprite->width)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    for (sy = 0u; sy < sprite->height; sy++) {
        int dy = dst_y + (int)sy;
        if ((dy < 0) || (dy >= (int)dst->height)) {
            continue;
        }
        for (sx = 0u; sx < sprite->width; sx++) {
            int dx = dst_x + (int)sx;
            if ((dx < 0) || (dx >= (int)dst->width)) {
                continue;
            }
            dst->pixels[((pet_u32_t)dy * dst->pitch_pixels) + (pet_u16_t)dx] =
                pet2d_resource_sprite_read_pixel(sprite, sx, sy);
        }
    }

    return PET_RESULT_OK;
}

pet_result_t pet2d_resource_sprite_poc_self_test(void)
{
    static pet_u16_t surface_pixels[32u * 32u];
    pet2d_minimal_surface_t surface;
    pet2d_resource_sprite_view_t sprite4;
    pet2d_resource_sprite_view_t sprite8;
    pet_result_t ret;
    pet_u32_t i;

    if (pet2d_resource_sprite_poc_open(PET2D_RESOURCE_SPRITE_POC_ID_4X4, 0) !=
        PET_RESULT_INVALID_ARGUMENT) {
        return PET_RESULT_ERROR;
    }
    if (pet2d_resource_sprite_poc_open(9999u, &sprite4) != PET_RESULT_NOT_FOUND) {
        return PET_RESULT_ERROR;
    }

    ret = pet2d_resource_sprite_poc_open(PET2D_RESOURCE_SPRITE_POC_ID_4X4, &sprite4);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    if ((sprite4.width != 4u) || (sprite4.height != 4u) ||
        (sprite4.resource_id != PET2D_RESOURCE_SPRITE_POC_ID_4X4)) {
        return PET_RESULT_ERROR;
    }

    ret = pet2d_resource_sprite_poc_open(PET2D_RESOURCE_SPRITE_POC_ID_8X8, &sprite8);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    if ((sprite8.width != 8u) || (sprite8.height != 8u) ||
        (sprite8.resource_id != PET2D_RESOURCE_SPRITE_POC_ID_8X8)) {
        return PET_RESULT_ERROR;
    }

    for (i = 0u; i < (pet_u32_t)(32u * 32u); i++) {
        surface_pixels[i] = 0u;
    }
    surface.width = 32u;
    surface.height = 32u;
    surface.pitch_pixels = 32u;
    surface.pixels = surface_pixels;

    ret = pet2d_minimal_visual_blit_sprite(&surface, 14, 14, &sprite4);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    if ((surface_pixels[(14u * 32u) + 14u] != 0x2211u) ||
        (surface_pixels[(14u * 32u) + 15u] != 0x4433u) ||
        (surface_pixels[(17u * 32u) + 17u] != 0x4433u)) {
        return PET_RESULT_ERROR;
    }

    ret = pet2d_minimal_visual_blit_sprite(&surface, 1, 1, &sprite8);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    if (surface_pixels[(1u * 32u) + 1u] != 0x0401u) {
        return PET_RESULT_ERROR;
    }

    ret = pet2d_minimal_visual_blit_sprite(&surface, -2, -2, &sprite4);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    if (surface_pixels[0u] == 0u) {
        return PET_RESULT_ERROR;
    }

    if (pet2d_minimal_visual_blit_sprite(0, 0, 0, &sprite4) !=
        PET_RESULT_INVALID_ARGUMENT) {
        return PET_RESULT_ERROR;
    }

    surface.pitch_pixels = 31u;
    if (pet2d_minimal_visual_blit_sprite(&surface, 0, 0, &sprite4) !=
        PET_RESULT_INVALID_ARGUMENT) {
        return PET_RESULT_ERROR;
    }

    return PET_RESULT_OK;
}
