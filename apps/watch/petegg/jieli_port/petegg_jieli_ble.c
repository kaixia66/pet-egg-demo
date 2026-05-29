#include "petegg_jieli_port.h"

int petegg_jieli_ble_send_stub(const uint8_t* bytes, uint16_t len) {
  if (!bytes || len == 0u) {
    return PETEGG_JIELI_ERR_INVALID_ARG;
  }
  return PETEGG_JIELI_OK;
}
