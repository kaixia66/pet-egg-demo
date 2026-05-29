#ifndef PETEGG_JIELI_PORT_STATE_H_
#define PETEGG_JIELI_PORT_STATE_H_

#include "pet_save_format.h"
#include "petegg_jieli_port.h"

#include <stdint.h>

extern PetKeyEvent g_petegg_jieli_fake_key;
extern uint8_t g_petegg_jieli_has_fake_key;
extern pet_nfc_card_payload_t g_petegg_jieli_fake_nfc;
extern uint8_t g_petegg_jieli_has_fake_nfc;
extern petegg_jieli_battery_state_t g_petegg_jieli_fake_battery;
extern uint8_t g_petegg_jieli_storage[2][PET_SAVE_SLOT_SERIALIZED_SIZE];
extern uint32_t g_petegg_jieli_storage_len[2];

void petegg_jieli_reset_fake_state(void);

#endif  /* PETEGG_JIELI_PORT_STATE_H_ */
