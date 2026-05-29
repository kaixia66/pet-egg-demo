#ifndef PETEGG_PORTABLE_PET_CARE_H_
#define PETEGG_PORTABLE_PET_CARE_H_

#include "pet_growth.h"
#include "pet_result.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Portable Game Core minimal care rules. Care is deterministic and does not call time,
   storage, files, Flash, hardware integration, UI frameworks, or platform services. */
#define PET_CARE_ACTION_FEED_DAILY 1u
#define PET_CARE_ACTION_PLAY_COMPANION 2u
#define PET_CARE_ACTION_CLEAN_HOME 3u
#define PET_CARE_ACTION_REPAIR_CORE 4u
#define PET_CARE_ACTION_REST 5u
#define PET_CARE_ACTION_WAKE 6u

#define PET_CARE_SCORE_MIN 0u
#define PET_CARE_SCORE_MAX 1000u
#define PET_CARE_FATIGUE_MIN 0u
#define PET_CARE_FATIGUE_MAX 1000u
#define PET_CARE_FATIGUE_LIMITED_THRESHOLD 850u
#define PET_CARE_FATIGUE_CLEAR_THRESHOLD 500u

#define PET_CARE_RESULT_FLAG_NONE 0u
#define PET_CARE_RESULT_FLAG_CLAMPED 1u
#define PET_CARE_RESULT_FLAG_LIMITED_BY_FATIGUE 2u
#define PET_CARE_RESULT_FLAG_WOKE 4u
#define PET_CARE_RESULT_FLAG_STAGE_CHANGED 8u

typedef enum pet_care_status_t {
  PET_CARE_STATUS_OK = 0,
  PET_CARE_STATUS_INVALID_ARG = 1,
  PET_CARE_STATUS_INVALID_ACTION = 2,
  PET_CARE_STATUS_LIMITED_BY_FATIGUE = 3,
  PET_CARE_STATUS_ALREADY_AWAKE = 4,
  PET_CARE_STATUS_NOT_SLEEPING = 5,
  PET_CARE_STATUS_CLAMPED = 6
} pet_care_status_t;

typedef struct pet_care_request_t {
  uint8_t action_type;
  uint64_t now_ms;
  uint16_t intensity;
  uint8_t flags;
} pet_care_request_t;

typedef struct pet_care_result_t {
  uint8_t status;
  int16_t hunger_delta;
  int16_t mood_delta;
  int16_t health_delta;
  int16_t bond_delta;
  int16_t growth_delta;
  int16_t care_score_delta;
  int16_t bond_score_delta;
  int16_t fatigue_delta;
  uint8_t stage_changed;
  uint8_t old_stage;
  uint8_t new_stage;
  uint8_t flags;
} pet_care_result_t;

typedef pet_care_status_t PetCareStatus;
typedef pet_care_request_t PetCareRequest;
typedef pet_care_result_t PetCareResult;

uint16_t pet_care_fatigue(const pet_pet_record_t* pet);
pet_result_t pet_care_set_fatigue(pet_pet_record_t* pet, uint16_t fatigue);
pet_result_t pet_care_preview(const pet_pet_record_t* pet,
                              const pet_care_request_t* request,
                              pet_care_result_t* out_result);
pet_result_t pet_care_apply(pet_pet_record_t* pet,
                            const pet_care_request_t* request,
                            pet_care_result_t* out_result);
const char* pet_care_action_name(uint8_t action_type);
const char* pet_care_status_name(uint8_t status);

#ifdef __cplusplus
}
#endif

#endif  /* PETEGG_PORTABLE_PET_CARE_H_ */
