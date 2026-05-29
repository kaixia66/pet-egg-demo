#ifndef PETEGG_PORTABLE_PET_GROWTH_H_
#define PETEGG_PORTABLE_PET_GROWTH_H_

#include "pet_model.h"
#include "pet_result.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Portable Game Core minimal growth rules. These helpers are deterministic and operate
   only on in-memory pet records. pet_pet_record_t::reserved0 keeps portable-game
   growth/fatigue state without changing the save ABI layout. */
#define PET_GROWTH_PROGRESS_MASK 0x0000FFFFu
#define PET_GROWTH_FATIGUE_MASK 0xFFFF0000u
#define PET_GROWTH_FATIGUE_SHIFT 16u

#define PET_GROWTH_THRESHOLD_STAGE0_TO_1 100u
#define PET_GROWTH_THRESHOLD_STAGE1_TO_2 220u
#define PET_GROWTH_THRESHOLD_STAGE2_TO_3 360u
#define PET_GROWTH_THRESHOLD_STAGE3_TO_4 520u
#define PET_GROWTH_THRESHOLD_STAGE4_TO_5 700u
#define PET_GROWTH_THRESHOLD_STAGE5_FINAL 0u

uint16_t pet_growth_progress(const pet_pet_record_t* pet);
pet_result_t pet_growth_set_progress(pet_pet_record_t* pet, uint16_t progress);
uint16_t pet_growth_stage_threshold(uint8_t stage);
const char* pet_growth_stage_name(uint8_t stage);
pet_result_t pet_growth_can_advance_stage(const pet_pet_record_t* pet,
                                          uint8_t* out_can_advance);
pet_result_t pet_growth_try_advance_stage(pet_pet_record_t* pet,
                                          uint8_t* out_stage_changed,
                                          uint8_t* out_old_stage,
                                          uint8_t* out_new_stage);
pet_result_t pet_growth_apply_progress(pet_pet_record_t* pet,
                                       uint16_t progress_delta,
                                       uint8_t* out_stage_changed);

#ifdef __cplusplus
}
#endif

#endif  /* PETEGG_PORTABLE_PET_GROWTH_H_ */
