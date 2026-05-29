#ifndef PETEGG_PORTABLE_PET_NFC_PAYLOAD_H_
#define PETEGG_PORTABLE_PET_NFC_PAYLOAD_H_

#include "pet_result.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* NFC content-card activation payload.
   This is not the device-to-device NFC pair payload. It contains small metadata only;
   pets, items, animations, config, and resource bytes are preloaded locally or in
   external Flash. Multi-byte fields are serialized little-endian when stored or sent.
   mock_signature is simulator/test-only and must be replaced by a secure tag
   signature/counter check for physical NFC cards. */
#define PET_NFC_CARD_PET 1u
#define PET_NFC_CARD_FOOD 2u
#define PET_NFC_CARD_COMPANION 3u
#define PET_NFC_CARD_EQUIPMENT 4u
#define PET_NFC_CARD_HOME 5u

#define PET_NFC_CARD_FLAG_NONE 0u
#define PET_NFC_CARD_FLAG_TEST_PAYLOAD 1u
#define PET_NFC_CARD_PAYLOAD_SERIALIZED_SIZE 20u

#define PET_CARD_STATUS_UNUSED 1u
#define PET_CARD_STATUS_USED 2u
#define PET_CARD_STATUS_EQUIPPABLE 3u
#define PET_CARD_STATUS_HOME_AVAILABLE 4u

typedef enum pet_nfc_card_validation_status_t {
  PET_NFC_CARD_VALIDATION_OK = 0,
  PET_NFC_CARD_VALIDATION_INVALID_ARG = 1,
  PET_NFC_CARD_VALIDATION_INVALID_PAYLOAD = 2,
  PET_NFC_CARD_VALIDATION_UNSUPPORTED_CARD_TYPE = 3,
  PET_NFC_CARD_VALIDATION_INVALID_SIGNATURE = 4
} pet_nfc_card_validation_status_t;

#if defined(_MSC_VER)
#pragma pack(push, 1)
#define PET_NFC_PACKED_STRUCT
#elif defined(__GNUC__) || defined(__clang__)
#define PET_NFC_PACKED_STRUCT __attribute__((packed))
#else
#pragma pack(push, 1)
#define PET_NFC_PAYLOAD_PACK_POP 1
#define PET_NFC_PACKED_STRUCT
#endif

typedef struct PET_NFC_PACKED_STRUCT pet_nfc_card_payload_t {
  uint64_t uid;
  uint8_t card_type;
  uint8_t rarity;
  uint16_t content_id;
  uint16_t value;
  uint32_t mock_signature;
  uint16_t flags;
} pet_nfc_card_payload_t;

typedef struct PET_NFC_PACKED_STRUCT pet_nfc_card_validation_result_t {
  uint8_t status;
  uint8_t card_type;
  uint16_t content_id;
  uint32_t expected_mock_signature;
} pet_nfc_card_validation_result_t;

#if defined(_MSC_VER)
#pragma pack(pop)
#elif defined(PET_NFC_PAYLOAD_PACK_POP)
#pragma pack(pop)
#undef PET_NFC_PAYLOAD_PACK_POP
#endif
#undef PET_NFC_PACKED_STRUCT

typedef pet_nfc_card_payload_t PetNfcCardPayload;
typedef pet_nfc_card_validation_result_t PetNfcCardValidationResult;

uint8_t pet_nfc_card_type_is_supported(uint8_t card_type);
uint32_t pet_nfc_card_payload_expected_mock_signature(const pet_nfc_card_payload_t* payload);
pet_result_t pet_nfc_card_payload_validate(
    const pet_nfc_card_payload_t* payload,
    pet_nfc_card_validation_result_t* out_result);
const char* pet_nfc_card_validation_status_name(uint8_t status);

#ifdef __cplusplus
}
#endif

#endif  /* PETEGG_PORTABLE_PET_NFC_PAYLOAD_H_ */
