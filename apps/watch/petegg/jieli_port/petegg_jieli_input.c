#include "petegg_jieli_port_state.h"

int petegg_jieli_poll_input(PetKeyEvent* out_event) {
  if (!out_event) {
    return PETEGG_JIELI_ERR_INVALID_ARG;
  }
  if (!g_petegg_jieli_has_fake_key) {
    return PETEGG_JIELI_ERR_UNSUPPORTED;
  }
  *out_event = g_petegg_jieli_fake_key;
  g_petegg_jieli_has_fake_key = 0u;
  return PETEGG_JIELI_OK;
}
