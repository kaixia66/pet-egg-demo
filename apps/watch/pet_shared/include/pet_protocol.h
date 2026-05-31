#ifndef PET_PROTOCOL_H
#define PET_PROTOCOL_H

#include "pet_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PET_PROTOCOL_VERSION_MAJOR 1u
#define PET_PROTOCOL_VERSION_MINOR 0u
#define PET_PROTOCOL_VERSION       PET_VERSION_MAKE(PET_PROTOCOL_VERSION_MAJOR, PET_PROTOCOL_VERSION_MINOR, 0u)
#define PET_PACKET_MAGIC           0xE6u
#define PET_PACKET_VERSION         1u
#define PET_PACKET_SERIALIZED_HEADER_SIZE 10u
#define PET_PACKET_CRC16_SIZE      2u
#define PET_PACKET_MAX_SERIALIZED_SIZE \
    (PET_PACKET_SERIALIZED_HEADER_SIZE + PET_PACKET_MAX_PAYLOAD + PET_PACKET_CRC16_SIZE)
#define PET_PROTOCOL_MAGIC         PET_PACKET_MAGIC
#define PET_PROTOCOL_MAX_PAYLOAD   PET_PACKET_MAX_PAYLOAD
#define PET_PROTOCOL_NFC_UID_MAX   10u
#define PET_PROTOCOL_DEVICE_ID_LEN 16u
#define PET_PROTOCOL_NONCE_LEN     16u

#define PET_NFC_PAIR_MAGIC         0x50414952u
#define PET_NFC_PAIR_PROTOCOL_VERSION 1u
#define PET_NFC_PAIR_RESOURCE_VERSION 1u
#define PET_NFC_PAIR_BLE_SERVICE_ID 1u
#define PET_NFC_PAIR_PAYLOAD_SERIALIZED_SIZE 24u
#define PET_NFC_PAIR_CRC_RANGE_SIZE 22u

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

typedef enum {
    PET_PACKET_FLAG_NONE = 0,
    PET_PACKET_FLAG_REQUIRES_ACK = 1u << 0,
    PET_PACKET_FLAG_IS_ACK = 1u << 1,
    PET_PACKET_FLAG_RETRANSMIT = 1u << 2
} pet_packet_flags_t;

typedef enum pet_packet_status_t {
    PET_PACKET_STATUS_OK = 0,
    PET_PACKET_STATUS_NULL = 1,
    PET_PACKET_STATUS_BAD_MAGIC = 2,
    PET_PACKET_STATUS_BAD_VERSION = 3,
    PET_PACKET_STATUS_BAD_LEN = 4,
    PET_PACKET_STATUS_BAD_CRC = 5
} pet_packet_status_t;

PET_PACKED_BEGIN
typedef struct PET_PACKED pet_packet_t {
    pet_u8_t magic;
    pet_u8_t version;
    pet_u8_t type;
    pet_u8_t flags;
    pet_u16_t seq;
    pet_u16_t ack;
    pet_u16_t len;
    pet_u8_t payload[PET_PACKET_MAX_PAYLOAD];
    pet_u16_t crc16;
} pet_packet_t;
PET_PACKED_END

PET_PACKED_BEGIN
typedef struct PET_PACKED pet_nfc_pair_payload_t {
    pet_u16_t protocol_version;
    pet_u32_t device_id;
    pet_u16_t device_short_id;
    pet_u32_t nonce;
    pet_u32_t session_seed;
    pet_u16_t ble_service_id;
    pet_u16_t resource_version;
    pet_u16_t flags;
    pet_u16_t crc16;
} pet_nfc_pair_payload_t;
PET_PACKED_END

typedef PetPacketType pet_packet_type_t;
typedef pet_packet_status_t PetPacketStatus;
typedef pet_packet_t PetPacket;
typedef pet_nfc_pair_payload_t PetNfcPairPayload;

PET_STATIC_ASSERT(packet_header_prefix_size,
                  offsetof(pet_packet_t, payload) == PET_PACKET_SERIALIZED_HEADER_SIZE);
PET_STATIC_ASSERT(packet_size,
                  sizeof(pet_packet_t) == PET_PACKET_MAX_SERIALIZED_SIZE);
PET_STATIC_ASSERT(nfc_pair_payload_size,
                  sizeof(pet_nfc_pair_payload_t) == PET_NFC_PAIR_PAYLOAD_SERIALIZED_SIZE);

#ifdef __cplusplus
}
#endif

#endif
