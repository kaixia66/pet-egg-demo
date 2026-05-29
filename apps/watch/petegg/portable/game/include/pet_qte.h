#ifndef PETEGG_PORTABLE_PET_QTE_H_
#define PETEGG_PORTABLE_PET_QTE_H_

#include "pet_result.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Portable Game Core QTE scoring. UI/input timing collection happens outside this API. */
#define PET_QTE_TYPE_TAP 1u
#define PET_QTE_TYPE_TIMING 2u
#define PET_QTE_TYPE_HOLD 3u

#define PET_QTE_SCORE_MIN 0u
#define PET_QTE_SCORE_MAX 100u
#define PET_QTE_BONUS_MIN_MILLI 900u
#define PET_QTE_BONUS_MAX_MILLI 1500u

typedef enum pet_qte_status_t {
  PET_QTE_STATUS_OK = 0,
  PET_QTE_STATUS_INVALID_ARG = 1,
  PET_QTE_STATUS_INVALID_INPUT = 2,
  PET_QTE_STATUS_CLAMPED = 3
} pet_qte_status_t;

typedef struct pet_qte_input_t {
  uint8_t qte_type;
  uint16_t hit_count;
  uint16_t target_count;
  uint16_t timing_score;
  uint32_t duration_ms;
  uint8_t flags;
} pet_qte_input_t;

typedef struct pet_qte_result_t {
  uint16_t normalized_score;
  uint16_t bonus_milli;
  uint8_t status;
  uint8_t flags;
} pet_qte_result_t;

typedef pet_qte_status_t PetQteStatus;
typedef pet_qte_input_t PetQteInput;
typedef pet_qte_result_t PetQteResult;

pet_result_t pet_qte_evaluate(const pet_qte_input_t* input, pet_qte_result_t* out_result);
const char* pet_qte_status_name(uint8_t status);

#ifdef __cplusplus
}
#endif

#endif  /* PETEGG_PORTABLE_PET_QTE_H_ */
