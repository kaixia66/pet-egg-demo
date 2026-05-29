#ifndef PETEGG_PORTABLE_PET_ANIM_H_
#define PETEGG_PORTABLE_PET_ANIM_H_

#include "pet_result.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PET_ANIM_FLAG_NONE 0u
#define PET_ANIM_FLAG_LOOP 1u
#define PET_ANIM_LOOP_FOREVER 0xFFFFu

typedef struct pet_anim_frame_t {
  uint16_t frame_id;
  uint32_t duration_ms;
  int16_t offset_x;
  int16_t offset_y;
  uint16_t flags;
} pet_anim_frame_t;

typedef struct pet_anim_clip_t {
  uint32_t animation_id;
  uint16_t frame_count;
  uint16_t loop_count;
  uint32_t total_duration_ms;
  uint16_t flags;
  const pet_anim_frame_t* frames;
} pet_anim_clip_t;

typedef struct pet_anim_player_t {
  uint32_t animation_id;
  uint16_t current_frame_index;
  uint32_t elapsed_in_frame_ms;
  uint16_t loop_counter;
  uint8_t playing;
  uint8_t finished;
  uint16_t flags;
} pet_anim_player_t;

typedef pet_anim_frame_t PetAnimFrame;
typedef pet_anim_clip_t PetAnimClip;
typedef pet_anim_player_t PetAnimPlayer;

pet_result_t pet_anim_clip_validate(const pet_anim_clip_t* clip);
pet_result_t pet_anim_player_init(pet_anim_player_t* player, uint32_t animation_id);
pet_result_t pet_anim_player_tick(pet_anim_player_t* player,
                                  const pet_anim_clip_t* clip,
                                  uint32_t dt_ms);
pet_result_t pet_anim_player_current_frame(const pet_anim_player_t* player,
                                           const pet_anim_clip_t* clip,
                                           const pet_anim_frame_t** out_frame);
pet_result_t pet_anim_player_reset(pet_anim_player_t* player);

#ifdef __cplusplus
}
#endif

#endif  /* PETEGG_PORTABLE_PET_ANIM_H_ */
