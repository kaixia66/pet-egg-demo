#ifndef PETEGG_PORTABLE_PET_INPUT_H_
#define PETEGG_PORTABLE_PET_INPUT_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum PetProductKey {
  PET_KEY_LEFT_UP = 0,
  PET_KEY_RIGHT_DOWN = 1,
  PET_KEY_OK = 2,
  PET_KEY_CANCEL = 3
} PetProductKey;

typedef enum PetKeyEventType {
  PET_KEY_EVENT_DOWN = 0,
  PET_KEY_EVENT_UP = 1,
  PET_KEY_EVENT_CLICK = 2,
  PET_KEY_EVENT_LONG_PRESS = 3,
  PET_KEY_EVENT_REPEAT = 4
} PetKeyEventType;

typedef struct PetKeyEvent {
  PetProductKey key;
  PetKeyEventType type;
} PetKeyEvent;

typedef enum PetPhysicalKey {
  PET_PHYSICAL_KEY_UP = 0,
  PET_PHYSICAL_KEY_DOWN = 1,
  PET_PHYSICAL_KEY_LEFT = 2,
  PET_PHYSICAL_KEY_RIGHT = 3
} PetPhysicalKey;

#ifdef __cplusplus
}
#endif

#endif  /* PETEGG_PORTABLE_PET_INPUT_H_ */
