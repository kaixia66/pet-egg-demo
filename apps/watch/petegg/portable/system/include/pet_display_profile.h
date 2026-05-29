#ifndef PETEGG_PORTABLE_PET_DISPLAY_PROFILE_H_
#define PETEGG_PORTABLE_PET_DISPLAY_PROFILE_H_

#include "pet_types.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct PetScreenProfile {
  int32_t width;
  int32_t height;
  PetScreenShape shape;
  int32_t safe_margin_percent;
  int32_t default_scale;
} PetScreenProfile;

#ifdef __cplusplus
}
#endif

#endif  /* PETEGG_PORTABLE_PET_DISPLAY_PROFILE_H_ */
