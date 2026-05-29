#ifndef PETEGG_PORTABLE_PET_REWARD_H_
#define PETEGG_PORTABLE_PET_REWARD_H_

#include "pet_boss.h"
#include "pet_result.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Portable Game Core reward summary. P9 only builds a deterministic summary; it does not
   apply rewards, add cards, write save data, or commit transactions. */
typedef struct pet_reward_summary_t {
  uint16_t reward_id;
  int16_t bond_delta;
  int16_t care_delta;
  int16_t growth_delta;
  uint16_t virtual_card_delta_count;
  uint32_t summary_hash;
  uint16_t flags;
} pet_reward_summary_t;

typedef pet_reward_summary_t PetRewardSummary;

pet_result_t pet_reward_build_boss_summary(const pet_boss_result_summary_t* boss_result,
                                           pet_reward_summary_t* out_reward);
uint32_t pet_reward_summary_hash(const pet_reward_summary_t* reward);

#ifdef __cplusplus
}
#endif

#endif  /* PETEGG_PORTABLE_PET_REWARD_H_ */
