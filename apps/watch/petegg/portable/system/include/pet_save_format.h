#ifndef PETEGG_PORTABLE_PET_SAVE_FORMAT_H_
#define PETEGG_PORTABLE_PET_SAVE_FORMAT_H_

#include "pet_config.h"
#include "pet_crc32.h"
#include "pet_nfc_payload.h"
#include "pet_result.h"
#include "pet_types.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PET_SAVE_MAGIC 0x50455453u
#define PET_SAVE_VERSION 1u
#define PET_SAVE_SCHEMA_VERSION 2u
#define PET_SAVE_SCHEMA_VERSION_MIN_READ 1u
#define PET_SAVE_PAYLOAD_DEVICE 1u
#define PET_SAVE_ENDIAN_LITTLE 1u

#define PET_SAVE_SLOT_HEADER_SERIALIZED_SIZE PET_SAVE_SLOT_HEADER_SIZE
#define PET_DEVICE_SETTINGS_SERIALIZED_SIZE 10u
#define PET_PET_RECORD_SERIALIZED_SIZE 52u
#define PET_ACTIVATED_NFC_RECORD_SERIALIZED_SIZE 16u
#define PET_VIRTUAL_CARD_RECORD_SERIALIZED_SIZE 20u
#define PET_EQUIPMENT_RECORD_SERIALIZED_SIZE 20u
#define PET_HOME_ASSET_RECORD_SERIALIZED_SIZE 20u
#define PET_DEVICE_SAVE_PAYLOAD_PREFIX_SIZE 46u
#define PET_DEVICE_SAVE_PAYLOAD_COUNTS_SIZE 16u
#define PET_DEVICE_SAVE_PAYLOAD_SERIALIZED_SIZE \
  (PET_DEVICE_SAVE_PAYLOAD_PREFIX_SIZE + \
   (PET_MAX_COUNT * PET_PET_RECORD_SERIALIZED_SIZE) + \
   PET_DEVICE_SAVE_PAYLOAD_COUNTS_SIZE + \
   (PET_ACTIVATED_NFC_MAX * PET_ACTIVATED_NFC_RECORD_SERIALIZED_SIZE) + \
   (PET_VIRTUAL_CARD_MAX * PET_VIRTUAL_CARD_RECORD_SERIALIZED_SIZE) + \
   (PET_EQUIPMENT_MAX * PET_EQUIPMENT_RECORD_SERIALIZED_SIZE) + \
   (PET_HOME_ASSET_MAX * PET_HOME_ASSET_RECORD_SERIALIZED_SIZE))
#define PET_SAVE_SLOT_SERIALIZED_SIZE \
  (PET_SAVE_SLOT_HEADER_SERIALIZED_SIZE + PET_DEVICE_SAVE_PAYLOAD_SERIALIZED_SIZE)

#if defined(_MSC_VER)
#pragma pack(push, 1)
#define PET_PACKED_STRUCT
#elif defined(__GNUC__) || defined(__clang__)
#define PET_PACKED_STRUCT __attribute__((packed))
#else
#pragma pack(push, 1)
#define PET_PORTABLE_SAVE_FORMAT_PACK_POP 1
#define PET_PACKED_STRUCT
#endif

typedef enum PetSaveLoadSource {
  PET_SAVE_LOAD_SOURCE_NONE = 0,
  PET_SAVE_LOAD_SOURCE_SLOT_A = 1,
  PET_SAVE_LOAD_SOURCE_SLOT_B = 2,
  PET_SAVE_LOAD_SOURCE_BOOTSTRAP = 3
} PetSaveLoadSource;

typedef struct PET_PACKED_STRUCT pet_save_slot_header_t {
  uint32_t magic;
  uint16_t version;
  uint16_t schema_version;
  uint16_t payload_type;
  uint16_t reserved0;
  uint32_t payload_len;
  uint64_t counter;
  uint64_t timestamp_sec;
  uint32_t crc32;
  uint8_t reserved1[28];
} pet_save_slot_header_t;

typedef struct PET_PACKED_STRUCT pet_save_slot_metadata_t {
  uint8_t slot;
  uint64_t counter;
  uint64_t timestamp_sec;
  uint32_t payload_len;
  uint32_t crc32;
} pet_save_slot_metadata_t;

typedef struct PET_PACKED_STRUCT pet_device_settings_t {
  uint8_t volume;
  uint8_t brightness;
  uint32_t debug_flags;
  uint32_t last_scene_id;
} pet_device_settings_t;

typedef struct PET_PACKED_STRUCT pet_pet_record_t {
  uint32_t pet_id;
  uint32_t species_id;
  char nickname[PET_PET_NAME_MAX];
  uint8_t stage;
  uint8_t attribute;
  uint16_t status;
  int16_t wood_factor;
  int16_t fire_factor;
  int16_t water_factor;
  uint16_t care_score;
  uint16_t bond_score;
  uint16_t level_coefficient_milli;
  uint32_t battle_count;
  uint32_t boss_win_count;
  uint32_t reserved0;
} pet_pet_record_t;

typedef struct PET_PACKED_STRUCT pet_activated_nfc_record_t {
  uint64_t uid;
  uint8_t card_type;
  uint8_t reserved0;
  uint16_t content_id;
  uint32_t activated_counter;
} pet_activated_nfc_record_t;

typedef struct PET_PACKED_STRUCT pet_virtual_card_record_t {
  uint32_t virtual_card_id;
  uint64_t source_uid;
  uint8_t card_type;
  uint8_t rarity;
  uint16_t content_id;
  uint16_t value;
  uint8_t status;
  uint8_t reserved0;
} pet_virtual_card_record_t;

typedef struct PET_PACKED_STRUCT pet_equipment_record_t {
  uint32_t equipment_id;
  uint64_t source_uid;
  uint8_t rarity;
  uint8_t reserved0;
  uint16_t content_id;
  uint16_t value;
  uint16_t reserved1;
} pet_equipment_record_t;

typedef struct PET_PACKED_STRUCT pet_home_asset_record_t {
  uint32_t home_asset_id;
  uint64_t source_uid;
  uint8_t rarity;
  uint8_t reserved0;
  uint16_t content_id;
  uint16_t value;
  uint16_t reserved1;
} pet_home_asset_record_t;

typedef struct PET_PACKED_STRUCT pet_device_save_payload_t {
  uint16_t schema_version;
  uint32_t device_id;
  uint32_t device_short_id;
  uint8_t active_pet_index;
  uint8_t pet_count;
  uint16_t cardbag_summary_count;
  uint16_t equipment_summary_count;
  uint16_t home_asset_summary_count;
  uint32_t battle_count;
  uint32_t boss_win_count;
  uint16_t gift_record_count;
  pet_device_settings_t settings;
  uint32_t reserved0;
  uint32_t reserved1;
  pet_pet_record_t pets[PET_MAX_COUNT];
  uint16_t activated_nfc_count;
  uint16_t virtual_card_count;
  uint16_t equipment_count;
  uint16_t home_asset_count;
  uint32_t reserved2;
  uint32_t reserved3;
  pet_activated_nfc_record_t activated_nfc_records[PET_ACTIVATED_NFC_MAX];
  pet_virtual_card_record_t virtual_cards[PET_VIRTUAL_CARD_MAX];
  pet_equipment_record_t equipment[PET_EQUIPMENT_MAX];
  pet_home_asset_record_t home_assets[PET_HOME_ASSET_MAX];
} pet_device_save_payload_t;

typedef pet_save_slot_header_t PetSaveSlotHeader;
typedef pet_save_slot_metadata_t PetSaveSlotMetadata;
typedef pet_device_settings_t PetSettings;
typedef pet_pet_record_t PetRecord;
typedef pet_activated_nfc_record_t PetActivatedNfcRecord;
typedef pet_virtual_card_record_t PetVirtualCardRecord;
typedef pet_equipment_record_t PetEquipmentRecord;
typedef pet_home_asset_record_t PetHomeAssetRecord;
typedef pet_device_save_payload_t PetDeviceSavePayload;

/* All serialized save numeric fields are little-endian. The slot CRC32 covers only
   the serialized device payload bytes, not the 64-byte slot header. */
uint32_t pet_device_save_payload_crc32(const pet_device_save_payload_t* payload);
PetResult pet_device_save_payload_serialize(const pet_device_save_payload_t* payload,
                                            uint8_t* out_bytes,
                                            size_t out_capacity,
                                            size_t* out_len);
PetResult pet_device_save_payload_deserialize(const uint8_t* bytes,
                                              size_t len,
                                              pet_device_save_payload_t* out_payload);
PetResult pet_save_slot_header_init(pet_save_slot_header_t* header,
                                    uint32_t payload_len,
                                    uint64_t counter,
                                    uint64_t timestamp_sec,
                                    uint32_t payload_crc32);
PetResult pet_save_slot_serialize(const pet_device_save_payload_t* payload,
                                  uint64_t counter,
                                  uint64_t timestamp_sec,
                                  uint8_t* out_bytes,
                                  size_t out_capacity,
                                  size_t* out_len);
PetResult pet_save_slot_deserialize(const uint8_t* bytes,
                                    size_t len,
                                    pet_device_save_payload_t* out_payload,
                                    pet_save_slot_header_t* out_header);

#if defined(_MSC_VER)
#pragma pack(pop)
#elif defined(PET_PORTABLE_SAVE_FORMAT_PACK_POP)
#pragma pack(pop)
#undef PET_PORTABLE_SAVE_FORMAT_PACK_POP
#endif
#undef PET_PACKED_STRUCT

#ifdef __cplusplus
}
#endif

#endif  /* PETEGG_PORTABLE_PET_SAVE_FORMAT_H_ */
