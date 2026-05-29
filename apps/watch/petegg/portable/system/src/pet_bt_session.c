#include "pet_bt_session.h"

#include <string.h>

static void pet_bt_write_u8(uint8_t* bytes, size_t* offset, uint8_t value) {
  bytes[*offset] = value;
  *offset += 1u;
}

static void pet_bt_write_u16(uint8_t* bytes, size_t* offset, uint16_t value) {
  pet_bt_write_u8(bytes, offset, (uint8_t)(value & 0xFFu));
  pet_bt_write_u8(bytes, offset, (uint8_t)((value >> 8u) & 0xFFu));
}

static void pet_bt_write_u32(uint8_t* bytes, size_t* offset, uint32_t value) {
  pet_bt_write_u8(bytes, offset, (uint8_t)(value & 0xFFu));
  pet_bt_write_u8(bytes, offset, (uint8_t)((value >> 8u) & 0xFFu));
  pet_bt_write_u8(bytes, offset, (uint8_t)((value >> 16u) & 0xFFu));
  pet_bt_write_u8(bytes, offset, (uint8_t)((value >> 24u) & 0xFFu));
}

static uint16_t pet_bt_read_u16(const uint8_t* bytes, size_t* offset) {
  uint16_t value = (uint16_t)bytes[*offset] | ((uint16_t)bytes[*offset + 1u] << 8u);
  *offset += 2u;
  return value;
}

static uint32_t pet_bt_read_u32(const uint8_t* bytes, size_t* offset) {
  uint32_t value = (uint32_t)bytes[*offset] | ((uint32_t)bytes[*offset + 1u] << 8u) |
                   ((uint32_t)bytes[*offset + 2u] << 16u) |
                   ((uint32_t)bytes[*offset + 3u] << 24u);
  *offset += 4u;
  return value;
}

static void pet_bt_make_hello_payload(const pet_bt_session_t* session, uint8_t* out_payload) {
  size_t offset = 0u;
  memset(out_payload, 0, PET_PACKET_MAX_PAYLOAD);
  pet_bt_write_u16(out_payload, &offset, session->device_short_id);
  pet_bt_write_u32(out_payload, &offset, session->session_seed);
  pet_bt_write_u16(out_payload, &offset, session->protocol_version);
  pet_bt_write_u16(out_payload, &offset, session->resource_version);
}

static void pet_bt_make_hello_ack_payload(const pet_bt_session_t* session,
                                          uint8_t ack_status,
                                          uint8_t* out_payload) {
  size_t offset = 0u;
  memset(out_payload, 0, PET_PACKET_MAX_PAYLOAD);
  pet_bt_write_u16(out_payload, &offset, session->device_short_id);
  pet_bt_write_u32(out_payload, &offset, session->session_seed);
  pet_bt_write_u8(out_payload, &offset, ack_status);
}

static uint8_t pet_bt_packet_status_to_bt_status(uint8_t status) {
  if (status == PET_PACKET_STATUS_BAD_CRC) {
    return PET_BT_FAIL_PACKET_CRC;
  }
  if (status == PET_PACKET_STATUS_OK) {
    return PET_BT_OK;
  }
  return PET_BT_FAIL_PACKET_INVALID;
}

void pet_bt_session_init(pet_bt_session_t* session, uint16_t device_short_id) {
  if (session == 0) {
    return;
  }
  memset(session, 0, sizeof(*session));
  session->device_short_id = device_short_id;
  session->protocol_version = PET_BT_PROTOCOL_VERSION;
  session->resource_version = PET_BT_RESOURCE_VERSION;
  session->state = PET_BT_IDLE;
  session->last_status = PET_BT_STATUS_NONE;
  session->last_packet_type = PET_PACKET_NONE;
}

void pet_bt_session_reset(pet_bt_session_t* session) {
  uint16_t device_short_id;
  if (session == 0) {
    return;
  }
  device_short_id = session->device_short_id;
  pet_bt_session_init(session, device_short_id);
}

void pet_bt_session_set_ready(pet_bt_session_t* session,
                              uint16_t peer_device_short_id,
                              uint32_t session_seed) {
  if (session == 0) {
    return;
  }
  session->peer_device_short_id = peer_device_short_id;
  session->session_seed = session_seed;
  session->state = PET_BT_READY;
  session->last_status = PET_BT_STATUS_NONE;
  session->last_packet_type = PET_PACKET_NONE;
  session->seq = 0u;
  session->ack = 0u;
}

void pet_bt_session_set_failure(pet_bt_session_t* session, uint8_t status) {
  if (session != 0) {
    session->state = PET_BT_ERROR;
    session->last_status = status;
  }
}

uint8_t pet_bt_session_make_hello(pet_bt_session_t* session, pet_packet_t* out_packet) {
  uint8_t payload[PET_PACKET_MAX_PAYLOAD];
  if (session == 0 || out_packet == 0) {
    return PET_BT_FAIL_PACKET_INVALID;
  }
  if (session->state != PET_BT_READY && session->state != PET_BT_CONNECTING) {
    pet_bt_session_set_failure(session, PET_BT_FAIL_NOT_PAIRED);
    return PET_BT_FAIL_NOT_PAIRED;
  }
  if (session->session_seed == 0u || session->peer_device_short_id == 0u) {
    pet_bt_session_set_failure(session, PET_BT_FAIL_NOT_PAIRED);
    return PET_BT_FAIL_NOT_PAIRED;
  }

  session->state = PET_BT_CONNECTING;
  session->seq = (uint16_t)(session->seq + 1u);
  session->ack = 0u;
  pet_bt_make_hello_payload(session, payload);
  if (pet_packet_build(PET_PACKET_HELLO, session->seq, session->ack, payload,
                       PET_BT_HELLO_PAYLOAD_LEN, 0u, out_packet) != PET_RESULT_OK) {
    pet_bt_session_set_failure(session, PET_BT_FAIL_PACKET_INVALID);
    return PET_BT_FAIL_PACKET_INVALID;
  }

  session->last_packet_type = PET_PACKET_HELLO;
  session->last_status = PET_BT_STATUS_NONE;
  return PET_BT_OK;
}

static uint8_t pet_bt_handle_hello(pet_bt_session_t* session,
                                   const pet_packet_t* packet,
                                   pet_packet_t* out_response) {
  uint8_t payload[PET_PACKET_MAX_PAYLOAD];
  uint16_t peer_short_id;
  uint32_t session_seed;
  uint16_t protocol_version;
  size_t offset = 0u;
  if (packet->len != PET_BT_HELLO_PAYLOAD_LEN) {
    pet_bt_session_set_failure(session, PET_BT_FAIL_PACKET_INVALID);
    return PET_BT_FAIL_PACKET_INVALID;
  }
  peer_short_id = pet_bt_read_u16(packet->payload, &offset);
  session_seed = pet_bt_read_u32(packet->payload, &offset);
  protocol_version = pet_bt_read_u16(packet->payload, &offset);
  (void)pet_bt_read_u16(packet->payload, &offset);

  session->last_packet_type = packet->type;
  session->ack = packet->seq;
  if (session->session_seed == 0u || session_seed != session->session_seed) {
    pet_bt_session_set_failure(session, PET_BT_FAIL_SESSION_MISMATCH);
    return PET_BT_FAIL_SESSION_MISMATCH;
  }
  if (session->peer_device_short_id != 0u && peer_short_id != session->peer_device_short_id) {
    pet_bt_session_set_failure(session, PET_BT_FAIL_SESSION_MISMATCH);
    return PET_BT_FAIL_SESSION_MISMATCH;
  }
  if (protocol_version != session->protocol_version) {
    pet_bt_session_set_failure(session, PET_BT_FAIL_PACKET_INVALID);
    return PET_BT_FAIL_PACKET_INVALID;
  }

  session->peer_device_short_id = peer_short_id;
  session->state = PET_BT_CONNECTED;
  session->last_status = PET_BT_OK;

  if (out_response != 0) {
    session->seq = (uint16_t)(session->seq + 1u);
    pet_bt_make_hello_ack_payload(session, PET_BT_OK, payload);
    if (pet_packet_build(PET_PACKET_HELLO_ACK, session->seq, session->ack, payload,
                         PET_BT_HELLO_ACK_PAYLOAD_LEN, 0u, out_response) != PET_RESULT_OK) {
      pet_bt_session_set_failure(session, PET_BT_FAIL_PACKET_INVALID);
      return PET_BT_FAIL_PACKET_INVALID;
    }
    session->last_packet_type = PET_PACKET_HELLO_ACK;
  }

  return PET_BT_OK;
}

static uint8_t pet_bt_handle_hello_ack(pet_bt_session_t* session, const pet_packet_t* packet) {
  uint16_t peer_short_id;
  uint32_t session_seed;
  uint8_t ack_status;
  size_t offset = 0u;
  if (packet->len != PET_BT_HELLO_ACK_PAYLOAD_LEN) {
    pet_bt_session_set_failure(session, PET_BT_FAIL_PACKET_INVALID);
    return PET_BT_FAIL_PACKET_INVALID;
  }

  peer_short_id = pet_bt_read_u16(packet->payload, &offset);
  session_seed = pet_bt_read_u32(packet->payload, &offset);
  ack_status = packet->payload[offset];

  session->last_packet_type = packet->type;
  session->ack = packet->seq;
  if (packet->ack != session->seq) {
    pet_bt_session_set_failure(session, PET_BT_FAIL_BAD_ACK);
    return PET_BT_FAIL_BAD_ACK;
  }
  if (ack_status != PET_BT_OK) {
    pet_bt_session_set_failure(session, ack_status);
    return ack_status;
  }
  if (session->session_seed == 0u || session_seed != session->session_seed) {
    pet_bt_session_set_failure(session, PET_BT_FAIL_SESSION_MISMATCH);
    return PET_BT_FAIL_SESSION_MISMATCH;
  }
  if (session->peer_device_short_id != 0u && peer_short_id != session->peer_device_short_id) {
    pet_bt_session_set_failure(session, PET_BT_FAIL_SESSION_MISMATCH);
    return PET_BT_FAIL_SESSION_MISMATCH;
  }

  session->peer_device_short_id = peer_short_id;
  session->state = PET_BT_CONNECTED;
  session->last_status = PET_BT_OK;
  return PET_BT_OK;
}

uint8_t pet_bt_session_handle_packet(pet_bt_session_t* session,
                                     const pet_packet_t* packet,
                                     pet_packet_t* out_response) {
  uint8_t packet_status;
  uint8_t bt_status;
  if (session == 0 || packet == 0) {
    return PET_BT_FAIL_PACKET_INVALID;
  }
  if (out_response != 0) {
    memset(out_response, 0, sizeof(*out_response));
  }

  packet_status = pet_packet_validate(packet);
  if (packet_status != PET_PACKET_STATUS_OK) {
    bt_status = pet_bt_packet_status_to_bt_status(packet_status);
    pet_bt_session_set_failure(session, bt_status);
    return bt_status;
  }

  switch (packet->type) {
    case PET_PACKET_HELLO:
      return pet_bt_handle_hello(session, packet, out_response);
    case PET_PACKET_HELLO_ACK:
      return pet_bt_handle_hello_ack(session, packet);
    default:
      pet_bt_session_set_failure(session, PET_BT_FAIL_UNEXPECTED_PACKET);
      session->last_packet_type = packet->type;
      return PET_BT_FAIL_UNEXPECTED_PACKET;
  }
}

const char* pet_bt_state_name(uint8_t state) {
  switch (state) {
    case PET_BT_IDLE:
      return "idle";
    case PET_BT_READY:
      return "ready";
    case PET_BT_CONNECTING:
      return "connecting";
    case PET_BT_CONNECTED:
      return "connected";
    case PET_BT_DISCONNECTED:
      return "disconnected";
    case PET_BT_ERROR:
      return "error";
    case PET_BT_SYNCING:
      return "syncing";
    case PET_BT_IN_GAME:
      return "in_game";
    case PET_BT_RECONNECTING:
      return "reconnecting";
    default:
      return "unknown";
  }
}

const char* pet_bt_status_name(uint8_t status) {
  switch (status) {
    case PET_BT_STATUS_NONE:
      return "none";
    case PET_BT_OK:
      return "ok";
    case PET_BT_FAIL_NOT_PAIRED:
      return "fail_not_paired";
    case PET_BT_FAIL_SESSION_MISMATCH:
      return "fail_session_mismatch";
    case PET_BT_FAIL_PACKET_CRC:
      return "fail_packet_crc";
    case PET_BT_FAIL_PACKET_INVALID:
      return "fail_packet_invalid";
    case PET_BT_FAIL_UNEXPECTED_PACKET:
      return "fail_unexpected_packet";
    case PET_BT_FAIL_BAD_ACK:
      return "fail_bad_ack";
    default:
      return "unknown";
  }
}
