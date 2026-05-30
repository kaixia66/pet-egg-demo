#include "pet_protocol.h"
#include "pet_protocol_jieli.h"

PET_STATIC_ASSERT(p7_packet_header_size,
                  offsetof(pet_packet_t, payload) == PET_PACKET_SERIALIZED_HEADER_SIZE);
PET_STATIC_ASSERT(p7_nfc_pair_payload_size,
                  sizeof(pet_nfc_pair_payload_t) == PET_NFC_PAIR_PAYLOAD_SERIALIZED_SIZE);

pet_result_t pet_protocol_jieli_compile_check(void)
{
    return pet_protocol_jieli_self_test();
}
