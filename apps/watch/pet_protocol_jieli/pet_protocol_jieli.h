#ifndef PET_PROTOCOL_JIELI_H
#define PET_PROTOCOL_JIELI_H

#include "pet_protocol.h"

#ifdef __cplusplus
extern "C" {
#endif

pet_u16_t pet_protocol_jieli_crc16_ccitt_false(const pet_u8_t *data, pet_size_t len);
pet_u16_t pet_protocol_jieli_packet_crc16(const pet_packet_t *packet);
void pet_protocol_jieli_packet_finalize(pet_packet_t *packet);
pet_result_t pet_protocol_jieli_build_packet(pet_u8_t type,
                                             pet_u16_t seq,
                                             pet_u16_t ack,
                                             const pet_u8_t *payload,
                                             pet_u16_t payload_len,
                                             pet_u8_t flags,
                                             pet_packet_t *out_packet);
pet_packet_status_t pet_protocol_jieli_validate_packet(const pet_packet_t *packet);
pet_u16_t pet_protocol_jieli_nfc_pair_crc16(const pet_nfc_pair_payload_t *payload);
void pet_protocol_jieli_nfc_pair_finalize(pet_nfc_pair_payload_t *payload);
pet_result_t pet_protocol_jieli_build_nfc_pair_payload(pet_u32_t device_id,
                                                       pet_u16_t device_short_id,
                                                       pet_u32_t nonce,
                                                       pet_u32_t session_seed,
                                                       pet_u16_t protocol_version,
                                                       pet_u16_t service_id,
                                                       pet_u16_t resource_version,
                                                       pet_u16_t flags,
                                                       pet_nfc_pair_payload_t *out_payload);
pet_result_t pet_protocol_jieli_validate_nfc_pair_payload(const pet_nfc_pair_payload_t *payload);
pet_result_t pet_protocol_jieli_self_test(void);

#ifdef __cplusplus
}
#endif

#endif
