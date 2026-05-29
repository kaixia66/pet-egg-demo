#ifndef PETEGG_PORTABLE_PET_MODEL_H_
#define PETEGG_PORTABLE_PET_MODEL_H_

#include "pet_result.h"
#include "pet_save_format.h"
#include "pet_species_table.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Portable pet model helpers operate on in-memory save records only. They do not read
   files, read Flash, call platform services, or implement RPG role/stat systems. */
#define PET_STAGE_0_CORE 0u
#define PET_STAGE_1_BABY_UNKNOWN 1u
#define PET_STAGE_2_ATTRIBUTE_REVEALED 2u
#define PET_STAGE_3_COCOON 3u
#define PET_STAGE_4_SPECIES_BABY 4u
#define PET_STAGE_5_FINAL 5u
#define PET_STAGE_MAX PET_STAGE_5_FINAL

#define PET_STATUS_FLAG_NONE 0u
#define PET_STATUS_FLAG_ACTIVE 1u
#define PET_STATUS_FLAG_SLEEPING 2u
#define PET_STATUS_FLAG_LOCKED 4u
#define PET_STATUS_FLAG_DORMANT 8u
#define PET_STATUS_FLAG_FATIGUED 16u

#define PET_PET_STATUS_ACTIVE PET_STATUS_FLAG_ACTIVE
#define PET_PET_STATUS_SLEEPING PET_STATUS_FLAG_SLEEPING
#define PET_PET_STATUS_LOCKED PET_STATUS_FLAG_LOCKED
#define PET_PET_STATUS_DORMANT PET_STATUS_FLAG_DORMANT
#define PET_PET_STATUS_FATIGUED PET_STATUS_FLAG_FATIGUED

#define PET_MODEL_DEFAULT_CARE_SCORE 50u
#define PET_MODEL_DEFAULT_BOND_SCORE 0u
#define PET_MODEL_DEFAULT_LEVEL_COEFFICIENT_MILLI 1000u
#define PET_MODEL_DOMINANT_FACTOR 420
#define PET_MODEL_SECONDARY_FACTOR 290

pet_result_t pet_model_init_stage0(pet_pet_record_t* pet,
                                   uint16_t species_id,
                                   uint32_t pet_instance_id,
                                   const char* nickname);
pet_result_t pet_model_validate(const pet_pet_record_t* pet);
pet_result_t pet_model_apply_basic_care_delta(pet_pet_record_t* pet,
                                              int16_t care_delta,
                                              int16_t bond_delta);
const char* pet_model_stage_name(uint8_t stage);
const char* pet_model_attribute_name(uint8_t attribute);

#ifdef __cplusplus
}
#endif

#endif  /* PETEGG_PORTABLE_PET_MODEL_H_ */
