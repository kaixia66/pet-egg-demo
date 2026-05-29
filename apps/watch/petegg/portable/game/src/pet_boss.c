#include "pet_boss.h"

#include "pet_game_hash.h"

#include <string.h>

static uint8_t pet_boss_attribute_valid(uint8_t attribute) {
  return attribute == PET_ATTRIBUTE_SPIRIT_WOOD || attribute == PET_ATTRIBUTE_BLAZING_FIRE ||
         attribute == PET_ATTRIBUTE_DARK_WATER;
}

static uint8_t pet_boss_config_valid(const pet_boss_config_t* config) {
  return config != 0 && config->boss_id != 0u && config->boss_hp != 0u &&
         config->round_limit != 0u && pet_boss_attribute_valid(config->boss_attribute) != 0u;
}

static void pet_boss_hash_config(uint32_t* hash, const pet_boss_config_t* config) {
  *hash = pet_game_hash_u32(*hash, config->boss_id);
  *hash = pet_game_hash_u16(*hash, config->boss_level);
  *hash = pet_game_hash_u8(*hash, config->boss_attribute);
  *hash = pet_game_hash_u32(*hash, config->boss_hp);
  *hash = pet_game_hash_u16(*hash, config->round_limit);
  *hash = pet_game_hash_u16(*hash, config->qte_required_score);
  *hash = pet_game_hash_u16(*hash, config->reward_id);
  *hash = pet_game_hash_u16(*hash, config->flags);
}

static pet_battle_pet_snapshot_t pet_boss_build_boss_snapshot(
    const pet_boss_session_state_t* state) {
  pet_battle_pet_snapshot_t boss;
  memset(&boss, 0, sizeof(boss));
  boss.pet_instance_id = state->boss_config.boss_id;
  boss.species_id = (uint16_t)(60000u + (state->boss_config.boss_id % 1000u));
  boss.stage = PET_STAGE_5_FINAL;
  boss.attribute = state->boss_config.boss_attribute;
  boss.level_coefficient_milli = (uint16_t)(1000u + state->boss_config.boss_level * 50u);
  boss.bond_score = 0u;
  boss.care_score = (uint16_t)(100u + state->boss_config.boss_level * 10u);
  boss.growth_progress = 0u;
  boss.equipment_power_summary = (uint16_t)(state->boss_config.boss_level * 20u);
  return boss;
}

static uint16_t pet_boss_clamp_score(uint16_t score) {
  return score > 100u ? 100u : score;
}

uint32_t pet_boss_state_hash(const pet_boss_session_state_t* state) {
  uint32_t hash;
  if (state == 0) {
    return 0u;
  }
  hash = pet_game_hash_begin();
  hash = pet_game_hash_u32(hash, state->session_seed);
  pet_boss_hash_config(&hash, &state->boss_config);
  hash = pet_game_hash_u16(hash, state->round_index);
  hash = pet_game_hash_u32(hash, state->boss_hp_remaining);
  hash = pet_game_hash_u32(hash, (uint32_t)state->local_total_score);
  hash = pet_game_hash_u32(hash, (uint32_t)state->peer_total_score);
  hash = pet_game_hash_u8(hash, state->status);
  hash = pet_game_hash_u16(hash, state->flags);
  return pet_game_hash_finish(hash);
}

uint32_t pet_boss_result_summary_hash(const pet_boss_result_summary_t* summary) {
  uint32_t hash;
  if (summary == 0) {
    return 0u;
  }
  hash = pet_game_hash_begin();
  hash = pet_game_hash_u32(hash, summary->boss_id);
  hash = pet_game_hash_u16(hash, summary->reward_id);
  hash = pet_game_hash_u8(hash, summary->status);
  hash = pet_game_hash_u16(hash, summary->rounds_played);
  hash = pet_game_hash_u32(hash, summary->boss_hp_remaining);
  hash = pet_game_hash_u32(hash, (uint32_t)summary->local_total_score);
  hash = pet_game_hash_u32(hash, (uint32_t)summary->peer_total_score);
  hash = pet_game_hash_u32(hash, summary->state_hash);
  hash = pet_game_hash_u16(hash, summary->flags);
  return pet_game_hash_finish(hash);
}

pet_result_t pet_boss_session_init(const pet_boss_config_t* config,
                                   uint32_t session_seed,
                                   pet_boss_session_state_t* out_state) {
  if (pet_boss_config_valid(config) == 0u || session_seed == 0u || out_state == 0) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  memset(out_state, 0, sizeof(*out_state));
  out_state->session_seed = session_seed;
  out_state->boss_config = *config;
  out_state->round_index = 0u;
  out_state->boss_hp_remaining = config->boss_hp;
  out_state->status = PET_BOSS_STATUS_IN_PROGRESS;
  out_state->state_hash = pet_boss_state_hash(out_state);
  return PET_RESULT_OK;
}

pet_result_t pet_boss_resolve_round(pet_boss_session_state_t* state,
                                    const pet_battle_pet_snapshot_t* local_pet,
                                    const pet_battle_pet_snapshot_t* peer_pet,
                                    const pet_boss_action_select_t* action,
                                    pet_boss_round_result_t* out_result) {
  pet_battle_pet_snapshot_t boss_snapshot;
  pet_battle_round_input_t local_input;
  pet_battle_round_input_t peer_input;
  pet_battle_round_result_t local_result;
  pet_battle_round_result_t peer_result;
  uint32_t total_damage;
  uint16_t qte_bonus;

  if (out_result != 0) {
    memset(out_result, 0, sizeof(*out_result));
  }
  if (state == 0 || local_pet == 0 || peer_pet == 0 || action == 0 || out_result == 0 ||
      state->status != PET_BOSS_STATUS_IN_PROGRESS) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  if (state->round_index >= state->boss_config.round_limit) {
    state->status = PET_BOSS_STATUS_FAILED;
    state->state_hash = pet_boss_state_hash(state);
    return PET_RESULT_INVALID_ARGUMENT;
  }

  boss_snapshot = pet_boss_build_boss_snapshot(state);
  memset(&local_input, 0, sizeof(local_input));
  local_input.round_index = (uint16_t)(state->round_index + 1u);
  local_input.session_seed = state->session_seed ^ action->round_nonce;
  local_input.battle_mode = PET_BATTLE_MODE_BOSS;
  local_input.local_pet = *local_pet;
  local_input.peer_pet = boss_snapshot;
  local_input.local_action.action_type = action->action_type;
  local_input.local_action.action_value = action->event_seq;
  local_input.local_action.qte_score = action->local_qte_score;
  local_input.peer_action.action_type = PET_BATTLE_ACTION_GUARD;
  local_input.peer_action.qte_score = state->boss_config.qte_required_score / 2u;

  peer_input = local_input;
  peer_input.local_pet = *peer_pet;
  peer_input.local_action.qte_score = action->peer_qte_score;
  peer_input.peer_pet = boss_snapshot;

  if (pet_battle_resolve_round(&local_input, &local_result) != PET_RESULT_OK ||
      pet_battle_resolve_round(&peer_input, &peer_result) != PET_RESULT_OK) {
    return PET_RESULT_INVALID_ARGUMENT;
  }

  qte_bonus = (uint16_t)((pet_boss_clamp_score(action->local_qte_score) +
                          pet_boss_clamp_score(action->peer_qte_score)) /
                         2u);
  total_damage = (uint32_t)(local_result.local_effect + peer_result.local_effect);
  if (total_damage > state->boss_hp_remaining) {
    total_damage = state->boss_hp_remaining;
  }

  out_result->round_index = (uint16_t)(state->round_index + 1u);
  out_result->boss_hp_before = state->boss_hp_remaining;
  out_result->boss_hp_after = state->boss_hp_remaining - total_damage;
  out_result->local_contribution = local_result.local_effect;
  out_result->peer_contribution = peer_result.local_effect;
  out_result->qte_bonus = qte_bonus;
  out_result->action_bonus = (uint16_t)(local_result.qte_bonus + peer_result.qte_bonus);
  out_result->attribute_bonus = local_result.attribute_multiplier_milli;
  out_result->flags = PET_BOSS_RESULT_FLAG_NONE;

  state->round_index = out_result->round_index;
  state->boss_hp_remaining = out_result->boss_hp_after;
  state->local_total_score += out_result->local_contribution;
  state->peer_total_score += out_result->peer_contribution;
  if (state->boss_hp_remaining == 0u) {
    state->status = PET_BOSS_STATUS_SUCCESS;
    state->flags |= PET_BOSS_RESULT_FLAG_SUCCESS;
    out_result->flags |= PET_BOSS_RESULT_FLAG_SUCCESS;
  } else if (state->round_index >= state->boss_config.round_limit) {
    state->status = PET_BOSS_STATUS_FAILED;
    state->flags |= PET_BOSS_RESULT_FLAG_ROUND_LIMIT;
    out_result->flags |= PET_BOSS_RESULT_FLAG_ROUND_LIMIT;
  }
  state->state_hash = pet_boss_state_hash(state);
  out_result->state_hash = state->state_hash;
  out_result->result_hash = local_result.result_hash ^ pet_game_hash_u32(peer_result.result_hash,
                                                                         state->state_hash);
  out_result->result_hash = pet_game_hash_finish(out_result->result_hash);
  state->result_hash = out_result->result_hash;
  return PET_RESULT_OK;
}

pet_result_t pet_boss_is_finished(const pet_boss_session_state_t* state,
                                  uint8_t* out_finished) {
  if (state == 0 || out_finished == 0) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  *out_finished =
      state->status == PET_BOSS_STATUS_SUCCESS || state->status == PET_BOSS_STATUS_FAILED ? 1u
                                                                                           : 0u;
  return PET_RESULT_OK;
}

pet_result_t pet_boss_build_result_summary(const pet_boss_session_state_t* state,
                                           pet_boss_result_summary_t* out_summary) {
  if (state == 0 || out_summary == 0) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  memset(out_summary, 0, sizeof(*out_summary));
  out_summary->boss_id = state->boss_config.boss_id;
  out_summary->reward_id = state->boss_config.reward_id;
  out_summary->status = state->status;
  out_summary->rounds_played = state->round_index;
  out_summary->boss_hp_remaining = state->boss_hp_remaining;
  out_summary->local_total_score = state->local_total_score;
  out_summary->peer_total_score = state->peer_total_score;
  out_summary->state_hash = state->state_hash;
  out_summary->flags = state->flags;
  out_summary->result_hash = pet_boss_result_summary_hash(out_summary);
  return PET_RESULT_OK;
}

const char* pet_boss_status_name(uint8_t status) {
  switch (status) {
    case PET_BOSS_STATUS_IN_PROGRESS:
      return "in_progress";
    case PET_BOSS_STATUS_SUCCESS:
      return "success";
    case PET_BOSS_STATUS_FAILED:
      return "failed";
    default:
      return "unknown";
  }
}
