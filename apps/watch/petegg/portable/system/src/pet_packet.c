#include "pet_packet.h"

#include <string.h>

static void pet_packet_write_u8(uint8_t* bytes, size_t* offset, uint8_t value) {
  bytes[*offset] = value;
  *offset += 1u;
}

static void pet_packet_write_u16(uint8_t* bytes, size_t* offset, uint16_t value) {
  pet_packet_write_u8(bytes, offset, (uint8_t)(value & 0xFFu));
  pet_packet_write_u8(bytes, offset, (uint8_t)((value >> 8u) & 0xFFu));
}

static uint16_t pet_packet_crc16_ccitt_false(const uint8_t* data, size_t len) {
  uint16_t crc = 0xFFFFu;
  size_t i;
  for (i = 0u; i < len; ++i) {
    int bit;
    crc ^= (uint16_t)data[i] << 8u;
    for (bit = 0; bit < 8; ++bit) {
      if ((crc & 0x8000u) != 0u) {
        crc = (uint16_t)((crc << 1u) ^ 0x1021u);
      } else {
        crc = (uint16_t)(crc << 1u);
      }
    }
  }
  return crc;
}

static uint8_t pet_packet_serialize_without_crc(const pet_packet_t* packet,
                                                uint8_t* bytes,
                                                size_t* out_len) {
  size_t offset = 0u;
  uint16_t i;
  if (packet == 0 || bytes == 0 || out_len == 0 || packet->len > PET_PACKET_MAX_PAYLOAD) {
    return 0u;
  }
  pet_packet_write_u8(bytes, &offset, packet->magic);
  pet_packet_write_u8(bytes, &offset, packet->version);
  pet_packet_write_u8(bytes, &offset, packet->type);
  pet_packet_write_u8(bytes, &offset, packet->flags);
  pet_packet_write_u16(bytes, &offset, packet->seq);
  pet_packet_write_u16(bytes, &offset, packet->ack);
  pet_packet_write_u16(bytes, &offset, packet->len);
  for (i = 0u; i < packet->len; ++i) {
    pet_packet_write_u8(bytes, &offset, packet->payload[i]);
  }
  *out_len = offset;
  return 1u;
}

uint16_t pet_packet_crc16(const pet_packet_t* packet) {
  uint8_t bytes[PET_PACKET_SERIALIZED_HEADER_SIZE + PET_PACKET_MAX_PAYLOAD];
  size_t len = 0u;
  if (!pet_packet_serialize_without_crc(packet, bytes, &len)) {
    return 0u;
  }
  return pet_packet_crc16_ccitt_false(bytes, len);
}

void pet_packet_finalize(pet_packet_t* packet) {
  if (packet != 0) {
    packet->crc16 = pet_packet_crc16(packet);
  }
}

pet_packet_status_t pet_packet_validate(const pet_packet_t* packet) {
  if (packet == 0) {
    return PET_PACKET_STATUS_NULL;
  }
  if (packet->magic != PET_PACKET_MAGIC) {
    return PET_PACKET_STATUS_BAD_MAGIC;
  }
  if (packet->version != PET_PACKET_VERSION) {
    return PET_PACKET_STATUS_BAD_VERSION;
  }
  if (packet->len > PET_PACKET_MAX_PAYLOAD) {
    return PET_PACKET_STATUS_BAD_LEN;
  }
  if (packet->crc16 != pet_packet_crc16(packet)) {
    return PET_PACKET_STATUS_BAD_CRC;
  }
  return PET_PACKET_STATUS_OK;
}

pet_result_t pet_packet_build(uint8_t type,
                              uint16_t seq,
                              uint16_t ack,
                              const uint8_t* payload,
                              uint16_t payload_len,
                              uint8_t flags,
                              pet_packet_t* out_packet) {
  pet_packet_t packet;
  if (out_packet == 0 || payload_len > PET_PACKET_MAX_PAYLOAD ||
      (payload_len > 0u && payload == 0)) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  memset(&packet, 0, sizeof(packet));
  packet.magic = PET_PACKET_MAGIC;
  packet.version = PET_PACKET_VERSION;
  packet.type = type;
  packet.flags = flags;
  packet.seq = seq;
  packet.ack = ack;
  packet.len = payload_len;
  if (payload_len > 0u) {
    memcpy(packet.payload, payload, payload_len);
  }
  pet_packet_finalize(&packet);
  *out_packet = packet;
  return PET_RESULT_OK;
}

const char* pet_packet_type_name(uint8_t type) {
  switch (type) {
    case PET_PACKET_NONE:
      return "none";
    case PET_PACKET_HELLO:
      return "hello";
    case PET_PACKET_HELLO_ACK:
      return "hello_ack";
    case PET_PACKET_PING:
      return "ping";
    case PET_PACKET_PONG:
      return "pong";
    case PET_PACKET_BOSS_READY:
      return "boss_ready";
    case PET_PACKET_ACTION_SELECT:
      return "action_select";
    case PET_PACKET_QTE_RESULT:
      return "qte_result";
    case PET_PACKET_RESULT_HASH:
      return "result_hash";
    default:
      return "unknown";
  }
}

const char* pet_packet_status_name(uint8_t status) {
  switch (status) {
    case PET_PACKET_STATUS_OK:
      return "ok";
    case PET_PACKET_STATUS_NULL:
      return "null";
    case PET_PACKET_STATUS_BAD_MAGIC:
      return "bad_magic";
    case PET_PACKET_STATUS_BAD_VERSION:
      return "bad_version";
    case PET_PACKET_STATUS_BAD_LEN:
      return "bad_len";
    case PET_PACKET_STATUS_BAD_CRC:
      return "bad_crc";
    default:
      return "unknown";
  }
}
