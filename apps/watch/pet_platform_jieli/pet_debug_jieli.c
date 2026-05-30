#include "pet_platform_jieli_internal.h"

void pet_debug_jieli_init(void)
{
}

#if defined(PET_DEBUG)
pet_result_t pet_debug_inject_key_event(const pet_platform_t *platform, const pet_key_event_t *event)
{
    (void)platform;
    (void)event;
    return PET_RESULT_UNSUPPORTED;
}

pet_result_t pet_debug_inject_packet(const pet_platform_t *platform, const pet_packet_t *packet)
{
    (void)platform;
    (void)packet;
    return PET_RESULT_UNSUPPORTED;
}

pet_result_t pet_debug_inject_nfc_card(const pet_platform_t *platform, const pet_nfc_card_t *card)
{
    (void)platform;
    (void)card;
    return PET_RESULT_UNSUPPORTED;
}
#endif
