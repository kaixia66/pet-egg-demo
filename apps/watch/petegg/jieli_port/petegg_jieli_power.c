#include "petegg_jieli_port_state.h"

int petegg_jieli_power_state_stub(petegg_jieli_battery_state_t* out_state) {
  if (!out_state) {
    return PETEGG_JIELI_ERR_INVALID_ARG;
  }
  *out_state = g_petegg_jieli_fake_battery;
  return PETEGG_JIELI_OK;
}
