#ifndef PETEGG_PORTABLE_PET_BATTLE_H_
#define PETEGG_PORTABLE_PET_BATTLE_H_

#include "pet_growth.h"
#include "pet_result.h"
#include "pet_save_format.h"
#include "pet_species_table.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Portable Game Core battle math. It is deterministic rule calculation only: no BLE/BT
   transport, packet send/receive, SimBroker, UI, animation sync, resource IO, or save IO. */
#define PET_BATTLE_MODE_TRAINING 1u
#define PET_BATTLE_MODE_BOSS 2u
#define PET_BATTLE_MODE_PVP 3u

#define PET_BATTLE_ACTION_NONE 0u
#define PET_BATTLE_ACTION_ATTACK 1u
#define PET_BATTLE_ACTION_GUARD 2u
#define PET_BATTLE_ACTION_CHEER 3u
#define PET_BATTLE_ACTION_FOCUS 4u

#define PET_BATTLE_MULTIPLIER_DISADVANTAGE_MILLI 850u
#define PET_BATTLE_MULTIPLIER_NEUTRAL_MILLI 1000u
#define PET_BATTLE_MULTIPLIER_ADVANTAGE_MILLI 1200u

#define PET_BATTLE_FLAG_NONE 0u
#define PET_BATTLE_FLAG_ATTRIBUTE_ADVANTAGE 1u
#define PET_BATTLE_FLAG_ATTRIBUTE_DISADVANTAGE 2u

typedef enum pet_battle_status_t {
  PET_BATTLE_STATUS_OK = 0,
  PET_BATTLE_STATUS_INVALID_ARG = 1,
  PET_BATTLE_STATUS_INVALID_ATTRIBUTE = 2,
  PET_BATTLE_STATUS_INVALID_ACTION = 3,
  PET_BATTLE_STATUS_HASH_MISMATCH = 4,
  PET_BATTLE_STATUS_UNSUPPORTED = 5
} pet_battle_status_t;

typedef struct pet_battle_pet_snapshot_t {
  uint32_t pet_instance_id;
  uint16_t species_id;
  uint8_t stage;
  uint8_t attribute;
  uint16_t level_coefficient_milli;
  uint16_t bond_score;
  uint16_t care_score;
  uint16_t growth_progress;
  uint16_t equipment_power_summary;
  uint16_t flags;
} pet_battle_pet_snapshot_t;

typedef struct pet_battle_action_t {
  uint8_t action_type;
  uint16_t action_value;
  uint16_t qte_score;
  uint16_t flags;
} pet_battle_action_t;

typedef struct pet_battle_round_input_t {
  uint16_t round_index;
  uint32_t session_seed;
  uint8_t battle_mode;
  pet_battle_pet_snapshot_t local_pet;
  pet_battle_pet_snapshot_t peer_pet;
  pet_battle_action_t local_action;
  pet_battle_action_t peer_action;
  uint32_t state_hash_before;
} pet_battle_round_input_t;

typedef struct pet_battle_round_result_t {
  int32_t local_effect;
  int32_t peer_effect;
  int32_t boss_effect;
  uint16_t qte_bonus;
  uint16_t attribute_multiplier_milli;
  int32_t round_score;
  uint32_t state_hash_after;
  uint32_t result_hash;
  uint16_t flags;
} pet_battle_round_result_t;

typedef pet_battle_status_t PetBattleStatus;
typedef pet_battle_pet_snapshot_t PetBattlePetSnapshot;
typedef pet_battle_action_t PetBattleAction;
typedef pet_battle_round_input_t PetBattleRoundInput;
typedef pet_battle_round_result_t PetBattleRoundResult;

pet_result_t pet_battle_attribute_multiplier_milli(uint8_t attacker_attribute,
                                                   uint8_t defender_attribute,
                                                   uint16_t* out_multiplier_milli);
pet_result_t pet_battle_build_pet_snapshot(const pet_pet_record_t* pet,
                                           pet_battle_pet_snapshot_t* out_snapshot);
pet_result_t pet_battle_resolve_round(const pet_battle_round_input_t* input,
                                      pet_battle_round_result_t* out_result);
uint32_t pet_battle_state_hash(const pet_battle_round_input_t* input);
uint32_t pet_battle_result_hash(const pet_battle_round_result_t* result);
const char* pet_battle_action_name(uint8_t action_type);
const char* pet_battle_status_name(uint8_t status);

#ifdef __cplusplus
}
#endif

#endif  /* PETEGG_PORTABLE_PET_BATTLE_H_ */
