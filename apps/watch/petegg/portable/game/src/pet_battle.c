#include "pet_battle.h"

#include "pet_game_hash.h"

#include <string.h>

static uint8_t pet_battle_attribute_valid(uint8_t attribute) {
  return attribute == PET_ATTRIBUTE_SPIRIT_WOOD || attribute == PET_ATTRIBUTE_BLAZING_FIRE ||
         attribute == PET_ATTRIBUTE_DARK_WATER;
}

static uint8_t pet_battle_action_valid(uint8_t action_type) {
  return action_type == PET_BATTLE_ACTION_ATTACK || action_type == PET_BATTLE_ACTION_GUARD ||
         action_type == PET_BATTLE_ACTION_CHEER || action_type == PET_BATTLE_ACTION_FOCUS;
}

static uint8_t pet_battle_has_advantage(uint8_t attacker_attribute, uint8_t defender_attribute) {
  return (attacker_attribute == PET_ATTRIBUTE_SPIRIT_WOOD &&
          defender_attribute == PET_ATTRIBUTE_DARK_WATER) ||
         (attacker_attribute == PET_ATTRIBUTE_DARK_WATER &&
          defender_attribute == PET_ATTRIBUTE_BLAZING_FIRE) ||
         (attacker_attribute == PET_ATTRIBUTE_BLAZING_FIRE &&
          defender_attribute == PET_ATTRIBUTE_SPIRIT_WOOD);
}

static uint16_t pet_battle_action_bonus(uint8_t action_type, uint16_t action_value) {
  uint16_t value = action_value;
  if (value > 100u) {
    value = 100u;
  }
  switch (action_type) {
    case PET_BATTLE_ACTION_ATTACK:
      return (uint16_t)(30u + value / 4u);
    case PET_BATTLE_ACTION_GUARD:
      return (uint16_t)(10u + value / 8u);
    case PET_BATTLE_ACTION_CHEER:
      return (uint16_t)(15u + value / 6u);
    case PET_BATTLE_ACTION_FOCUS:
      return (uint16_t)(20u + value / 5u);
    default:
      return 0u;
  }
}

static int32_t pet_battle_base_effect(const pet_battle_pet_snapshot_t* pet) {
  return 100 + ((int32_t)pet->stage * 10) + ((int32_t)pet->care_score / 20) +
         ((int32_t)pet->bond_score / 25) + ((int32_t)pet->growth_progress / 50) +
         ((int32_t)pet->level_coefficient_milli / 100) +
         ((int32_t)pet->equipment_power_summary / 10);
}

static uint16_t pet_battle_clamp_qte_score(uint16_t qte_score) {
  return qte_score > 100u ? 100u : qte_score;
}

static uint32_t pet_battle_hash_snapshot(uint32_t hash,
                                         const pet_battle_pet_snapshot_t* snapshot) {
  hash = pet_game_hash_u32(hash, snapshot->pet_instance_id);
  hash = pet_game_hash_u16(hash, snapshot->species_id);
  hash = pet_game_hash_u8(hash, snapshot->stage);
  hash = pet_game_hash_u8(hash, snapshot->attribute);
  hash = pet_game_hash_u16(hash, snapshot->level_coefficient_milli);
  hash = pet_game_hash_u16(hash, snapshot->bond_score);
  hash = pet_game_hash_u16(hash, snapshot->care_score);
  hash = pet_game_hash_u16(hash, snapshot->growth_progress);
  hash = pet_game_hash_u16(hash, snapshot->equipment_power_summary);
  hash = pet_game_hash_u16(hash, snapshot->flags);
  return hash;
}

static uint32_t pet_battle_hash_action(uint32_t hash, const pet_battle_action_t* action) {
  hash = pet_game_hash_u8(hash, action->action_type);
  hash = pet_game_hash_u16(hash, action->action_value);
  hash = pet_game_hash_u16(hash, action->qte_score);
  hash = pet_game_hash_u16(hash, action->flags);
  return hash;
}

pet_result_t pet_battle_attribute_multiplier_milli(uint8_t attacker_attribute,
                                                   uint8_t defender_attribute,
                                                   uint16_t* out_multiplier_milli) {
  if (out_multiplier_milli == 0) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  *out_multiplier_milli = 0u;
  if (pet_battle_attribute_valid(attacker_attribute) == 0u ||
      pet_battle_attribute_valid(defender_attribute) == 0u) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  if (attacker_attribute == defender_attribute) {
    *out_multiplier_milli = PET_BATTLE_MULTIPLIER_NEUTRAL_MILLI;
  } else if (pet_battle_has_advantage(attacker_attribute, defender_attribute) != 0u) {
    *out_multiplier_milli = PET_BATTLE_MULTIPLIER_ADVANTAGE_MILLI;
  } else if (pet_battle_has_advantage(defender_attribute, attacker_attribute) != 0u) {
    *out_multiplier_milli = PET_BATTLE_MULTIPLIER_DISADVANTAGE_MILLI;
  } else {
    *out_multiplier_milli = PET_BATTLE_MULTIPLIER_NEUTRAL_MILLI;
  }
  return PET_RESULT_OK;
}

pet_result_t pet_battle_build_pet_snapshot(const pet_pet_record_t* pet,
                                           pet_battle_pet_snapshot_t* out_snapshot) {
  if (pet == 0 || out_snapshot == 0 || pet_model_validate(pet) != PET_RESULT_OK) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  memset(out_snapshot, 0, sizeof(*out_snapshot));
  out_snapshot->pet_instance_id = pet->pet_id;
  out_snapshot->species_id = (uint16_t)pet->species_id;
  out_snapshot->stage = pet->stage;
  out_snapshot->attribute = pet->attribute;
  out_snapshot->level_coefficient_milli = pet->level_coefficient_milli;
  out_snapshot->bond_score = pet->bond_score;
  out_snapshot->care_score = pet->care_score;
  out_snapshot->growth_progress = pet_growth_progress(pet);
  out_snapshot->flags = pet->status;
  return PET_RESULT_OK;
}

uint32_t pet_battle_state_hash(const pet_battle_round_input_t* input) {
  uint32_t hash;
  if (input == 0) {
    return 0u;
  }
  hash = pet_game_hash_begin();
  hash = pet_game_hash_u16(hash, input->round_index);
  hash = pet_game_hash_u32(hash, input->session_seed);
  hash = pet_game_hash_u8(hash, input->battle_mode);
  hash = pet_battle_hash_snapshot(hash, &input->local_pet);
  hash = pet_battle_hash_snapshot(hash, &input->peer_pet);
  hash = pet_battle_hash_action(hash, &input->local_action);
  hash = pet_battle_hash_action(hash, &input->peer_action);
  return pet_game_hash_finish(hash);
}

uint32_t pet_battle_result_hash(const pet_battle_round_result_t* result) {
  uint32_t hash;
  if (result == 0) {
    return 0u;
  }
  hash = pet_game_hash_begin();
  hash = pet_game_hash_u32(hash, (uint32_t)result->local_effect);
  hash = pet_game_hash_u32(hash, (uint32_t)result->peer_effect);
  hash = pet_game_hash_u32(hash, (uint32_t)result->boss_effect);
  hash = pet_game_hash_u16(hash, result->qte_bonus);
  hash = pet_game_hash_u16(hash, result->attribute_multiplier_milli);
  hash = pet_game_hash_u32(hash, (uint32_t)result->round_score);
  hash = pet_game_hash_u32(hash, result->state_hash_after);
  hash = pet_game_hash_u16(hash, result->flags);
  return pet_game_hash_finish(hash);
}

pet_result_t pet_battle_resolve_round(const pet_battle_round_input_t* input,
                                      pet_battle_round_result_t* out_result) {
  uint16_t local_multiplier = 0u;
  uint16_t peer_multiplier = 0u;
  uint16_t local_qte;
  uint16_t peer_qte;
  uint32_t expected_hash;
  int32_t local_effect;
  int32_t peer_effect;

  if (out_result != 0) {
    memset(out_result, 0, sizeof(*out_result));
  }
  if (input == 0 || out_result == 0) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  if (input->battle_mode != PET_BATTLE_MODE_TRAINING &&
      input->battle_mode != PET_BATTLE_MODE_BOSS && input->battle_mode != PET_BATTLE_MODE_PVP) {
    return PET_RESULT_UNSUPPORTED;
  }
  if (pet_battle_action_valid(input->local_action.action_type) == 0u ||
      pet_battle_action_valid(input->peer_action.action_type) == 0u) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  if (pet_battle_attribute_multiplier_milli(input->local_pet.attribute, input->peer_pet.attribute,
                                           &local_multiplier) != PET_RESULT_OK ||
      pet_battle_attribute_multiplier_milli(input->peer_pet.attribute, input->local_pet.attribute,
                                           &peer_multiplier) != PET_RESULT_OK) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  expected_hash = pet_battle_state_hash(input);
  if (input->state_hash_before != 0u && input->state_hash_before != expected_hash) {
    return PET_RESULT_BAD_CRC;
  }

  local_qte = pet_battle_clamp_qte_score(input->local_action.qte_score);
  peer_qte = pet_battle_clamp_qte_score(input->peer_action.qte_score);
  out_result->qte_bonus = (uint16_t)((local_qte + peer_qte) / 2u);
  out_result->attribute_multiplier_milli = local_multiplier;
  local_effect = ((pet_battle_base_effect(&input->local_pet) +
                   (int32_t)pet_battle_action_bonus(input->local_action.action_type,
                                                    input->local_action.action_value)) *
                  (int32_t)local_multiplier) /
                 1000;
  peer_effect = ((pet_battle_base_effect(&input->peer_pet) +
                  (int32_t)pet_battle_action_bonus(input->peer_action.action_type,
                                                   input->peer_action.action_value)) *
                 (int32_t)peer_multiplier) /
                1000;
  local_effect += (int32_t)local_qte * 2;
  peer_effect += (int32_t)peer_qte * 2;
  if (input->local_action.action_type == PET_BATTLE_ACTION_GUARD) {
    peer_effect = (peer_effect * 80) / 100;
  }
  if (input->peer_action.action_type == PET_BATTLE_ACTION_GUARD) {
    local_effect = (local_effect * 80) / 100;
  }

  out_result->local_effect = local_effect;
  out_result->peer_effect = peer_effect;
  out_result->boss_effect = local_effect + peer_effect;
  out_result->round_score = local_effect - peer_effect + (int32_t)out_result->qte_bonus;
  out_result->state_hash_after = expected_hash;
  if (local_multiplier == PET_BATTLE_MULTIPLIER_ADVANTAGE_MILLI) {
    out_result->flags |= PET_BATTLE_FLAG_ATTRIBUTE_ADVANTAGE;
  } else if (local_multiplier == PET_BATTLE_MULTIPLIER_DISADVANTAGE_MILLI) {
    out_result->flags |= PET_BATTLE_FLAG_ATTRIBUTE_DISADVANTAGE;
  }
  out_result->result_hash = pet_battle_result_hash(out_result);
  return PET_RESULT_OK;
}

const char* pet_battle_action_name(uint8_t action_type) {
  switch (action_type) {
    case PET_BATTLE_ACTION_NONE:
      return "none";
    case PET_BATTLE_ACTION_ATTACK:
      return "attack";
    case PET_BATTLE_ACTION_GUARD:
      return "guard";
    case PET_BATTLE_ACTION_CHEER:
      return "cheer";
    case PET_BATTLE_ACTION_FOCUS:
      return "focus";
    default:
      return "unknown";
  }
}

const char* pet_battle_status_name(uint8_t status) {
  switch (status) {
    case PET_BATTLE_STATUS_OK:
      return "ok";
    case PET_BATTLE_STATUS_INVALID_ARG:
      return "invalid_arg";
    case PET_BATTLE_STATUS_INVALID_ATTRIBUTE:
      return "invalid_attribute";
    case PET_BATTLE_STATUS_INVALID_ACTION:
      return "invalid_action";
    case PET_BATTLE_STATUS_HASH_MISMATCH:
      return "hash_mismatch";
    case PET_BATTLE_STATUS_UNSUPPORTED:
      return "unsupported";
    default:
      return "unknown";
  }
}
