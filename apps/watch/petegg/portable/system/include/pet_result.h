#ifndef PETEGG_PORTABLE_PET_RESULT_H_
#define PETEGG_PORTABLE_PET_RESULT_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum PetResult {
  PET_RESULT_OK = 0,
  PET_RESULT_INVALID_ARGUMENT = 1,
  PET_RESULT_BUFFER_TOO_SMALL = 2,
  PET_RESULT_UNSUPPORTED = 3,
  PET_RESULT_DUPLICATE = 4,
  PET_RESULT_FULL = 5,
  PET_RESULT_BAD_CRC = 6,
  PET_RESULT_BAD_VERSION = 7,
  PET_RESULT_STORAGE_ERROR = 8
} PetResult;

typedef PetResult pet_result_t;

#ifdef __cplusplus
}
#endif

#endif  /* PETEGG_PORTABLE_PET_RESULT_H_ */
