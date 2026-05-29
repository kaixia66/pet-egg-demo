#ifndef PETEGG_PORTABLE_PET_BOSS_H_
#define PETEGG_PORTABLE_PET_BOSS_H_

#include "pet_battle.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Portable Game Core Boss rules. This module does deterministic state math only. It does
   not send packets, access BLE/NFC, call SimBroker, render UI/animations, read resources,
   or write save data. */
#define PET_BOSS_STATUS_IN_PROGRESS 1u
#define PET_BOSS_STATUS_SUCCESS 2u
#define PET_BOSS_STATUS_FAILED 3u

#define PET_BOSS_RESULT_FLAG_NONE 0u
#define PET_BOSS_RESULT_FLAG_SUCCESS 1u
#define PET_BOSS_RESULT_FLAG_ROUND_LIMIT 2u

typedef struct pet_boss_config_t {
  uint32_t boss_id;
  uint16_t boss_level;
  uint8_t boss_attribute;
  uint32_t boss_hp;
  uint16_t round_limit;
  uint16_t qte_required_score;
  uint16_t reward_id;
  uint16_t flags;
} pet_boss_config_t;

typedef struct pet_boss_session_state_t {
  uint32_t session_seed;
  pet_boss_config_t boss_config;
  uint16_t round_index;
  uint32_t boss_hp_remaining;
  int32_t local_total_score;
  int32_t peer_total_score;
  uint32_t state_hash;
  uint32_t result_hash;
  uint8_t status;
  uint16_t flags;
} pet_boss_session_state_t;

typedef struct pet_boss_action_select_t {
  uint8_t action_type;
  uint16_t local_qte_score;
  uint16_t peer_qte_score;
  uint32_t round_nonce;
  uint16_t event_seq;
} pet_boss_action_select_t;

typedef struct pet_boss_round_result_t {
  uint16_t round_index;
  uint32_t boss_hp_before;
  uint32_t boss_hp_after;
  int32_t local_contribution;
  int32_t peer_contribution;
  uint16_t qte_bonus;
  uint16_t action_bonus;
  uint16_t attribute_bonus;
  uint32_t state_hash;
  uint32_t result_hash;
  uint16_t flags;
} pet_boss_round_result_t;

typedef struct pet_boss_result_summary_t {
  uint32_t boss_id;
  uint16_t reward_id;
  uint8_t status;
  uint16_t rounds_played;
  uint32_t boss_hp_remaining;
  int32_t local_total_score;
  int32_t peer_total_score;
  uint32_t state_hash;
  uint32_t result_hash;
  uint16_t flags;
} pet_boss_result_summary_t;

typedef pet_boss_config_t PetBossConfig;
typedef pet_boss_session_state_t PetBossSessionState;
typedef pet_boss_action_select_t PetBossActionSelect;
typedef pet_boss_round_result_t PetBossRoundResult;
typedef pet_boss_result_summary_t PetBossResultSummary;

pet_result_t pet_boss_session_init(const pet_boss_config_t* config,
                                   uint32_t session_seed,
                                   pet_boss_session_state_t* out_state);
pet_result_t pet_boss_resolve_round(pet_boss_session_state_t* state,
                                    const pet_battle_pet_snapshot_t* local_pet,
                                    const pet_battle_pet_snapshot_t* peer_pet,
                                    const pet_boss_action_select_t* action,
                                    pet_boss_round_result_t* out_result);
pet_result_t pet_boss_is_finished(const pet_boss_session_state_t* state,
                                  uint8_t* out_finished);
pet_result_t pet_boss_build_result_summary(const pet_boss_session_state_t* state,
                                           pet_boss_result_summary_t* out_summary);
uint32_t pet_boss_state_hash(const pet_boss_session_state_t* state);
uint32_t pet_boss_result_summary_hash(const pet_boss_result_summary_t* summary);
const char* pet_boss_status_name(uint8_t status);

#ifdef __cplusplus
}
#endif

#endif  /* PETEGG_PORTABLE_PET_BOSS_H_ */
