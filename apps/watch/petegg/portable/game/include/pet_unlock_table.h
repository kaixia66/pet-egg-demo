#ifndef PETEGG_PORTABLE_PET_UNLOCK_TABLE_H_
#define PETEGG_PORTABLE_PET_UNLOCK_TABLE_H_

#include "pet_result.h"
#include "pet_species_table.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* NFC content-card payload content_id is treated as an unlock_id. The unlock table maps
   that small id to local or external-Flash preloaded content; NFC cards do not carry
   resource bytes. */
#define PET_UNLOCK_TYPE_NONE 0u
#define PET_UNLOCK_TYPE_PET_SPECIES 1u
#define PET_UNLOCK_TYPE_FOOD_CARD 2u
#define PET_UNLOCK_TYPE_COMPANION_CARD 3u
#define PET_UNLOCK_TYPE_EQUIPMENT 4u
#define PET_UNLOCK_TYPE_HOME_ASSET 5u

#define PET_UNLOCK_FLAG_DEFAULT 0u

typedef struct pet_unlock_record_t {
  uint16_t unlock_id;
  uint8_t unlock_type;
  uint8_t rarity;
  uint16_t target_id;
  uint16_t value;
  uint16_t resource_group_id;
  uint16_t flags;
} pet_unlock_record_t;

typedef pet_unlock_record_t PetUnlockRecord;

const pet_unlock_record_t* pet_unlock_find(uint16_t unlock_id);
const pet_unlock_record_t* pet_unlock_at(uint16_t index);
uint16_t pet_unlock_count(void);
pet_result_t pet_unlock_validate_record(const pet_unlock_record_t* record);

#ifdef __cplusplus
}
#endif

#endif  /* PETEGG_PORTABLE_PET_UNLOCK_TABLE_H_ */
