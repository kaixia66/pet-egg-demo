#include "petegg_jieli_port_state.h"

#include <string.h>

PetKeyEvent g_petegg_jieli_fake_key;
uint8_t g_petegg_jieli_has_fake_key;
pet_nfc_card_payload_t g_petegg_jieli_fake_nfc;
uint8_t g_petegg_jieli_has_fake_nfc;
petegg_jieli_battery_state_t g_petegg_jieli_fake_battery;
uint8_t g_petegg_jieli_storage[2][PET_SAVE_SLOT_SERIALIZED_SIZE];
uint32_t g_petegg_jieli_storage_len[2];

void petegg_jieli_reset_fake_state(void) {
  memset(&g_petegg_jieli_fake_key, 0, sizeof(g_petegg_jieli_fake_key));
  memset(&g_petegg_jieli_fake_nfc, 0, sizeof(g_petegg_jieli_fake_nfc));
  memset(g_petegg_jieli_storage, 0, sizeof(g_petegg_jieli_storage));
  g_petegg_jieli_has_fake_key = 0u;
  g_petegg_jieli_has_fake_nfc = 0u;
  g_petegg_jieli_storage_len[0] = 0u;
  g_petegg_jieli_storage_len[1] = 0u;
  g_petegg_jieli_fake_battery.millivolts = 3900u;
  g_petegg_jieli_fake_battery.percent = 80u;
  g_petegg_jieli_fake_battery.charging = 0u;
  g_petegg_jieli_fake_battery.low_battery = 0u;
}

int petegg_jieli_port_init(void) {
  petegg_jieli_reset_fake_state();
  return PETEGG_JIELI_OK;
}
