#include "pet_model.h"

#include <string.h>

static void pet_model_set_fixed_name(char out[PET_PET_NAME_MAX], const char* name) {
  size_t i;
  memset(out, 0, PET_PET_NAME_MAX);
  if (name == 0) {
    return;
  }
  for (i = 0u; i + 1u < PET_PET_NAME_MAX && name[i] != '\0'; ++i) {
    out[i] = name[i];
  }
}

static uint16_t pet_model_clamp_score(int32_t value) {
  if (value < 0) {
    return 0u;
  }
  if (value > 1000) {
    return 1000u;
  }
  return (uint16_t)value;
}

static void pet_model_set_attribute_factors(pet_pet_record_t* pet, uint8_t attribute) {
  pet->wood_factor = PET_MODEL_SECONDARY_FACTOR;
  pet->fire_factor = PET_MODEL_SECONDARY_FACTOR;
  pet->water_factor = PET_MODEL_SECONDARY_FACTOR;
  if (attribute == PET_ATTRIBUTE_SPIRIT_WOOD) {
    pet->wood_factor = PET_MODEL_DOMINANT_FACTOR;
  } else if (attribute == PET_ATTRIBUTE_BLAZING_FIRE) {
    pet->fire_factor = PET_MODEL_DOMINANT_FACTOR;
  } else if (attribute == PET_ATTRIBUTE_DARK_WATER) {
    pet->water_factor = PET_MODEL_DOMINANT_FACTOR;
  }
}

pet_result_t pet_model_init_stage0(pet_pet_record_t* pet,
                                   uint16_t species_id,
                                   uint32_t pet_instance_id,
                                   const char* nickname) {
  const pet_species_record_t* species;
  const char* final_name;
  if (pet == 0 || pet_instance_id == 0u) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  species = pet_species_find(species_id);
  if (species == 0) {
    return PET_RESULT_INVALID_ARGUMENT;
  }

  memset(pet, 0, sizeof(*pet));
  pet->pet_id = pet_instance_id;
  pet->species_id = species_id;
  final_name = nickname != 0 && nickname[0] != '\0' ? nickname : species->display_name;
  pet_model_set_fixed_name(pet->nickname, final_name);
  pet->stage = PET_STAGE_0_CORE;
  pet->attribute = species->attribute;
  pet->status = PET_STATUS_FLAG_NONE;
  pet_model_set_attribute_factors(pet, species->attribute);
  pet->care_score = PET_MODEL_DEFAULT_CARE_SCORE;
  pet->bond_score = PET_MODEL_DEFAULT_BOND_SCORE;
  pet->level_coefficient_milli = PET_MODEL_DEFAULT_LEVEL_COEFFICIENT_MILLI;
  return PET_RESULT_OK;
}

pet_result_t pet_model_validate(const pet_pet_record_t* pet) {
  const pet_species_record_t* species;
  if (pet == 0 || pet->pet_id == 0u) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  species = pet_species_find((uint16_t)pet->species_id);
  if (species == 0) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  if (pet->stage > PET_STAGE_MAX || pet->attribute != species->attribute ||
      pet->level_coefficient_milli == 0u) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  return PET_RESULT_OK;
}

pet_result_t pet_model_apply_basic_care_delta(pet_pet_record_t* pet,
                                              int16_t care_delta,
                                              int16_t bond_delta) {
  if (pet_model_validate(pet) != PET_RESULT_OK) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  pet->care_score = pet_model_clamp_score((int32_t)pet->care_score + (int32_t)care_delta);
  pet->bond_score = pet_model_clamp_score((int32_t)pet->bond_score + (int32_t)bond_delta);
  return PET_RESULT_OK;
}

const char* pet_model_stage_name(uint8_t stage) {
  switch (stage) {
    case PET_STAGE_0_CORE:
      return "stage0_core";
    case PET_STAGE_1_BABY_UNKNOWN:
      return "stage1_baby_unknown";
    case PET_STAGE_2_ATTRIBUTE_REVEALED:
      return "stage2_attribute_revealed";
    case PET_STAGE_3_COCOON:
      return "stage3_cocoon";
    case PET_STAGE_4_SPECIES_BABY:
      return "stage4_species_baby";
    case PET_STAGE_5_FINAL:
      return "stage5_final";
    default:
      return "unknown";
  }
}

const char* pet_model_attribute_name(uint8_t attribute) {
  switch (attribute) {
    case PET_ATTRIBUTE_SPIRIT_WOOD:
      return "spirit_wood";
    case PET_ATTRIBUTE_BLAZING_FIRE:
      return "blazing_fire";
    case PET_ATTRIBUTE_DARK_WATER:
      return "dark_water";
    case PET_ATTRIBUTE_NONE:
      return "none";
    default:
      return "unknown";
  }
}
