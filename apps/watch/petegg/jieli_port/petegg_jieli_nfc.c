#include "petegg_jieli_port_state.h"

int petegg_jieli_nfc_poll_stub(pet_nfc_card_payload_t* out_card, uint8_t* out_has_card) {
  if (!out_card || !out_has_card) {
    return PETEGG_JIELI_ERR_INVALID_ARG;
  }
  *out_has_card = g_petegg_jieli_has_fake_nfc;
  if (g_petegg_jieli_has_fake_nfc) {
    *out_card = g_petegg_jieli_fake_nfc;
    g_petegg_jieli_has_fake_nfc = 0u;
  }
  return PETEGG_JIELI_OK;
}
