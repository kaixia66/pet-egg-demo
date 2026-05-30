#ifndef PET2D_BOUNDARY_H
#define PET2D_BOUNDARY_H

#include "pet_platform.h"

#ifdef __cplusplus
extern "C" {
#endif

pet_result_t pet2d_boundary_enter_placeholder(void);
pet_result_t pet2d_boundary_exit_placeholder(void);
pet_result_t pet2d_boundary_self_test(void);
pet_result_t pet2d_boundary_resource_probe_self_test(void);
pet_result_t pet2d_boundary_tiny_visual_probe(void);

#ifdef __cplusplus
}
#endif

#endif
