#include "pet_nfc_pair_payload.h"

#include <string.h>

static void pet_pair_write_u8(uint8_t* bytes, size_t* offset, uint8_t value) {
  bytes[*offset] = value;
  *offset += 1u;
}

static void pet_pair_write_u16(uint8_t* bytes, size_t* offset, uint16_t value) {
  pet_pair_write_u8(bytes, offset, (uint8_t)(value & 0xFFu));
  pet_pair_write_u8(bytes, offset, (uint8_t)((value >> 8u) & 0xFFu));
}

static void pet_pair_write_u32(uint8_t* bytes, size_t* offset, uint32_t value) {
  pet_pair_write_u8(bytes, offset, (uint8_t)(value & 0xFFu));
  pet_pair_write_u8(bytes, offset, (uint8_t)((value >> 8u) & 0xFFu));
  pet_pair_write_u8(bytes, offset, (uint8_t)((value >> 16u) & 0xFFu));
  pet_pair_write_u8(bytes, offset, (uint8_t)((value >> 24u) & 0xFFu));
}

static uint16_t pet_pair_crc16_ccitt_false(const uint8_t* data, size_t len) {
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

static void pet_pair_serialize_without_crc(const pet_nfc_pair_payload_t* payload,
                                           uint8_t bytes[PET_NFC_PAIR_CRC_RANGE_SIZE]) {
  size_t offset = 0u;
  pet_pair_write_u16(bytes, &offset, payload->protocol_version);
  pet_pair_write_u32(bytes, &offset, payload->device_id);
  pet_pair_write_u16(bytes, &offset, payload->device_short_id);
  pet_pair_write_u32(bytes, &offset, payload->nonce);
  pet_pair_write_u32(bytes, &offset, payload->session_seed);
  pet_pair_write_u16(bytes, &offset, payload->ble_service_id);
  pet_pair_write_u16(bytes, &offset, payload->resource_version);
  pet_pair_write_u16(bytes, &offset, payload->flags);
}

static uint32_t pet_pair_mix32(uint32_t value) {
  value ^= value >> 16u;
  value *= 0x7feb352du;
  value ^= value >> 15u;
  value *= 0x846ca68bu;
  value ^= value >> 16u;
  return value;
}

pet_result_t pet_nfc_pair_payload_build(uint32_t device_id,
                                        uint16_t device_short_id,
                                        uint32_t nonce,
                                        uint32_t session_seed,
                                        uint16_t protocol_version,
                                        uint16_t service_id,
                                        uint16_t resource_version,
                                        uint16_t flags,
                                        pet_nfc_pair_payload_t* out_payload) {
  if (out_payload == 0) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  memset(out_payload, 0, sizeof(*out_payload));
  out_payload->protocol_version = protocol_version;
  out_payload->device_id = device_id;
  out_payload->device_short_id = device_short_id;
  out_payload->nonce = nonce;
  out_payload->session_seed = session_seed;
  out_payload->ble_service_id = service_id;
  out_payload->resource_version = resource_version;
  out_payload->flags = flags;
  pet_nfc_pair_payload_finalize(out_payload);
  return PET_RESULT_OK;
}

uint16_t pet_nfc_pair_payload_crc16(const pet_nfc_pair_payload_t* payload) {
  uint8_t bytes[PET_NFC_PAIR_CRC_RANGE_SIZE];
  if (payload == 0) {
    return 0u;
  }
  pet_pair_serialize_without_crc(payload, bytes);
  return pet_pair_crc16_ccitt_false(bytes, sizeof(bytes));
}

void pet_nfc_pair_payload_finalize(pet_nfc_pair_payload_t* payload) {
  if (payload != 0) {
    payload->crc16 = pet_nfc_pair_payload_crc16(payload);
  }
}

pet_result_t pet_nfc_pair_payload_validate(const pet_nfc_pair_payload_t* payload) {
  if (payload == 0) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  if (payload->protocol_version != PET_NFC_PAIR_PROTOCOL_VERSION) {
    return PET_RESULT_BAD_VERSION;
  }
  if (payload->crc16 != pet_nfc_pair_payload_crc16(payload)) {
    return PET_RESULT_BAD_CRC;
  }
  return PET_RESULT_OK;
}

pet_result_t pet_nfc_pair_payload_derive_session_seed(const pet_nfc_pair_payload_t* a,
                                                      const pet_nfc_pair_payload_t* b,
                                                      uint32_t* out_session_seed) {
  const pet_nfc_pair_payload_t* first;
  const pet_nfc_pair_payload_t* second;
  uint32_t seed;
  if (a == 0 || b == 0 || out_session_seed == 0) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  if (pet_nfc_pair_payload_validate(a) != PET_RESULT_OK ||
      pet_nfc_pair_payload_validate(b) != PET_RESULT_OK) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  first = a->device_id <= b->device_id ? a : b;
  second = a->device_id <= b->device_id ? b : a;

  seed = PET_NFC_PAIR_MAGIC;
  seed ^= pet_pair_mix32(first->device_id);
  seed ^= pet_pair_mix32(first->nonce + 0x9e3779b9u);
  seed ^= pet_pair_mix32(second->device_id + 0x85ebca6bu);
  seed ^= pet_pair_mix32(second->nonce + 0xc2b2ae35u);
  *out_session_seed = pet_pair_mix32(seed);
  return PET_RESULT_OK;
}

const char* pet_nfc_pair_payload_status_name(uint8_t status) {
  switch (status) {
    case PET_NFC_PAIR_PAYLOAD_OK:
      return "ok";
    case PET_NFC_PAIR_PAYLOAD_INVALID_ARG:
      return "invalid_arg";
    case PET_NFC_PAIR_PAYLOAD_BAD_VERSION:
      return "bad_version";
    case PET_NFC_PAIR_PAYLOAD_BAD_CRC:
      return "bad_crc";
    default:
      return "unknown";
  }
}
