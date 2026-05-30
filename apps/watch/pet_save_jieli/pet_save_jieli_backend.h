#ifndef PET_SAVE_JIELI_BACKEND_H
#define PET_SAVE_JIELI_BACKEND_H

#include "pet_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    PET_SAVE_JIELI_FAULT_NONE = 0,
    PET_SAVE_JIELI_FAULT_BEFORE_WRITE,
    PET_SAVE_JIELI_FAULT_AFTER_HEADER,
    PET_SAVE_JIELI_FAULT_AFTER_PAYLOAD,
    PET_SAVE_JIELI_FAULT_CORRUPT_AFTER_WRITE
} pet_save_jieli_fault_t;

#ifdef __cplusplus
}
#endif

#endif
