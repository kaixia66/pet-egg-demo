#ifndef PETEGG_PORTABLE_PET_PACKET_H_
#define PETEGG_PORTABLE_PET_PACKET_H_

#include "pet_config.h"
#include "pet_result.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PET_PACKET_MAGIC 0xE6u
#define PET_PACKET_VERSION 1u
#define PET_PACKET_SERIALIZED_HEADER_SIZE 10u
#define PET_PACKET_CRC16_SIZE 2u
#define PET_PACKET_MAX_SERIALIZED_SIZE \
  (PET_PACKET_SERIALIZED_HEADER_SIZE + PET_PACKET_MAX_PAYLOAD + PET_PACKET_CRC16_SIZE)

typedef enum PetPacketType {
  PET_PACKET_NONE = 0,
  PET_PACKET_HELLO = 1,
  PET_PACKET_HELLO_ACK = 2,
  PET_PACKET_PING = 3,
  PET_PACKET_PONG = 4,
  PET_PACKET_BOSS_READY = 16,
  PET_PACKET_ACTION_SELECT = 17,
  PET_PACKET_QTE_RESULT = 18,
  PET_PACKET_RESULT_HASH = 19
} PetPacketType;

typedef enum pet_packet_status_t {
  PET_PACKET_STATUS_OK = 0,
  PET_PACKET_STATUS_NULL = 1,
  PET_PACKET_STATUS_BAD_MAGIC = 2,
  PET_PACKET_STATUS_BAD_VERSION = 3,
  PET_PACKET_STATUS_BAD_LEN = 4,
  PET_PACKET_STATUS_BAD_CRC = 5
} pet_packet_status_t;

/* BLE/BT small packet ABI. This is not an NFC content-card payload and does not carry
   animation frames. Multi-byte fields are serialized little-endian. */
typedef struct pet_packet_t {
  uint8_t magic;
  uint8_t version;
  uint8_t type;
  uint8_t flags;
  uint16_t seq;
  uint16_t ack;
  uint16_t len;
  uint8_t payload[PET_PACKET_MAX_PAYLOAD];
  uint16_t crc16;
} pet_packet_t;

typedef PetPacketType pet_packet_type_t;
typedef pet_packet_status_t PetPacketStatus;
typedef pet_packet_t PetPacket;

pet_result_t pet_packet_build(uint8_t type,
                              uint16_t seq,
                              uint16_t ack,
                              const uint8_t* payload,
                              uint16_t payload_len,
                              uint8_t flags,
                              pet_packet_t* out_packet);
pet_packet_status_t pet_packet_validate(const pet_packet_t* packet);
uint16_t pet_packet_crc16(const pet_packet_t* packet);
void pet_packet_finalize(pet_packet_t* packet);
const char* pet_packet_type_name(uint8_t type);
const char* pet_packet_status_name(uint8_t status);

#ifdef __cplusplus
}
#endif

#endif  /* PETEGG_PORTABLE_PET_PACKET_H_ */
