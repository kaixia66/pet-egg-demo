#ifndef PETEGG_PORTABLE_PET_GAME_HASH_H_
#define PETEGG_PORTABLE_PET_GAME_HASH_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Deterministic game-rule hash helper. This is not a security signature. Callers must
   feed fields explicitly in stable order and must not hash struct padding. */
#define PET_GAME_HASH_OFFSET_BASIS 2166136261u
#define PET_GAME_HASH_PRIME 16777619u

uint32_t pet_game_hash_begin(void);
uint32_t pet_game_hash_u8(uint32_t hash, uint8_t value);
uint32_t pet_game_hash_u16(uint32_t hash, uint16_t value);
uint32_t pet_game_hash_u32(uint32_t hash, uint32_t value);
uint32_t pet_game_hash_finish(uint32_t hash);

#ifdef __cplusplus
}
#endif

#endif  /* PETEGG_PORTABLE_PET_GAME_HASH_H_ */
