#include "petegg_jieli_port_state.h"

#include <string.h>

int petegg_jieli_storage_read_stub(uint8_t slot,
                                   uint8_t* out_bytes,
                                   uint32_t capacity,
                                   uint32_t* out_len) {
  if (slot > 1u || !out_bytes || !out_len) {
    return PETEGG_JIELI_ERR_INVALID_ARG;
  }
  if (g_petegg_jieli_storage_len[slot] == 0u) {
    *out_len = 0u;
    return PETEGG_JIELI_ERR_UNSUPPORTED;
  }
  if (capacity < g_petegg_jieli_storage_len[slot]) {
    return PETEGG_JIELI_ERR_INVALID_ARG;
  }
  memcpy(out_bytes, g_petegg_jieli_storage[slot], g_petegg_jieli_storage_len[slot]);
  *out_len = g_petegg_jieli_storage_len[slot];
  return PETEGG_JIELI_OK;
}

int petegg_jieli_storage_write_stub(uint8_t slot, const uint8_t* bytes, uint32_t len) {
  if (slot > 1u || !bytes || len > PET_SAVE_SLOT_SERIALIZED_SIZE) {
    return PETEGG_JIELI_ERR_INVALID_ARG;
  }
  memcpy(g_petegg_jieli_storage[slot], bytes, len);
  g_petegg_jieli_storage_len[slot] = len;
  return PETEGG_JIELI_OK;
}
