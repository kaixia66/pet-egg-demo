#include "pet_save_transaction.h"

#include <string.h>

static uint8_t pet_save_transaction_counts_valid(const pet_device_save_payload_t* payload) {
  if (payload->pet_count > PET_MAX_COUNT ||
      payload->activated_nfc_count > PET_ACTIVATED_NFC_MAX ||
      payload->virtual_card_count > PET_VIRTUAL_CARD_MAX ||
      payload->equipment_count > PET_EQUIPMENT_MAX ||
      payload->home_asset_count > PET_HOME_ASSET_MAX) {
    return 0u;
  }
  if (payload->cardbag_summary_count > PET_VIRTUAL_CARD_MAX ||
      payload->equipment_summary_count > PET_EQUIPMENT_MAX ||
      payload->home_asset_summary_count > PET_HOME_ASSET_MAX) {
    return 0u;
  }
  return 1u;
}

static uint8_t pet_save_transaction_species_id_known(uint32_t species_id) {
  return species_id >= 1001u && species_id <= 1018u;
}

static uint8_t pet_save_transaction_pet_record_valid(const pet_pet_record_t* pet) {
  if (pet->pet_id == 0u || pet_save_transaction_species_id_known(pet->species_id) == 0u ||
      pet->stage > 5u || pet->level_coefficient_milli == 0u) {
    return 0u;
  }
  return 1u;
}

pet_result_t pet_save_transaction_begin(const pet_device_save_payload_t* base,
                                        pet_device_save_payload_t* working) {
  if (base == 0 || working == 0) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  *working = *base;
  return PET_RESULT_OK;
}

pet_result_t pet_save_transaction_validate(const pet_device_save_payload_t* working) {
  uint8_t i;
  if (working == 0) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  if (working->schema_version < PET_SAVE_SCHEMA_VERSION_MIN_READ ||
      working->schema_version > PET_SAVE_SCHEMA_VERSION) {
    return PET_RESULT_BAD_VERSION;
  }
  if (pet_save_transaction_counts_valid(working) == 0u) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  if (working->pet_count > 0u && working->active_pet_index >= working->pet_count) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  for (i = 0u; i < working->pet_count; ++i) {
    if (pet_save_transaction_pet_record_valid(&working->pets[i]) == 0u) {
      return PET_RESULT_INVALID_ARGUMENT;
    }
  }
  return PET_RESULT_OK;
}

pet_result_t pet_save_transaction_commit_to_bytes(const pet_device_save_payload_t* working,
                                                  uint64_t counter,
                                                  uint64_t timestamp_ms,
                                                  uint8_t* out_bytes,
                                                  uint32_t out_capacity,
                                                  uint32_t* out_len) {
  size_t serialized_len = 0u;
  pet_result_t result;
  if (out_len != 0) {
    *out_len = 0u;
  }
  if (working == 0 || out_bytes == 0 || out_len == 0) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  result = pet_save_transaction_validate(working);
  if (result != PET_RESULT_OK) {
    return result;
  }
  result = pet_save_slot_serialize(working, counter, timestamp_ms, out_bytes, out_capacity,
                                   &serialized_len);
  if (result != PET_RESULT_OK) {
    return result;
  }
  *out_len = (uint32_t)serialized_len;
  return PET_RESULT_OK;
}

pet_result_t pet_save_transaction_abort(pet_device_save_payload_t* working,
                                        const pet_device_save_payload_t* base) {
  if (working == 0 || base == 0) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  *working = *base;
  return PET_RESULT_OK;
}
