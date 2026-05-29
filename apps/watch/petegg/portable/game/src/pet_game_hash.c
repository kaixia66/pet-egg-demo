#include "pet_game_hash.h"

uint32_t pet_game_hash_begin(void) {
  return PET_GAME_HASH_OFFSET_BASIS;
}

uint32_t pet_game_hash_u8(uint32_t hash, uint8_t value) {
  hash ^= value;
  hash *= PET_GAME_HASH_PRIME;
  return hash;
}

uint32_t pet_game_hash_u16(uint32_t hash, uint16_t value) {
  hash = pet_game_hash_u8(hash, (uint8_t)(value & 0xFFu));
  hash = pet_game_hash_u8(hash, (uint8_t)((value >> 8u) & 0xFFu));
  return hash;
}

uint32_t pet_game_hash_u32(uint32_t hash, uint32_t value) {
  hash = pet_game_hash_u8(hash, (uint8_t)(value & 0xFFu));
  hash = pet_game_hash_u8(hash, (uint8_t)((value >> 8u) & 0xFFu));
  hash = pet_game_hash_u8(hash, (uint8_t)((value >> 16u) & 0xFFu));
  hash = pet_game_hash_u8(hash, (uint8_t)((value >> 24u) & 0xFFu));
  return hash;
}

uint32_t pet_game_hash_finish(uint32_t hash) {
  return hash == 0u ? 1u : hash;
}
