#ifndef PETEGG_PORTABLE_PET_PLATFORM_H_
#define PETEGG_PORTABLE_PET_PLATFORM_H_

#include "pet_result.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t (*PetPlatformMillisFn)(void* user);
typedef void (*PetPlatformLogFn)(void* user, const char* message);
typedef PetResult (*PetPlatformSaveReadFn)(void* user,
                                           uint8_t slot,
                                           uint8_t* out_bytes,
                                           size_t out_capacity,
                                           size_t* out_len);
typedef PetResult (*PetPlatformSaveWriteFn)(void* user,
                                            uint8_t slot,
                                            const uint8_t* bytes,
                                            size_t len);

typedef struct PetPlatformCallbacks {
  void* user;
  PetPlatformMillisFn millis;
  PetPlatformLogFn log;
  PetPlatformSaveReadFn save_read;
  PetPlatformSaveWriteFn save_write;
} PetPlatformCallbacks;

#ifdef __cplusplus
}
#endif

#endif  /* PETEGG_PORTABLE_PET_PLATFORM_H_ */
