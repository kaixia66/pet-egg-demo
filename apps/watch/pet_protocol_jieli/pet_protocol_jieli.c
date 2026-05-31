#include "pet_protocol_jieli.h"

static void pet_protocol_jieli_zero_bytes(pet_u8_t *bytes, pet_size_t len)
{
    pet_size_t i;

    for (i = 0u; i < len; ++i) {
        bytes[i] = 0u;
    }
}

static void pet_protocol_jieli_copy_bytes(pet_u8_t *dst, const pet_u8_t *src, pet_size_t len)
{
    pet_size_t i;

    for (i = 0u; i < len; ++i) {
        dst[i] = src[i];
    }
}

static void pet_protocol_jieli_write_u8(pet_u8_t *bytes, pet_size_t *offset, pet_u8_t value)
{
    bytes[*offset] = value;
    *offset += 1u;
}

static void pet_protocol_jieli_write_u16(pet_u8_t *bytes, pet_size_t *offset, pet_u16_t value)
{
    pet_protocol_jieli_write_u8(bytes, offset, (pet_u8_t)(value & 0xffu));
    pet_protocol_jieli_write_u8(bytes, offset, (pet_u8_t)((value >> 8u) & 0xffu));
}

static void pet_protocol_jieli_write_u32(pet_u8_t *bytes, pet_size_t *offset, pet_u32_t value)
{
    pet_protocol_jieli_write_u8(bytes, offset, (pet_u8_t)(value & 0xffu));
    pet_protocol_jieli_write_u8(bytes, offset, (pet_u8_t)((value >> 8u) & 0xffu));
    pet_protocol_jieli_write_u8(bytes, offset, (pet_u8_t)((value >> 16u) & 0xffu));
    pet_protocol_jieli_write_u8(bytes, offset, (pet_u8_t)((value >> 24u) & 0xffu));
}

static pet_bool_t pet_protocol_jieli_serialize_packet_without_crc(const pet_packet_t *packet,
                                                                  pet_u8_t *bytes,
                                                                  pet_size_t *out_len)
{
    pet_size_t offset = 0u;
    pet_u16_t i;

    if ((packet == 0) || (bytes == 0) || (out_len == 0) ||
        (packet->len > PET_PACKET_MAX_PAYLOAD)) {
        return PET_FALSE;
    }

    pet_protocol_jieli_write_u8(bytes, &offset, packet->magic);
    pet_protocol_jieli_write_u8(bytes, &offset, packet->version);
    pet_protocol_jieli_write_u8(bytes, &offset, packet->type);
    pet_protocol_jieli_write_u8(bytes, &offset, packet->flags);
    pet_protocol_jieli_write_u16(bytes, &offset, packet->seq);
    pet_protocol_jieli_write_u16(bytes, &offset, packet->ack);
    pet_protocol_jieli_write_u16(bytes, &offset, packet->len);
    for (i = 0u; i < packet->len; ++i) {
        pet_protocol_jieli_write_u8(bytes, &offset, packet->payload[i]);
    }
    *out_len = offset;
    return PET_TRUE;
}

static void pet_protocol_jieli_serialize_pair_without_crc(
    const pet_nfc_pair_payload_t *payload,
    pet_u8_t bytes[PET_NFC_PAIR_CRC_RANGE_SIZE])
{
    pet_size_t offset = 0u;

    pet_protocol_jieli_write_u16(bytes, &offset, payload->protocol_version);
    pet_protocol_jieli_write_u32(bytes, &offset, payload->device_id);
    pet_protocol_jieli_write_u16(bytes, &offset, payload->device_short_id);
    pet_protocol_jieli_write_u32(bytes, &offset, payload->nonce);
    pet_protocol_jieli_write_u32(bytes, &offset, payload->session_seed);
    pet_protocol_jieli_write_u16(bytes, &offset, payload->ble_service_id);
    pet_protocol_jieli_write_u16(bytes, &offset, payload->resource_version);
    pet_protocol_jieli_write_u16(bytes, &offset, payload->flags);
}

pet_u16_t pet_protocol_jieli_crc16_ccitt_false(const pet_u8_t *data, pet_size_t len)
{
    pet_u16_t crc = 0xffffu;
    pet_size_t i;
    pet_u8_t bit;

    if ((data == 0) && (len != 0u)) {
        return 0u;
    }

    for (i = 0u; i < len; ++i) {
        crc ^= (pet_u16_t)data[i] << 8u;
        for (bit = 0u; bit < 8u; ++bit) {
            if ((crc & 0x8000u) != 0u) {
                crc = (pet_u16_t)((crc << 1u) ^ 0x1021u);
            } else {
                crc = (pet_u16_t)(crc << 1u);
            }
        }
    }

    return crc;
}

pet_u16_t pet_protocol_jieli_packet_crc16(const pet_packet_t *packet)
{
    pet_u8_t bytes[PET_PACKET_SERIALIZED_HEADER_SIZE + PET_PACKET_MAX_PAYLOAD];
    pet_size_t len = 0u;

    if (pet_protocol_jieli_serialize_packet_without_crc(packet, bytes, &len) != PET_TRUE) {
        return 0u;
    }

    return pet_protocol_jieli_crc16_ccitt_false(bytes, len);
}

void pet_protocol_jieli_packet_finalize(pet_packet_t *packet)
{
    if (packet != 0) {
        packet->crc16 = pet_protocol_jieli_packet_crc16(packet);
    }
}

pet_result_t pet_protocol_jieli_build_packet(pet_u8_t type,
                                             pet_u16_t seq,
                                             pet_u16_t ack,
                                             const pet_u8_t *payload,
                                             pet_u16_t payload_len,
                                             pet_u8_t flags,
                                             pet_packet_t *out_packet)
{
    pet_packet_t packet;

    if ((out_packet == 0) || (payload_len > PET_PACKET_MAX_PAYLOAD) ||
        ((payload == 0) && (payload_len != 0u))) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    pet_protocol_jieli_zero_bytes((pet_u8_t *)&packet, sizeof(packet));
    packet.magic = PET_PACKET_MAGIC;
    packet.version = PET_PACKET_VERSION;
    packet.type = type;
    packet.flags = flags;
    packet.seq = seq;
    packet.ack = ack;
    packet.len = payload_len;
    if (payload_len != 0u) {
        pet_protocol_jieli_copy_bytes(packet.payload, payload, payload_len);
    }
    pet_protocol_jieli_packet_finalize(&packet);
    *out_packet = packet;
    return PET_RESULT_OK;
}

pet_packet_status_t pet_protocol_jieli_validate_packet(const pet_packet_t *packet)
{
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
    if (packet->crc16 != pet_protocol_jieli_packet_crc16(packet)) {
        return PET_PACKET_STATUS_BAD_CRC;
    }
    return PET_PACKET_STATUS_OK;
}

pet_u16_t pet_protocol_jieli_nfc_pair_crc16(const pet_nfc_pair_payload_t *payload)
{
    pet_u8_t bytes[PET_NFC_PAIR_CRC_RANGE_SIZE];

    if (payload == 0) {
        return 0u;
    }

    pet_protocol_jieli_serialize_pair_without_crc(payload, bytes);
    return pet_protocol_jieli_crc16_ccitt_false(bytes, sizeof(bytes));
}

void pet_protocol_jieli_nfc_pair_finalize(pet_nfc_pair_payload_t *payload)
{
    if (payload != 0) {
        payload->crc16 = pet_protocol_jieli_nfc_pair_crc16(payload);
    }
}

pet_result_t pet_protocol_jieli_build_nfc_pair_payload(pet_u32_t device_id,
                                                       pet_u16_t device_short_id,
                                                       pet_u32_t nonce,
                                                       pet_u32_t session_seed,
                                                       pet_u16_t protocol_version,
                                                       pet_u16_t service_id,
                                                       pet_u16_t resource_version,
                                                       pet_u16_t flags,
                                                       pet_nfc_pair_payload_t *out_payload)
{
    if (out_payload == 0) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    pet_protocol_jieli_zero_bytes((pet_u8_t *)out_payload, sizeof(*out_payload));
    out_payload->protocol_version = protocol_version;
    out_payload->device_id = device_id;
    out_payload->device_short_id = device_short_id;
    out_payload->nonce = nonce;
    out_payload->session_seed = session_seed;
    out_payload->ble_service_id = service_id;
    out_payload->resource_version = resource_version;
    out_payload->flags = flags;
    pet_protocol_jieli_nfc_pair_finalize(out_payload);
    return PET_RESULT_OK;
}

pet_result_t pet_protocol_jieli_validate_nfc_pair_payload(const pet_nfc_pair_payload_t *payload)
{
    if (payload == 0) {
        return PET_RESULT_INVALID_ARGUMENT;
    }
    if (payload->protocol_version != PET_NFC_PAIR_PROTOCOL_VERSION) {
        return PET_RESULT_BAD_VERSION;
    }
    if (payload->crc16 != pet_protocol_jieli_nfc_pair_crc16(payload)) {
        return PET_RESULT_BAD_CRC;
    }
    return PET_RESULT_OK;
}

pet_result_t pet_protocol_jieli_self_test(void)
{
    pet_u8_t payload[3] = {0x11u, 0x22u, 0x33u};
    pet_packet_t packet;
    pet_nfc_pair_payload_t pair;
    pet_result_t ret;

    ret = pet_protocol_jieli_build_packet(PET_PACKET_PING, 7u, 3u, payload,
                                          sizeof(payload), PET_PACKET_FLAG_REQUIRES_ACK,
                                          &packet);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    if (pet_protocol_jieli_validate_packet(&packet) != PET_PACKET_STATUS_OK) {
        return PET_RESULT_ERROR;
    }
    if ((packet.seq != 7u) || (packet.ack != 3u)) {
        return PET_RESULT_ERROR;
    }
    packet.magic = 0u;
    if (pet_protocol_jieli_validate_packet(&packet) != PET_PACKET_STATUS_BAD_MAGIC) {
        return PET_RESULT_ERROR;
    }
    packet.magic = PET_PACKET_MAGIC;
    packet.version = 0u;
    if (pet_protocol_jieli_validate_packet(&packet) != PET_PACKET_STATUS_BAD_VERSION) {
        return PET_RESULT_ERROR;
    }
    packet.version = PET_PACKET_VERSION;
    packet.len = (pet_u16_t)(PET_PACKET_MAX_PAYLOAD + 1u);
    if (pet_protocol_jieli_validate_packet(&packet) != PET_PACKET_STATUS_BAD_LEN) {
        return PET_RESULT_ERROR;
    }
    packet.len = sizeof(payload);
    pet_protocol_jieli_packet_finalize(&packet);
    packet.crc16 ^= 1u;
    if (pet_protocol_jieli_validate_packet(&packet) != PET_PACKET_STATUS_BAD_CRC) {
        return PET_RESULT_ERROR;
    }
    if (pet_protocol_jieli_build_packet(PET_PACKET_PING, 0u, 0u, payload,
                                        (pet_u16_t)(PET_PACKET_MAX_PAYLOAD + 1u), 0u,
                                        &packet) != PET_RESULT_INVALID_ARGUMENT) {
        return PET_RESULT_ERROR;
    }

    ret = pet_protocol_jieli_build_nfc_pair_payload(0x10203040u, 0x1234u,
                                                    0x01020304u, 0x55667788u,
                                                    PET_NFC_PAIR_PROTOCOL_VERSION,
                                                    PET_NFC_PAIR_BLE_SERVICE_ID,
                                                    PET_NFC_PAIR_RESOURCE_VERSION,
                                                    0u, &pair);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    if (pet_protocol_jieli_validate_nfc_pair_payload(&pair) != PET_RESULT_OK) {
        return PET_RESULT_ERROR;
    }
    pair.protocol_version = 0u;
    if (pet_protocol_jieli_validate_nfc_pair_payload(&pair) != PET_RESULT_BAD_VERSION) {
        return PET_RESULT_ERROR;
    }
    pair.protocol_version = PET_NFC_PAIR_PROTOCOL_VERSION;
    pet_protocol_jieli_nfc_pair_finalize(&pair);
    pair.crc16 ^= 1u;
    if (pet_protocol_jieli_validate_nfc_pair_payload(&pair) != PET_RESULT_BAD_CRC) {
        return PET_RESULT_ERROR;
    }

    return PET_RESULT_OK;
}
