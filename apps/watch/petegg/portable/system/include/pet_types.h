#ifndef PETEGG_PORTABLE_PET_TYPES_H_
#define PETEGG_PORTABLE_PET_TYPES_H_

#include "pet_config.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum PetDeviceId {
  PET_DEVICE_A = 0,
  PET_DEVICE_B = 1
} PetDeviceId;

typedef enum PetScreenShape {
  PET_SCREEN_SHAPE_CIRCLE = 0,
  PET_SCREEN_SHAPE_RECTANGLE = 1
} PetScreenShape;

typedef enum PetSaveSlot {
  PET_SAVE_SLOT_A = 0,
  PET_SAVE_SLOT_B = 1,
  PET_SAVE_SLOT_NONE = 255
} PetSaveSlot;

#ifdef __cplusplus
}
#endif

#endif  /* PETEGG_PORTABLE_PET_TYPES_H_ */
