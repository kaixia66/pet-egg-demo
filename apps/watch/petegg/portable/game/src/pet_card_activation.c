#include "pet_card_activation.h"

#include "pet_pool.h"
#include "pet_unlock_table.h"

#include <string.h>

static void pet_activation_result_set(pet_card_activation_result_t* out_result,
                                      uint8_t status,
                                      const pet_nfc_card_payload_t* payload,
                                      uint32_t created_id) {
  if (out_result != 0) {
    out_result->status = status;
    out_result->card_type = payload != 0 ? payload->card_type : 0u;
    out_result->content_id = payload != 0 ? payload->content_id : 0u;
    out_result->created_id = created_id;
    out_result->card_uid = payload != 0 ? payload->uid : 0u;
  }
}

static uint32_t pet_activation_next_pet_id(const pet_device_save_payload_t* save) {
  uint32_t max_id = 0u;
  uint8_t i;
  for (i = 0u; i < save->pet_count && i < PET_MAX_COUNT; ++i) {
    if (save->pets[i].pet_id > max_id) {
      max_id = save->pets[i].pet_id;
    }
  }
  return max_id + 1u;
}

static uint8_t pet_activation_expected_unlock_type(uint8_t card_type) {
  switch (card_type) {
    case PET_NFC_CARD_PET:
      return PET_UNLOCK_TYPE_PET_SPECIES;
    case PET_NFC_CARD_FOOD:
      return PET_UNLOCK_TYPE_FOOD_CARD;
    case PET_NFC_CARD_COMPANION:
      return PET_UNLOCK_TYPE_COMPANION_CARD;
    case PET_NFC_CARD_EQUIPMENT:
      return PET_UNLOCK_TYPE_EQUIPMENT;
    case PET_NFC_CARD_HOME:
      return PET_UNLOCK_TYPE_HOME_ASSET;
    default:
      return PET_UNLOCK_TYPE_NONE;
  }
}

static void pet_activation_record_uid(pet_device_save_payload_t* save,
                                      const pet_nfc_card_payload_t* payload) {
  pet_activated_nfc_record_t* record = &save->activated_nfc_records[save->activated_nfc_count];
  memset(record, 0, sizeof(*record));
  record->uid = payload->uid;
  record->card_type = payload->card_type;
  record->content_id = payload->content_id;
  record->activated_counter = (uint32_t)save->activated_nfc_count + 1u;
  ++save->activated_nfc_count;
}

static uint8_t pet_activation_save_counts_valid(const pet_device_save_payload_t* save) {
  return save->pet_count <= PET_MAX_COUNT && save->activated_nfc_count <= PET_ACTIVATED_NFC_MAX &&
         save->virtual_card_count <= PET_VIRTUAL_CARD_MAX &&
         save->equipment_count <= PET_EQUIPMENT_MAX &&
         save->home_asset_count <= PET_HOME_ASSET_MAX;
}

pet_result_t pet_card_activation_check_duplicate(const pet_device_save_payload_t* save,
                                                 uint64_t card_uid,
                                                 uint8_t* out_is_duplicate) {
  uint16_t i;
  if (save == 0 || out_is_duplicate == 0) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  if (save->activated_nfc_count > PET_ACTIVATED_NFC_MAX) {
    return PET_RESULT_INVALID_ARGUMENT;
  }

  *out_is_duplicate = 0u;
  for (i = 0u; i < save->activated_nfc_count; ++i) {
    if (save->activated_nfc_records[i].uid == card_uid) {
      *out_is_duplicate = 1u;
      break;
    }
  }
  return PET_RESULT_OK;
}

pet_result_t pet_card_activation_apply(pet_device_save_payload_t* save,
                                       const pet_nfc_card_payload_t* payload,
                                       pet_card_activation_result_t* out_result) {
  pet_nfc_card_validation_result_t validation;
  const pet_unlock_record_t* unlock;
  pet_device_save_payload_t next;
  uint8_t duplicate = 0u;
  uint8_t expected_unlock_type;
  uint32_t created_id = 0u;

  if (out_result != 0) {
    memset(out_result, 0, sizeof(*out_result));
  }

  if (save == 0 || payload == 0 || out_result == 0) {
    pet_activation_result_set(out_result, PET_CARD_ACTIVATION_INVALID_ARG, payload, 0u);
    return PET_RESULT_INVALID_ARGUMENT;
  }

  if (!pet_activation_save_counts_valid(save)) {
    pet_activation_result_set(out_result, PET_CARD_ACTIVATION_SAVE_CAPACITY_ERROR, payload, 0u);
    return PET_RESULT_OK;
  }

  if (pet_nfc_card_payload_validate(payload, &validation) != PET_RESULT_OK) {
    pet_activation_result_set(out_result, PET_CARD_ACTIVATION_INVALID_ARG, payload, 0u);
    return PET_RESULT_INVALID_ARGUMENT;
  }
  if (validation.status == PET_NFC_CARD_VALIDATION_INVALID_PAYLOAD) {
    pet_activation_result_set(out_result, PET_CARD_ACTIVATION_INVALID_PAYLOAD, payload, 0u);
    return PET_RESULT_OK;
  }
  if (validation.status == PET_NFC_CARD_VALIDATION_INVALID_SIGNATURE) {
    pet_activation_result_set(out_result, PET_CARD_ACTIVATION_INVALID_SIGNATURE, payload, 0u);
    return PET_RESULT_OK;
  }
  if (validation.status == PET_NFC_CARD_VALIDATION_UNSUPPORTED_CARD_TYPE) {
    pet_activation_result_set(out_result, PET_CARD_ACTIVATION_UNSUPPORTED_CARD_TYPE, payload, 0u);
    return PET_RESULT_OK;
  }

  if (pet_card_activation_check_duplicate(save, payload->uid, &duplicate) != PET_RESULT_OK) {
    pet_activation_result_set(out_result, PET_CARD_ACTIVATION_SAVE_CAPACITY_ERROR, payload, 0u);
    return PET_RESULT_OK;
  }
  if (duplicate != 0u) {
    pet_activation_result_set(out_result, PET_CARD_ACTIVATION_DUPLICATE, payload, 0u);
    return PET_RESULT_OK;
  }
  if (save->activated_nfc_count >= PET_ACTIVATED_NFC_MAX) {
    pet_activation_result_set(out_result, PET_CARD_ACTIVATION_SAVE_CAPACITY_ERROR, payload, 0u);
    return PET_RESULT_OK;
  }

  expected_unlock_type = pet_activation_expected_unlock_type(payload->card_type);
  if (expected_unlock_type == PET_UNLOCK_TYPE_NONE) {
    pet_activation_result_set(out_result, PET_CARD_ACTIVATION_UNSUPPORTED_CARD_TYPE, payload,
                              0u);
    return PET_RESULT_OK;
  }
  unlock = pet_unlock_find(payload->content_id);
  if (unlock == 0 || pet_unlock_validate_record(unlock) != PET_RESULT_OK) {
    pet_activation_result_set(out_result, PET_CARD_ACTIVATION_INVALID_PAYLOAD, payload, 0u);
    return PET_RESULT_OK;
  }
  if (unlock->unlock_type != expected_unlock_type) {
    pet_activation_result_set(out_result, PET_CARD_ACTIVATION_UNSUPPORTED_CARD_TYPE, payload,
                              0u);
    return PET_RESULT_OK;
  }

  next = *save;
  switch (payload->card_type) {
    case PET_NFC_CARD_PET: {
      uint8_t pet_index = 0u;
      if (next.pet_count >= PET_MAX_COUNT) {
        pet_activation_result_set(out_result, PET_CARD_ACTIVATION_PET_POOL_FULL, payload, 0u);
        return PET_RESULT_OK;
      }
      created_id = pet_activation_next_pet_id(&next);
      if (pet_pool_add_stage0_from_species(&next, unlock->target_id, created_id, 0u,
                                           &pet_index) != PET_RESULT_OK) {
        pet_activation_result_set(out_result, PET_CARD_ACTIVATION_INVALID_PAYLOAD, payload, 0u);
        return PET_RESULT_OK;
      }
      created_id = next.pets[pet_index].pet_id;
      break;
    }
    case PET_NFC_CARD_FOOD:
    case PET_NFC_CARD_COMPANION: {
      pet_virtual_card_record_t* card;
      if (next.virtual_card_count >= PET_VIRTUAL_CARD_MAX) {
        pet_activation_result_set(out_result, PET_CARD_ACTIVATION_CARD_BAG_FULL, payload, 0u);
        return PET_RESULT_OK;
      }
      card = &next.virtual_cards[next.virtual_card_count];
      memset(card, 0, sizeof(*card));
      card->virtual_card_id = (uint32_t)next.virtual_card_count + 1u;
      card->source_uid = payload->uid;
      card->card_type = payload->card_type;
      card->rarity = unlock->rarity;
      card->content_id = unlock->target_id;
      card->value = unlock->value;
      card->status = PET_CARD_STATUS_UNUSED;
      created_id = card->virtual_card_id;
      ++next.virtual_card_count;
      next.cardbag_summary_count = next.virtual_card_count;
      break;
    }
    case PET_NFC_CARD_EQUIPMENT: {
      pet_equipment_record_t* equipment;
      if (next.equipment_count >= PET_EQUIPMENT_MAX) {
        pet_activation_result_set(out_result, PET_CARD_ACTIVATION_EQUIPMENT_FULL, payload, 0u);
        return PET_RESULT_OK;
      }
      equipment = &next.equipment[next.equipment_count];
      memset(equipment, 0, sizeof(*equipment));
      equipment->equipment_id = (uint32_t)next.equipment_count + 1u;
      equipment->source_uid = payload->uid;
      equipment->rarity = unlock->rarity;
      equipment->content_id = unlock->target_id;
      equipment->value = unlock->value;
      created_id = equipment->equipment_id;
      ++next.equipment_count;
      next.equipment_summary_count = next.equipment_count;
      break;
    }
    case PET_NFC_CARD_HOME: {
      pet_home_asset_record_t* home_asset;
      if (next.home_asset_count >= PET_HOME_ASSET_MAX) {
        pet_activation_result_set(out_result, PET_CARD_ACTIVATION_HOME_ASSET_FULL, payload, 0u);
        return PET_RESULT_OK;
      }
      home_asset = &next.home_assets[next.home_asset_count];
      memset(home_asset, 0, sizeof(*home_asset));
      home_asset->home_asset_id = (uint32_t)next.home_asset_count + 1u;
      home_asset->source_uid = payload->uid;
      home_asset->rarity = unlock->rarity;
      home_asset->content_id = unlock->target_id;
      home_asset->value = unlock->value;
      created_id = home_asset->home_asset_id;
      ++next.home_asset_count;
      next.home_asset_summary_count = next.home_asset_count;
      break;
    }
    default:
      pet_activation_result_set(out_result, PET_CARD_ACTIVATION_UNSUPPORTED_CARD_TYPE, payload,
                                0u);
      return PET_RESULT_OK;
  }

  pet_activation_record_uid(&next, payload);
  *save = next;
  pet_activation_result_set(out_result, PET_CARD_ACTIVATION_OK, payload, created_id);
  return PET_RESULT_OK;
}

const char* pet_card_activation_status_name(uint8_t status) {
  switch (status) {
    case PET_CARD_ACTIVATION_OK:
      return "ok";
    case PET_CARD_ACTIVATION_INVALID_ARG:
      return "invalid_arg";
    case PET_CARD_ACTIVATION_INVALID_PAYLOAD:
      return "invalid_payload";
    case PET_CARD_ACTIVATION_INVALID_SIGNATURE:
      return "invalid_signature";
    case PET_CARD_ACTIVATION_DUPLICATE:
      return "duplicate";
    case PET_CARD_ACTIVATION_PET_POOL_FULL:
      return "pet_pool_full";
    case PET_CARD_ACTIVATION_CARD_BAG_FULL:
      return "card_bag_full";
    case PET_CARD_ACTIVATION_EQUIPMENT_FULL:
      return "equipment_full";
    case PET_CARD_ACTIVATION_HOME_ASSET_FULL:
      return "home_asset_full";
    case PET_CARD_ACTIVATION_UNSUPPORTED_CARD_TYPE:
      return "unsupported_card_type";
    case PET_CARD_ACTIVATION_SAVE_CAPACITY_ERROR:
      return "save_capacity_error";
    default:
      return "unknown";
  }
}
