#ifndef PETEGG_PORTABLE_PET_NFC_PAIR_PAYLOAD_H_
#define PETEGG_PORTABLE_PET_NFC_PAIR_PAYLOAD_H_

#include "pet_result.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Device-to-device NFC pair payload ABI. This is separate from NFC content-card
   activation payloads and carries no pet, card, animation, config, or resource bytes. */
#define PET_NFC_PAIR_MAGIC 0x50414952u
#define PET_NFC_PAIR_PROTOCOL_VERSION 1u
#define PET_NFC_PAIR_RESOURCE_VERSION 1u
#define PET_NFC_PAIR_BLE_SERVICE_ID 1u
#define PET_NFC_PAIR_PAYLOAD_SERIALIZED_SIZE 24u
#define PET_NFC_PAIR_CRC_RANGE_SIZE 22u

typedef enum pet_nfc_pair_payload_status_t {
  PET_NFC_PAIR_PAYLOAD_OK = 0,
  PET_NFC_PAIR_PAYLOAD_INVALID_ARG = 1,
  PET_NFC_PAIR_PAYLOAD_BAD_VERSION = 2,
  PET_NFC_PAIR_PAYLOAD_BAD_CRC = 3
} pet_nfc_pair_payload_status_t;

typedef struct pet_nfc_pair_payload_t {
  uint16_t protocol_version;
  uint32_t device_id;
  uint16_t device_short_id;
  uint32_t nonce;
  uint32_t session_seed;
  uint16_t ble_service_id;
  uint16_t resource_version;
  uint16_t flags;
  uint16_t crc16;
} pet_nfc_pair_payload_t;

typedef pet_nfc_pair_payload_t PetNfcPairPayload;

pet_result_t pet_nfc_pair_payload_build(uint32_t device_id,
                                        uint16_t device_short_id,
                                        uint32_t nonce,
                                        uint32_t session_seed,
                                        uint16_t protocol_version,
                                        uint16_t service_id,
                                        uint16_t resource_version,
                                        uint16_t flags,
                                        pet_nfc_pair_payload_t* out_payload);
pet_result_t pet_nfc_pair_payload_validate(const pet_nfc_pair_payload_t* payload);
uint16_t pet_nfc_pair_payload_crc16(const pet_nfc_pair_payload_t* payload);
void pet_nfc_pair_payload_finalize(pet_nfc_pair_payload_t* payload);
pet_result_t pet_nfc_pair_payload_derive_session_seed(const pet_nfc_pair_payload_t* a,
                                                      const pet_nfc_pair_payload_t* b,
                                                      uint32_t* out_session_seed);
const char* pet_nfc_pair_payload_status_name(uint8_t status);

#ifdef __cplusplus
}
#endif

#endif  /* PETEGG_PORTABLE_PET_NFC_PAIR_PAYLOAD_H_ */
