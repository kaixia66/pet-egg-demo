#ifndef PET_SAVE_FORMAT_H
#define PET_SAVE_FORMAT_H

#include "pet_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PET_SAVE_MAGIC              0x50455453u
#define PET_SAVE_VERSION            1u
#define PET_SAVE_FORMAT_VERSION     PET_SAVE_VERSION
#define PET_SAVE_SCHEMA_VERSION     2u
#define PET_SAVE_SCHEMA_VERSION_MIN_READ 1u
#define PET_SAVE_PAYLOAD_DEVICE     1u
#define PET_SAVE_ENDIAN_LITTLE      1u
#define PET_SAVE_SLOT_HEADER_SERIALIZED_SIZE PET_SAVE_SLOT_HEADER_SIZE

typedef enum {
    PET_SAVE_STATUS_EMPTY = 0,
    PET_SAVE_STATUS_VALID,
    PET_SAVE_STATUS_STALE,
    PET_SAVE_STATUS_CORRUPT
} pet_save_status_t;

typedef enum {
    PET_SAVE_TRANSACTION_IDLE = 0,
    PET_SAVE_TRANSACTION_WRITING,
    PET_SAVE_TRANSACTION_COMMITTED,
    PET_SAVE_TRANSACTION_ROLLBACK
} pet_save_transaction_status_t;

typedef enum PetSaveLoadSource {
    PET_SAVE_LOAD_SOURCE_NONE = 0,
    PET_SAVE_LOAD_SOURCE_SLOT_A = 1,
    PET_SAVE_LOAD_SOURCE_SLOT_B = 2,
    PET_SAVE_LOAD_SOURCE_BOOTSTRAP = 3
} PetSaveLoadSource;

PET_PACKED_BEGIN
typedef struct PET_PACKED pet_save_slot_header_t {
    pet_u32_t magic;
    pet_u16_t version;
    pet_u16_t schema_version;
    pet_u16_t payload_type;
    pet_u16_t reserved0;
    pet_u32_t payload_len;
    pet_u64_t counter;
    pet_u64_t timestamp_sec;
    pet_u32_t crc32;
    pet_u8_t reserved1[28];
} pet_save_slot_header_t;
PET_PACKED_END

PET_PACKED_BEGIN
typedef struct PET_PACKED pet_save_slot_metadata_t {
    pet_u8_t slot;
    pet_u64_t counter;
    pet_u64_t timestamp_sec;
    pet_u32_t payload_len;
    pet_u32_t crc32;
} pet_save_slot_metadata_t;
PET_PACKED_END

typedef pet_save_slot_header_t PetSaveSlotHeader;
typedef pet_save_slot_metadata_t PetSaveSlotMetadata;

PET_STATIC_ASSERT(save_slot_header_size,
                  sizeof(pet_save_slot_header_t) == PET_SAVE_SLOT_HEADER_SERIALIZED_SIZE);
PET_STATIC_ASSERT(save_payload_len_offset,
                  offsetof(pet_save_slot_header_t, payload_len) == 12u);
PET_STATIC_ASSERT(save_counter_offset,
                  offsetof(pet_save_slot_header_t, counter) == 16u);
PET_STATIC_ASSERT(save_timestamp_offset,
                  offsetof(pet_save_slot_header_t, timestamp_sec) == 24u);
PET_STATIC_ASSERT(save_crc32_offset,
                  offsetof(pet_save_slot_header_t, crc32) == 32u);

#ifdef __cplusplus
}
#endif

#endif
