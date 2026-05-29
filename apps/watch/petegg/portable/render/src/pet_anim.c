#include "pet_anim.h"

#include <string.h>

pet_result_t pet_anim_clip_validate(const pet_anim_clip_t* clip) {
  uint16_t i;
  if (clip == 0 || clip->animation_id == 0u || clip->frame_count == 0u || clip->frames == 0) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  for (i = 0u; i < clip->frame_count; ++i) {
    if (clip->frames[i].duration_ms == 0u) {
      return PET_RESULT_INVALID_ARGUMENT;
    }
  }
  return PET_RESULT_OK;
}

pet_result_t pet_anim_player_init(pet_anim_player_t* player, uint32_t animation_id) {
  if (player == 0 || animation_id == 0u) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  memset(player, 0, sizeof(*player));
  player->animation_id = animation_id;
  player->playing = 1u;
  return PET_RESULT_OK;
}

pet_result_t pet_anim_player_reset(pet_anim_player_t* player) {
  uint32_t animation_id;
  if (player == 0 || player->animation_id == 0u) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  animation_id = player->animation_id;
  memset(player, 0, sizeof(*player));
  player->animation_id = animation_id;
  player->playing = 1u;
  return PET_RESULT_OK;
}

pet_result_t pet_anim_player_current_frame(const pet_anim_player_t* player,
                                           const pet_anim_clip_t* clip,
                                           const pet_anim_frame_t** out_frame) {
  if (out_frame != 0) {
    *out_frame = 0;
  }
  if (player == 0 || clip == 0 || out_frame == 0 ||
      pet_anim_clip_validate(clip) != PET_RESULT_OK ||
      player->animation_id != clip->animation_id ||
      player->current_frame_index >= clip->frame_count) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  *out_frame = &clip->frames[player->current_frame_index];
  return PET_RESULT_OK;
}

pet_result_t pet_anim_player_tick(pet_anim_player_t* player,
                                  const pet_anim_clip_t* clip,
                                  uint32_t dt_ms) {
  if (player == 0 || clip == 0 || pet_anim_clip_validate(clip) != PET_RESULT_OK ||
      player->animation_id != clip->animation_id) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  if (player->finished != 0u || player->playing == 0u || dt_ms == 0u) {
    return PET_RESULT_OK;
  }
  while (dt_ms > 0u && player->finished == 0u) {
    const pet_anim_frame_t* frame;
    uint32_t remaining;
    if (player->current_frame_index >= clip->frame_count) {
      return PET_RESULT_INVALID_ARGUMENT;
    }
    frame = &clip->frames[player->current_frame_index];
    remaining = frame->duration_ms - player->elapsed_in_frame_ms;
    if (dt_ms < remaining) {
      player->elapsed_in_frame_ms += dt_ms;
      dt_ms = 0u;
    } else {
      dt_ms -= remaining;
      player->elapsed_in_frame_ms = 0u;
      player->current_frame_index += 1u;
      if (player->current_frame_index >= clip->frame_count) {
        if ((clip->flags & PET_ANIM_FLAG_LOOP) != 0u &&
            (clip->loop_count == PET_ANIM_LOOP_FOREVER ||
             player->loop_counter + 1u < clip->loop_count)) {
          player->current_frame_index = 0u;
          player->loop_counter += 1u;
        } else {
          player->current_frame_index = (uint16_t)(clip->frame_count - 1u);
          player->finished = 1u;
          player->playing = 0u;
        }
      }
    }
  }
  return PET_RESULT_OK;
}
