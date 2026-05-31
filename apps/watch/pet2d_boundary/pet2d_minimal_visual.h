#ifndef PET2D_MINIMAL_VISUAL_H
#define PET2D_MINIMAL_VISUAL_H

#include "pet_platform.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PET2D_MINIMAL_VISUAL_WIDTH  16u
#define PET2D_MINIMAL_VISUAL_HEIGHT 16u

typedef struct {
    pet_u16_t width;
    pet_u16_t height;
    pet_u16_t pitch_pixels;
    pet_u16_t *pixels;
} pet2d_minimal_surface_t;

pet_result_t pet2d_minimal_visual_fill_test_pattern(pet2d_minimal_surface_t *surface);
pet_result_t pet2d_minimal_visual_self_test(void);

#ifdef __cplusplus
}
#endif

#endif
