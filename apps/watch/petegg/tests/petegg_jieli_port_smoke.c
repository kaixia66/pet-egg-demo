#include "pet_nfc_payload.h"
#include "petegg_jieli_port.h"

int petegg_jieli_port_smoke(void) {
  PetKeyEvent key_event;
  pet_nfc_card_payload_t card_payload;
  pet_nfc_card_payload_t polled_card;
  petegg_jieli_battery_state_t battery;
  uint8_t has_card = 0u;

  if (petegg_jieli_port_init() != PETEGG_JIELI_OK) {
    return -1;
  }
  if (petegg_jieli_debug_inject_fake_key(PET_KEY_OK, PET_KEY_EVENT_CLICK) !=
      PETEGG_JIELI_OK) {
    return -2;
  }
  if (petegg_jieli_poll_input(&key_event) != PETEGG_JIELI_OK ||
      key_event.key != PET_KEY_OK ||
      key_event.type != PET_KEY_EVENT_CLICK) {
    return -3;
  }

  card_payload.uid = 0x0102030405060708ull;
  card_payload.card_type = PET_NFC_CARD_HOME;
  card_payload.rarity = 1u;
  card_payload.content_id = 5001u;
  card_payload.value = 1u;
  card_payload.flags = PET_NFC_CARD_FLAG_TEST_PAYLOAD;
  card_payload.mock_signature = pet_nfc_card_payload_expected_mock_signature(&card_payload);
  if (petegg_jieli_debug_inject_fake_nfc(&card_payload) != PETEGG_JIELI_OK) {
    return -4;
  }
  if (petegg_jieli_nfc_poll_stub(&polled_card, &has_card) != PETEGG_JIELI_OK ||
      has_card != 1u ||
      polled_card.uid != card_payload.uid) {
    return -5;
  }

  if (petegg_jieli_debug_inject_fake_battery(3700u, 40u, 0u, 0u) != PETEGG_JIELI_OK) {
    return -6;
  }
  if (petegg_jieli_power_state_stub(&battery) != PETEGG_JIELI_OK ||
      battery.millivolts != 3700u ||
      battery.percent != 40u) {
    return -7;
  }

  return 0;
}
