#include "mvp_a_nfc.h"
#include "mvp_a_platform.h"

mvp_a_result_t mvp_a_nfc_start_read(void)
{
    return mvp_a_platform_nfc_start_read();
}

mvp_a_result_t mvp_a_nfc_poll(mvp_a_nfc_card_t *card)
{
    return mvp_a_platform_nfc_poll(card);
}
