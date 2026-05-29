#include "pet_crc32.h"

uint32_t pet_crc32_ieee(const uint8_t* data, size_t len) {
  uint32_t crc = 0xFFFFFFFFu;
  size_t i;

  if (data == 0 && len != 0u) {
    return 0u;
  }

  for (i = 0; i < len; ++i) {
    int bit;
    crc ^= data[i];
    for (bit = 0; bit < 8; ++bit) {
      const uint32_t mask = 0u - (crc & 1u);
      crc = (crc >> 1u) ^ (0xEDB88320u & mask);
    }
  }

  return ~crc;
}
