#ifndef PETEGG_PORTABLE_PET_CARD_ACTIVATION_H_
#define PETEGG_PORTABLE_PET_CARD_ACTIVATION_H_

#include "pet_nfc_payload.h"
#include "pet_result.h"
#include "pet_save_format.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Applies a validated NFC content-card payload to an in-memory device save payload.
   payload.content_id is interpreted as an unlock_id and resolved through the local
   unlock table; NFC cards do not carry resource bytes. This API does not read NFC
   hardware, read resources, access files or Flash, commit save slots, or call
   platform services. Failed activation leaves save unchanged. */
typedef enum pet_card_activation_status_t {
  PET_CARD_ACTIVATION_OK = 0,
  PET_CARD_ACTIVATION_INVALID_ARG = 1,
  PET_CARD_ACTIVATION_INVALID_PAYLOAD = 2,
  PET_CARD_ACTIVATION_INVALID_SIGNATURE = 3,
  PET_CARD_ACTIVATION_DUPLICATE = 4,
  PET_CARD_ACTIVATION_PET_POOL_FULL = 5,
  PET_CARD_ACTIVATION_CARD_BAG_FULL = 6,
  PET_CARD_ACTIVATION_EQUIPMENT_FULL = 7,
  PET_CARD_ACTIVATION_HOME_ASSET_FULL = 8,
  PET_CARD_ACTIVATION_UNSUPPORTED_CARD_TYPE = 9,
  PET_CARD_ACTIVATION_SAVE_CAPACITY_ERROR = 10
} pet_card_activation_status_t;

typedef struct pet_card_activation_result_t {
  uint8_t status;
  uint8_t card_type;
  uint16_t content_id;
  uint32_t created_id;
  uint64_t card_uid;
} pet_card_activation_result_t;

pet_result_t pet_card_activation_check_duplicate(const pet_device_save_payload_t* save,
                                                 uint64_t card_uid,
                                                 uint8_t* out_is_duplicate);
pet_result_t pet_card_activation_apply(pet_device_save_payload_t* save,
                                       const pet_nfc_card_payload_t* payload,
                                       pet_card_activation_result_t* out_result);
const char* pet_card_activation_status_name(uint8_t status);

#ifdef __cplusplus
}
#endif

#endif  /* PETEGG_PORTABLE_PET_CARD_ACTIVATION_H_ */
