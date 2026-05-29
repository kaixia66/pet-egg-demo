#include "petegg_jieli_port_state.h"

int petegg_jieli_debug_inject_fake_nfc(const pet_nfc_card_payload_t* payload) {
  if (!payload) {
    return PETEGG_JIELI_ERR_INVALID_ARG;
  }
  g_petegg_jieli_fake_nfc = *payload;
  g_petegg_jieli_has_fake_nfc = 1u;
  return PETEGG_JIELI_OK;
}

int petegg_jieli_debug_inject_fake_key(uint8_t key, uint8_t event_type) {
  if (key > PET_KEY_CANCEL || event_type > PET_KEY_EVENT_REPEAT) {
    return PETEGG_JIELI_ERR_INVALID_ARG;
  }
  g_petegg_jieli_fake_key.key = (PetProductKey)key;
  g_petegg_jieli_fake_key.type = (PetKeyEventType)event_type;
  g_petegg_jieli_has_fake_key = 1u;
  return PETEGG_JIELI_OK;
}

int petegg_jieli_debug_inject_fake_battery(uint16_t millivolts,
                                           uint8_t percent,
                                           uint8_t charging,
                                           uint8_t low_battery) {
  if (percent > 100u) {
    return PETEGG_JIELI_ERR_INVALID_ARG;
  }
  g_petegg_jieli_fake_battery.millivolts = millivolts;
  g_petegg_jieli_fake_battery.percent = percent;
  g_petegg_jieli_fake_battery.charging = charging ? 1u : 0u;
  g_petegg_jieli_fake_battery.low_battery = low_battery ? 1u : 0u;
  return PETEGG_JIELI_OK;
}
