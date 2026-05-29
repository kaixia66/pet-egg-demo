#include "pet_pool.h"

#include <string.h>

pet_result_t pet_pool_init_empty(pet_device_save_payload_t* save) {
  if (save == 0) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  save->pet_count = 0u;
  save->active_pet_index = 0u;
  memset(save->pets, 0, sizeof(save->pets));
  return PET_RESULT_OK;
}

pet_result_t pet_pool_add_pet(pet_device_save_payload_t* save,
                              uint16_t species_id,
                              uint32_t pet_instance_id,
                              const char* nickname,
                              uint8_t set_active,
                              uint8_t* out_index) {
  pet_device_save_payload_t next;
  uint8_t index;
  if (out_index != 0) {
    *out_index = 0u;
  }
  if (save == 0) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  if (save->pet_count >= PET_MAX_COUNT) {
    return PET_RESULT_FULL;
  }
  if (pet_pool_validate(save) != PET_RESULT_OK && save->pet_count != 0u) {
    return PET_RESULT_INVALID_ARGUMENT;
  }

  next = *save;
  index = next.pet_count;
  if (pet_model_init_stage0(&next.pets[index], species_id, pet_instance_id, nickname) !=
      PET_RESULT_OK) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  next.pet_count = (uint8_t)(next.pet_count + 1u);
  if (set_active != 0u || next.pet_count == 1u) {
    next.active_pet_index = index;
    next.pets[index].status |= PET_STATUS_FLAG_ACTIVE;
  }
  *save = next;
  if (out_index != 0) {
    *out_index = index;
  }
  return PET_RESULT_OK;
}

pet_result_t pet_pool_add_stage0_from_species(pet_device_save_payload_t* save,
                                              uint16_t species_id,
                                              uint32_t pet_instance_id,
                                              uint8_t set_active,
                                              uint8_t* out_index) {
  return pet_pool_add_pet(save, species_id, pet_instance_id, 0, set_active, out_index);
}

pet_result_t pet_pool_get_active(pet_device_save_payload_t* save,
                                 pet_pet_record_t** out_pet) {
  if (save == 0 || out_pet == 0) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  if (pet_pool_validate(save) != PET_RESULT_OK || save->pet_count == 0u) {
    *out_pet = 0;
    return PET_RESULT_INVALID_ARGUMENT;
  }
  *out_pet = &save->pets[save->active_pet_index];
  return PET_RESULT_OK;
}

pet_result_t pet_pool_get_active_const(const pet_device_save_payload_t* save,
                                       const pet_pet_record_t** out_pet) {
  if (save == 0 || out_pet == 0) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  if (pet_pool_validate(save) != PET_RESULT_OK || save->pet_count == 0u) {
    *out_pet = 0;
    return PET_RESULT_INVALID_ARGUMENT;
  }
  *out_pet = &save->pets[save->active_pet_index];
  return PET_RESULT_OK;
}

pet_result_t pet_pool_set_active(pet_device_save_payload_t* save, uint8_t index) {
  pet_device_save_payload_t next;
  uint8_t i;
  if (save == 0 || pet_pool_validate(save) != PET_RESULT_OK || index >= save->pet_count) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  next = *save;
  for (i = 0u; i < next.pet_count; ++i) {
    next.pets[i].status = (uint16_t)(next.pets[i].status & ~PET_STATUS_FLAG_ACTIVE);
  }
  next.active_pet_index = index;
  next.pets[index].status |= PET_STATUS_FLAG_ACTIVE;
  *save = next;
  return PET_RESULT_OK;
}

pet_result_t pet_pool_find_by_instance_id(pet_device_save_payload_t* save,
                                          uint32_t pet_instance_id,
                                          pet_pet_record_t** out_pet,
                                          uint8_t* out_index) {
  uint8_t i;
  if (save == 0 || out_pet == 0 || pet_instance_id == 0u) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  *out_pet = 0;
  if (out_index != 0) {
    *out_index = 0u;
  }
  if (save->pet_count > PET_MAX_COUNT) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  for (i = 0u; i < save->pet_count; ++i) {
    if (save->pets[i].pet_id == pet_instance_id) {
      *out_pet = &save->pets[i];
      if (out_index != 0) {
        *out_index = i;
      }
      return PET_RESULT_OK;
    }
  }
  return PET_RESULT_INVALID_ARGUMENT;
}

pet_result_t pet_pool_find_by_species_id(pet_device_save_payload_t* save,
                                         uint16_t species_id,
                                         pet_pet_record_t** out_pet,
                                         uint8_t* out_index) {
  uint8_t i;
  if (save == 0 || out_pet == 0 || pet_species_find(species_id) == 0) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  *out_pet = 0;
  if (out_index != 0) {
    *out_index = 0u;
  }
  if (save->pet_count > PET_MAX_COUNT) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  for (i = 0u; i < save->pet_count; ++i) {
    if (save->pets[i].species_id == species_id) {
      *out_pet = &save->pets[i];
      if (out_index != 0) {
        *out_index = i;
      }
      return PET_RESULT_OK;
    }
  }
  return PET_RESULT_INVALID_ARGUMENT;
}

pet_result_t pet_pool_validate(const pet_device_save_payload_t* save) {
  uint8_t i;
  if (save == 0 || save->pet_count > PET_MAX_COUNT) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  if (save->pet_count == 0u) {
    return PET_RESULT_OK;
  }
  if (save->active_pet_index >= save->pet_count) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  for (i = 0u; i < save->pet_count; ++i) {
    if (pet_model_validate(&save->pets[i]) != PET_RESULT_OK) {
      return PET_RESULT_INVALID_ARGUMENT;
    }
  }
  return PET_RESULT_OK;
}
