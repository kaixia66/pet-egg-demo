#ifndef PET_PLATFORM_JIELI_H
#define PET_PLATFORM_JIELI_H

#include "pet_platform.h"

#ifdef __cplusplus
extern "C" {
#endif

void pet_platform_jieli_init(void);
const pet_platform_t *pet_platform_jieli_get(void);
pet_result_t pet_platform_jieli_display_self_test(void);
pet_result_t pet_platform_jieli_input_self_test(void);

#ifdef __cplusplus
}
#endif

#endif
