#include "pet_nfc_payload.h"

uint8_t pet_nfc_card_type_is_supported(uint8_t card_type) {
  switch (card_type) {
    case PET_NFC_CARD_PET:
    case PET_NFC_CARD_FOOD:
    case PET_NFC_CARD_COMPANION:
    case PET_NFC_CARD_EQUIPMENT:
    case PET_NFC_CARD_HOME:
      return 1u;
    default:
      return 0u;
  }
}

uint32_t pet_nfc_card_payload_expected_mock_signature(const pet_nfc_card_payload_t* payload) {
  uint64_t mixed;
  if (payload == 0) {
    return 0u;
  }

  mixed = payload->uid ^ 0x5045544547474E46ull;
  mixed ^= (uint64_t)payload->card_type << 8u;
  mixed ^= (uint64_t)payload->rarity << 16u;
  mixed ^= (uint64_t)payload->content_id << 24u;
  mixed ^= (uint64_t)payload->value << 40u;
  mixed ^= (uint64_t)payload->flags << 52u;
  mixed ^= mixed >> 33u;
  mixed *= 0xff51afd7ed558ccdull;
  mixed ^= mixed >> 33u;
  mixed *= 0xc4ceb9fe1a85ec53ull;
  mixed ^= mixed >> 33u;
  return (uint32_t)(mixed ^ (mixed >> 32u));
}

static void pet_nfc_validation_set(pet_nfc_card_validation_result_t* out_result,
                                   uint8_t status,
                                   const pet_nfc_card_payload_t* payload,
                                   uint32_t expected_signature) {
  out_result->status = status;
  out_result->card_type = payload != 0 ? payload->card_type : 0u;
  out_result->content_id = payload != 0 ? payload->content_id : 0u;
  out_result->expected_mock_signature = expected_signature;
}

pet_result_t pet_nfc_card_payload_validate(const pet_nfc_card_payload_t* payload,
                                           pet_nfc_card_validation_result_t* out_result) {
  uint32_t expected_signature;
  if (out_result == 0) {
    return PET_RESULT_INVALID_ARGUMENT;
  }

  if (payload == 0) {
    pet_nfc_validation_set(out_result, PET_NFC_CARD_VALIDATION_INVALID_ARG, payload, 0u);
    return PET_RESULT_INVALID_ARGUMENT;
  }

  expected_signature = pet_nfc_card_payload_expected_mock_signature(payload);
  if (payload->uid == 0u || payload->content_id == 0u) {
    pet_nfc_validation_set(out_result, PET_NFC_CARD_VALIDATION_INVALID_PAYLOAD, payload,
                           expected_signature);
    return PET_RESULT_OK;
  }
  if (payload->mock_signature != expected_signature) {
    pet_nfc_validation_set(out_result, PET_NFC_CARD_VALIDATION_INVALID_SIGNATURE, payload,
                           expected_signature);
    return PET_RESULT_OK;
  }
  if (!pet_nfc_card_type_is_supported(payload->card_type)) {
    pet_nfc_validation_set(out_result, PET_NFC_CARD_VALIDATION_UNSUPPORTED_CARD_TYPE, payload,
                           expected_signature);
    return PET_RESULT_OK;
  }

  pet_nfc_validation_set(out_result, PET_NFC_CARD_VALIDATION_OK, payload, expected_signature);
  return PET_RESULT_OK;
}

const char* pet_nfc_card_validation_status_name(uint8_t status) {
  switch (status) {
    case PET_NFC_CARD_VALIDATION_OK:
      return "ok";
    case PET_NFC_CARD_VALIDATION_INVALID_ARG:
      return "invalid_arg";
    case PET_NFC_CARD_VALIDATION_INVALID_PAYLOAD:
      return "invalid_payload";
    case PET_NFC_CARD_VALIDATION_UNSUPPORTED_CARD_TYPE:
      return "unsupported_card_type";
    case PET_NFC_CARD_VALIDATION_INVALID_SIGNATURE:
      return "invalid_signature";
    default:
      return "unknown";
  }
}
