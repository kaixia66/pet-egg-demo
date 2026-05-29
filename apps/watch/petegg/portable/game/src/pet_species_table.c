#include "pet_species_table.h"

static const pet_species_record_t k_pet_species_table[PET_SPECIES_COUNT] = {
    {PET_SPECIES_QING_LONG, "青龙", PET_ATTRIBUTE_SPIRIT_WOOD, PET_RARITY_LEGENDARY,
     11001u, 11002u, 11003u, 11004u, 11005u, 11006u, 0u, 0u},
    {PET_SPECIES_ZHU_QUE, "朱雀", PET_ATTRIBUTE_BLAZING_FIRE, PET_RARITY_LEGENDARY,
     11011u, 11012u, 11013u, 11014u, 11015u, 11016u, 0u, 0u},
    {PET_SPECIES_XUAN_WU, "玄武", PET_ATTRIBUTE_DARK_WATER, PET_RARITY_LEGENDARY,
     11021u, 11022u, 11023u, 11024u, 11025u, 11026u, 0u, 0u},
    {PET_SPECIES_FROST_WHITE_TIGER, "冰霜白虎", PET_ATTRIBUTE_DARK_WATER,
     PET_RARITY_LEGENDARY, 11031u, 11032u, 11033u, 11034u, 11035u, 11036u, 0u, 0u},
    {PET_SPECIES_QI_LIN, "麒麟", PET_ATTRIBUTE_SPIRIT_WOOD, PET_RARITY_RARE,
     11041u, 11042u, 11043u, 11044u, 11045u, 11046u, 0u, 0u},
    {PET_SPECIES_NINE_TAILED_FOX, "九尾狐", PET_ATTRIBUTE_BLAZING_FIRE, PET_RARITY_RARE,
     11051u, 11052u, 11053u, 11054u, 11055u, 11056u, 0u, 0u},
    {PET_SPECIES_TAO_TIE, "饕餮", PET_ATTRIBUTE_BLAZING_FIRE, PET_RARITY_RARE,
     11061u, 11062u, 11063u, 11064u, 11065u, 11066u, 0u, 0u},
    {PET_SPECIES_QIONG_QI, "穷奇", PET_ATTRIBUTE_BLAZING_FIRE, PET_RARITY_RARE,
     11071u, 11072u, 11073u, 11074u, 11075u, 11076u, 0u, 0u},
    {PET_SPECIES_TAO_WU, "梼杌", PET_ATTRIBUTE_DARK_WATER, PET_RARITY_RARE,
     11081u, 11082u, 11083u, 11084u, 11085u, 11086u, 0u, 0u},
    {PET_SPECIES_HUN_DUN, "混沌", PET_ATTRIBUTE_DARK_WATER, PET_RARITY_RARE,
     11091u, 11092u, 11093u, 11094u, 11095u, 11096u, 0u, 0u},
    {PET_SPECIES_BAI_ZE, "白泽", PET_ATTRIBUTE_SPIRIT_WOOD, PET_RARITY_COMMON,
     11101u, 11102u, 11103u, 11104u, 11105u, 11106u, 0u, 0u},
    {PET_SPECIES_LU_SHU, "鹿蜀", PET_ATTRIBUTE_SPIRIT_WOOD, PET_RARITY_COMMON,
     11111u, 11112u, 11113u, 11114u, 11115u, 11116u, 0u, 0u},
    {PET_SPECIES_QING_NIAO, "青鸟", PET_ATTRIBUTE_SPIRIT_WOOD, PET_RARITY_COMMON,
     11121u, 11122u, 11123u, 11124u, 11125u, 11126u, 0u, 0u},
    {PET_SPECIES_DANG_KANG, "当康", PET_ATTRIBUTE_SPIRIT_WOOD, PET_RARITY_COMMON,
     11131u, 11132u, 11133u, 11134u, 11135u, 11136u, 0u, 0u},
    {PET_SPECIES_BI_FANG, "毕方", PET_ATTRIBUTE_BLAZING_FIRE, PET_RARITY_COMMON,
     11141u, 11142u, 11143u, 11144u, 11145u, 11146u, 0u, 0u},
    {PET_SPECIES_JING_WEI, "精卫", PET_ATTRIBUTE_BLAZING_FIRE, PET_RARITY_COMMON,
     11151u, 11152u, 11153u, 11154u, 11155u, 11156u, 0u, 0u},
    {PET_SPECIES_FU_ZHU, "夫诸", PET_ATTRIBUTE_DARK_WATER, PET_RARITY_COMMON,
     11161u, 11162u, 11163u, 11164u, 11165u, 11166u, 0u, 0u},
    {PET_SPECIES_CHI_RU, "赤鱬", PET_ATTRIBUTE_DARK_WATER, PET_RARITY_COMMON,
     11171u, 11172u, 11173u, 11174u, 11175u, 11176u, 0u, 0u},
};

const pet_species_record_t* pet_species_find(uint16_t species_id) {
  uint16_t i;
  for (i = 0u; i < PET_SPECIES_COUNT; ++i) {
    if (k_pet_species_table[i].species_id == species_id) {
      return &k_pet_species_table[i];
    }
  }
  return 0;
}

const pet_species_record_t* pet_species_at(uint16_t index) {
  if (index >= PET_SPECIES_COUNT) {
    return 0;
  }
  return &k_pet_species_table[index];
}

uint16_t pet_species_count(void) {
  return PET_SPECIES_COUNT;
}

uint8_t pet_species_attribute(uint16_t species_id) {
  const pet_species_record_t* species = pet_species_find(species_id);
  return species != 0 ? species->attribute : PET_ATTRIBUTE_NONE;
}

uint8_t pet_species_rarity(uint16_t species_id) {
  const pet_species_record_t* species = pet_species_find(species_id);
  return species != 0 ? species->rarity : 0u;
}
