#include "petegg_jieli_port.h"

int petegg_jieli_display_flush_stub(uint16_t x,
                                    uint16_t y,
                                    uint16_t width,
                                    uint16_t height,
                                    const uint16_t* rgb565) {
  (void)x;
  (void)y;
  if (width == 0u || height == 0u || !rgb565) {
    return PETEGG_JIELI_ERR_INVALID_ARG;
  }
  return PETEGG_JIELI_OK;
}
