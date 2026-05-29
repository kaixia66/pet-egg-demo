#include "petegg_jieli_port.h"

int petegg_jieli_resource_read_stub(uint16_t resource_id,
                                    uint32_t offset,
                                    uint8_t* out_bytes,
                                    uint32_t len) {
  (void)resource_id;
  (void)offset;
  if (!out_bytes || len == 0u) {
    return PETEGG_JIELI_ERR_INVALID_ARG;
  }
  return PETEGG_JIELI_ERR_UNSUPPORTED;
}
