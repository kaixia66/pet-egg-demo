#ifndef PET_KEY_H
#define PET_KEY_H

#include "pet_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum PetProductKey {
    PET_KEY_LEFT_UP = 0,
    PET_KEY_RIGHT_DOWN = 1,
    PET_KEY_OK = 2,
    PET_KEY_CANCEL = 3,
    PET_KEY_MAX = 4
} PetProductKey;

typedef enum PetKeyEventType {
    PET_KEY_EVENT_DOWN = 0,
    PET_KEY_EVENT_UP = 1,
    PET_KEY_EVENT_CLICK = 2,
    PET_KEY_EVENT_LONG_PRESS = 3,
    PET_KEY_EVENT_REPEAT = 4
} PetKeyEventType;

typedef enum PetPhysicalKey {
    PET_PHYSICAL_KEY_UP = 0,
    PET_PHYSICAL_KEY_DOWN = 1,
    PET_PHYSICAL_KEY_LEFT = 2,
    PET_PHYSICAL_KEY_RIGHT = 3
} PetPhysicalKey;

typedef PetProductKey pet_key_t;
typedef PetKeyEventType pet_key_action_t;

#define PET_KEY_ACTION_DOWN   PET_KEY_EVENT_DOWN
#define PET_KEY_ACTION_UP     PET_KEY_EVENT_UP
#define PET_KEY_ACTION_CLICK  PET_KEY_EVENT_CLICK
#define PET_KEY_ACTION_LONG   PET_KEY_EVENT_LONG_PRESS
#define PET_KEY_ACTION_REPEAT PET_KEY_EVENT_REPEAT

typedef struct PetKeyEvent {
    PetProductKey key;
    PetKeyEventType type;
    pet_u32_t timestamp_ms;
    pet_u32_t hold_ms;
    pet_u16_t repeat_count;
    pet_u16_t raw_code;
} PetKeyEvent;

typedef PetKeyEvent pet_key_event_t;

typedef struct {
    pet_u32_t timestamp_ms;
    pet_u8_t pressed_mask;
    PetProductKey last_key;
    PetKeyEventType last_action;
    pet_u32_t hold_ms[PET_KEY_MAX];
    pet_u16_t repeat_count[PET_KEY_MAX];
    pet_u16_t raw_code[PET_KEY_MAX];
} pet_input_snapshot_t;

#define PET_KEY_MASK(key) ((pet_u8_t)(1u << (pet_u8_t)(key)))

#ifdef __cplusplus
}
#endif

#endif
