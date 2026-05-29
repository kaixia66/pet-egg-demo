#ifndef PETEGG_PORTABLE_PET_SPECIES_TABLE_H_
#define PETEGG_PORTABLE_PET_SPECIES_TABLE_H_

#include "pet_config.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PET_ATTRIBUTE_NONE 0u
#define PET_ATTRIBUTE_SPIRIT_WOOD 1u
#define PET_ATTRIBUTE_BLAZING_FIRE 2u
#define PET_ATTRIBUTE_DARK_WATER 3u

#define PET_RARITY_COMMON 1u
#define PET_RARITY_RARE 2u
#define PET_RARITY_LEGENDARY 3u

#define PET_SPECIES_QING_LONG 1001u
#define PET_SPECIES_ZHU_QUE 1002u
#define PET_SPECIES_XUAN_WU 1003u
#define PET_SPECIES_FROST_WHITE_TIGER 1004u
#define PET_SPECIES_QI_LIN 1005u
#define PET_SPECIES_NINE_TAILED_FOX 1006u
#define PET_SPECIES_TAO_TIE 1007u
#define PET_SPECIES_QIONG_QI 1008u
#define PET_SPECIES_TAO_WU 1009u
#define PET_SPECIES_HUN_DUN 1010u
#define PET_SPECIES_BAI_ZE 1011u
#define PET_SPECIES_LU_SHU 1012u
#define PET_SPECIES_QING_NIAO 1013u
#define PET_SPECIES_DANG_KANG 1014u
#define PET_SPECIES_BI_FANG 1015u
#define PET_SPECIES_JING_WEI 1016u
#define PET_SPECIES_FU_ZHU 1017u
#define PET_SPECIES_CHI_RU 1018u

typedef struct pet_species_record_t {
  uint16_t species_id;
  char display_name[PET_SPECIES_NAME_MAX];
  uint8_t attribute;
  uint8_t rarity;
  uint16_t stage0_asset_group;
  uint16_t stage1_asset_group;
  uint16_t stage2_asset_group;
  uint16_t stage3_asset_group;
  uint16_t stage4_asset_group;
  uint16_t stage5_asset_group;
  uint16_t flags;
  uint16_t reserved;
} pet_species_record_t;

typedef pet_species_record_t PetSpeciesRecord;

const pet_species_record_t* pet_species_find(uint16_t species_id);
const pet_species_record_t* pet_species_at(uint16_t index);
uint16_t pet_species_count(void);
uint8_t pet_species_attribute(uint16_t species_id);
uint8_t pet_species_rarity(uint16_t species_id);

#ifdef __cplusplus
}
#endif

#endif  /* PETEGG_PORTABLE_PET_SPECIES_TABLE_H_ */
