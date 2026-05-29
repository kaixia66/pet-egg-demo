#include "pet_reward.h"

#include "pet_game_hash.h"

#include <string.h>

uint32_t pet_reward_summary_hash(const pet_reward_summary_t* reward) {
  uint32_t hash;
  if (reward == 0) {
    return 0u;
  }
  hash = pet_game_hash_begin();
  hash = pet_game_hash_u16(hash, reward->reward_id);
  hash = pet_game_hash_u16(hash, (uint16_t)reward->bond_delta);
  hash = pet_game_hash_u16(hash, (uint16_t)reward->care_delta);
  hash = pet_game_hash_u16(hash, (uint16_t)reward->growth_delta);
  hash = pet_game_hash_u16(hash, reward->virtual_card_delta_count);
  hash = pet_game_hash_u16(hash, reward->flags);
  return pet_game_hash_finish(hash);
}

pet_result_t pet_reward_build_boss_summary(const pet_boss_result_summary_t* boss_result,
                                           pet_reward_summary_t* out_reward) {
  if (boss_result == 0 || out_reward == 0) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  memset(out_reward, 0, sizeof(*out_reward));
  out_reward->reward_id = boss_result->reward_id;
  if (boss_result->status == PET_BOSS_STATUS_SUCCESS) {
    out_reward->bond_delta = 10;
    out_reward->care_delta = 5;
    out_reward->growth_delta = 25;
    out_reward->virtual_card_delta_count = 1u;
    out_reward->flags = PET_BOSS_RESULT_FLAG_SUCCESS;
  } else {
    out_reward->bond_delta = 2;
    out_reward->care_delta = 1;
    out_reward->growth_delta = 5;
    out_reward->virtual_card_delta_count = 0u;
    out_reward->flags = boss_result->flags;
  }
  out_reward->summary_hash = pet_reward_summary_hash(out_reward);
  return PET_RESULT_OK;
}
