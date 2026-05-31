#ifndef PET2D_DIRTY_RECT_POC_H
#define PET2D_DIRTY_RECT_POC_H

#include "pet2d_minimal_visual.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PET2D_DIRTY_RECT_SIZE_16 16u
#define PET2D_DIRTY_RECT_SIZE_32 32u
#define PET2D_DIRTY_RECT_SIZE_64 64u
#define PET2D_DIRTY_RECT_REPEAT_MAX 60u
#define PET2D_DIRTY_RECT_DEFAULT_REPEAT 10u

typedef enum {
    PET2D_DIRTY_RECT_PATTERN_16 = 0,
    PET2D_DIRTY_RECT_PATTERN_32,
    PET2D_DIRTY_RECT_PATTERN_64,
    PET2D_DIRTY_RECT_PATTERN_MAX
} pet2d_dirty_rect_pattern_t;

typedef struct {
    pet_i16_t x;
    pet_i16_t y;
    pet_u16_t width;
    pet_u16_t height;
} pet2d_dirty_rect_case_t;

pet_u16_t pet2d_dirty_rect_poc_pattern_size(pet2d_dirty_rect_pattern_t pattern);
pet_result_t pet2d_dirty_rect_poc_fill_pattern(pet2d_dirty_rect_pattern_t pattern,
                                               pet2d_minimal_surface_t *surface);
pet_result_t pet2d_dirty_rect_poc_rect_for_case(pet2d_dirty_rect_pattern_t pattern,
                                                pet_u8_t case_index,
                                                pet2d_dirty_rect_case_t *out_rect);
pet_result_t pet2d_dirty_rect_poc_rect_in_bounds(const pet2d_dirty_rect_case_t *rect);
pet_result_t pet2d_dirty_rect_poc_self_test(void);

#ifdef __cplusplus
}
#endif

#endif
