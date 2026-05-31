#ifndef PET2D_RESOURCE_SPRITE_POC_H
#define PET2D_RESOURCE_SPRITE_POC_H

#include "pet2d_minimal_visual.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PET2D_RESOURCE_SPRITE_POC_ID_4X4 2001u
#define PET2D_RESOURCE_SPRITE_POC_ID_8X8 1001u

typedef struct {
    pet_u16_t width;
    pet_u16_t height;
    pet_u16_t pitch_pixels;
    const pet_u16_t *pixels;
    pet_u32_t resource_id;
} pet2d_resource_sprite_view_t;

pet_result_t pet2d_resource_sprite_poc_open(pet_u32_t resource_id,
                                            pet2d_resource_sprite_view_t *out_view);
pet_result_t pet2d_minimal_visual_blit_sprite(pet2d_minimal_surface_t *dst,
                                              int dst_x,
                                              int dst_y,
                                              const pet2d_resource_sprite_view_t *sprite);
pet_result_t pet2d_resource_sprite_poc_self_test(void);

#ifdef __cplusplus
}
#endif

#endif
