#include "pet_growth.h"

static uint16_t pet_growth_clamp_progress_u32(uint32_t value) {
  if (value > 0xFFFFu) {
    return 0xFFFFu;
  }
  return (uint16_t)value;
}

uint16_t pet_growth_progress(const pet_pet_record_t* pet) {
  if (pet == 0) {
    return 0u;
  }
  return (uint16_t)(pet->reserved0 & PET_GROWTH_PROGRESS_MASK);
}

pet_result_t pet_growth_set_progress(pet_pet_record_t* pet, uint16_t progress) {
  if (pet == 0) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  pet->reserved0 = (pet->reserved0 & PET_GROWTH_FATIGUE_MASK) | (uint32_t)progress;
  return PET_RESULT_OK;
}

uint16_t pet_growth_stage_threshold(uint8_t stage) {
  switch (stage) {
    case PET_STAGE_0_CORE:
      return PET_GROWTH_THRESHOLD_STAGE0_TO_1;
    case PET_STAGE_1_BABY_UNKNOWN:
      return PET_GROWTH_THRESHOLD_STAGE1_TO_2;
    case PET_STAGE_2_ATTRIBUTE_REVEALED:
      return PET_GROWTH_THRESHOLD_STAGE2_TO_3;
    case PET_STAGE_3_COCOON:
      return PET_GROWTH_THRESHOLD_STAGE3_TO_4;
    case PET_STAGE_4_SPECIES_BABY:
      return PET_GROWTH_THRESHOLD_STAGE4_TO_5;
    case PET_STAGE_5_FINAL:
      return PET_GROWTH_THRESHOLD_STAGE5_FINAL;
    default:
      return 0u;
  }
}

const char* pet_growth_stage_name(uint8_t stage) {
  return pet_model_stage_name(stage);
}

pet_result_t pet_growth_can_advance_stage(const pet_pet_record_t* pet,
                                          uint8_t* out_can_advance) {
  uint16_t threshold;
  if (out_can_advance != 0) {
    *out_can_advance = 0u;
  }
  if (pet == 0 || out_can_advance == 0 || pet_model_validate(pet) != PET_RESULT_OK) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  if (pet->stage >= PET_STAGE_5_FINAL) {
    return PET_RESULT_OK;
  }
  threshold = pet_growth_stage_threshold(pet->stage);
  if (threshold == 0u) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  *out_can_advance = pet_growth_progress(pet) >= threshold ? 1u : 0u;
  return PET_RESULT_OK;
}

pet_result_t pet_growth_try_advance_stage(pet_pet_record_t* pet,
                                          uint8_t* out_stage_changed,
                                          uint8_t* out_old_stage,
                                          uint8_t* out_new_stage) {
  uint8_t can_advance = 0u;
  uint8_t old_stage;
  if (out_stage_changed != 0) {
    *out_stage_changed = 0u;
  }
  if (out_old_stage != 0) {
    *out_old_stage = 0u;
  }
  if (out_new_stage != 0) {
    *out_new_stage = 0u;
  }
  if (pet == 0 || pet_model_validate(pet) != PET_RESULT_OK) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  old_stage = pet->stage;
  if (out_old_stage != 0) {
    *out_old_stage = old_stage;
  }
  if (out_new_stage != 0) {
    *out_new_stage = old_stage;
  }
  if (pet_growth_can_advance_stage(pet, &can_advance) != PET_RESULT_OK) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  if (can_advance == 0u) {
    return PET_RESULT_OK;
  }
  pet->stage = (uint8_t)(pet->stage + 1u);
  if (out_stage_changed != 0) {
    *out_stage_changed = 1u;
  }
  if (out_new_stage != 0) {
    *out_new_stage = pet->stage;
  }
  return PET_RESULT_OK;
}

pet_result_t pet_growth_apply_progress(pet_pet_record_t* pet,
                                       uint16_t progress_delta,
                                       uint8_t* out_stage_changed) {
  uint32_t next_progress;
  if (out_stage_changed != 0) {
    *out_stage_changed = 0u;
  }
  if (pet == 0 || pet_model_validate(pet) != PET_RESULT_OK) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  next_progress = (uint32_t)pet_growth_progress(pet) + (uint32_t)progress_delta;
  pet_growth_set_progress(pet, pet_growth_clamp_progress_u32(next_progress));
  return pet_growth_try_advance_stage(pet, out_stage_changed, 0, 0);
}
