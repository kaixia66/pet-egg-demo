#include "pet_qte.h"

#include <string.h>

static void pet_qte_zero_result(pet_qte_result_t* out_result, uint8_t status) {
  if (out_result != 0) {
    memset(out_result, 0, sizeof(*out_result));
    out_result->status = status;
  }
}

static uint8_t pet_qte_type_valid(uint8_t qte_type) {
  return qte_type == PET_QTE_TYPE_TAP || qte_type == PET_QTE_TYPE_TIMING ||
         qte_type == PET_QTE_TYPE_HOLD;
}

pet_result_t pet_qte_evaluate(const pet_qte_input_t* input, pet_qte_result_t* out_result) {
  uint32_t hit_score;
  uint16_t timing_score;
  uint16_t normalized;
  uint8_t clamped = 0u;

  pet_qte_zero_result(out_result, PET_QTE_STATUS_INVALID_ARG);
  if (input == 0 || out_result == 0) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  if (pet_qte_type_valid(input->qte_type) == 0u || input->target_count == 0u) {
    pet_qte_zero_result(out_result, PET_QTE_STATUS_INVALID_INPUT);
    return PET_RESULT_INVALID_ARGUMENT;
  }

  hit_score = ((uint32_t)input->hit_count * PET_QTE_SCORE_MAX) / input->target_count;
  if (hit_score > PET_QTE_SCORE_MAX) {
    hit_score = PET_QTE_SCORE_MAX;
    clamped = 1u;
  }
  timing_score = input->timing_score;
  if (timing_score > PET_QTE_SCORE_MAX) {
    timing_score = PET_QTE_SCORE_MAX;
    clamped = 1u;
  }
  if (input->duration_ms > 60000u) {
    clamped = 1u;
  }
  normalized = (uint16_t)((hit_score + (uint32_t)timing_score) / 2u);

  memset(out_result, 0, sizeof(*out_result));
  out_result->normalized_score = normalized;
  out_result->bonus_milli =
      (uint16_t)(PET_QTE_BONUS_MIN_MILLI + (uint16_t)(normalized * 6u));
  if (out_result->bonus_milli > PET_QTE_BONUS_MAX_MILLI) {
    out_result->bonus_milli = PET_QTE_BONUS_MAX_MILLI;
    clamped = 1u;
  }
  out_result->status = clamped != 0u ? PET_QTE_STATUS_CLAMPED : PET_QTE_STATUS_OK;
  out_result->flags = clamped;
  return PET_RESULT_OK;
}

const char* pet_qte_status_name(uint8_t status) {
  switch (status) {
    case PET_QTE_STATUS_OK:
      return "ok";
    case PET_QTE_STATUS_INVALID_ARG:
      return "invalid_arg";
    case PET_QTE_STATUS_INVALID_INPUT:
      return "invalid_input";
    case PET_QTE_STATUS_CLAMPED:
      return "clamped";
    default:
      return "unknown";
  }
}
