#ifndef PETEGG_PORTABLE_PET_BT_SESSION_H_
#define PETEGG_PORTABLE_PET_BT_SESSION_H_

#include "pet_packet.h"
#include "pet_result.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PET_BT_PROTOCOL_VERSION 1u
#define PET_BT_RESOURCE_VERSION 1u
#define PET_BT_HELLO_PAYLOAD_LEN 10u
#define PET_BT_HELLO_ACK_PAYLOAD_LEN 7u

typedef enum pet_bt_state_t {
  PET_BT_IDLE = 0,
  PET_BT_READY = 1,
  PET_BT_CONNECTING = 2,
  PET_BT_CONNECTED = 3,
  PET_BT_DISCONNECTED = 4,
  PET_BT_ERROR = 5,
  PET_BT_SYNCING = 16,
  PET_BT_IN_GAME = 17,
  PET_BT_RECONNECTING = 18
} pet_bt_state_t;

typedef enum pet_bt_status_t {
  PET_BT_STATUS_NONE = 0,
  PET_BT_OK = 1,
  PET_BT_FAIL_NOT_PAIRED = 2,
  PET_BT_FAIL_SESSION_MISMATCH = 3,
  PET_BT_FAIL_PACKET_CRC = 4,
  PET_BT_FAIL_PACKET_INVALID = 5,
  PET_BT_FAIL_UNEXPECTED_PACKET = 6,
  PET_BT_FAIL_BAD_ACK = 7
} pet_bt_status_t;

typedef struct pet_bt_session_t {
  uint16_t device_short_id;
  uint16_t peer_device_short_id;
  uint32_t session_seed;
  uint16_t protocol_version;
  uint16_t resource_version;
  uint8_t state;
  uint8_t last_status;
  uint8_t last_packet_type;
  uint16_t seq;
  uint16_t ack;
} pet_bt_session_t;

typedef pet_bt_session_t PetBtSession;

void pet_bt_session_init(pet_bt_session_t* session, uint16_t device_short_id);
void pet_bt_session_reset(pet_bt_session_t* session);
void pet_bt_session_set_ready(pet_bt_session_t* session,
                              uint16_t peer_device_short_id,
                              uint32_t session_seed);
void pet_bt_session_set_failure(pet_bt_session_t* session, uint8_t status);
uint8_t pet_bt_session_make_hello(pet_bt_session_t* session, pet_packet_t* out_packet);
uint8_t pet_bt_session_handle_packet(pet_bt_session_t* session,
                                     const pet_packet_t* packet,
                                     pet_packet_t* out_response);
const char* pet_bt_state_name(uint8_t state);
const char* pet_bt_status_name(uint8_t status);

#ifdef __cplusplus
}
#endif

#endif  /* PETEGG_PORTABLE_PET_BT_SESSION_H_ */
