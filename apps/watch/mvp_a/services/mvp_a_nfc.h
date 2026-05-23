#ifndef MVP_A_NFC_H
#define MVP_A_NFC_H

#include "mvp_a_platform.h"

mvp_a_result_t mvp_a_nfc_start_read(void);
mvp_a_result_t mvp_a_nfc_poll(mvp_a_nfc_card_t *card);

#endif
