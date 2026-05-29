#ifndef PETEGG_PORTABLE_PET_CRC32_H_
#define PETEGG_PORTABLE_PET_CRC32_H_

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint32_t pet_crc32_ieee(const uint8_t* data, size_t len);

#ifdef __cplusplus
}
#endif

#endif  /* PETEGG_PORTABLE_PET_CRC32_H_ */
