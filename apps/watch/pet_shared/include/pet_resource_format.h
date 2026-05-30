#ifndef PET_RESOURCE_FORMAT_H
#define PET_RESOURCE_FORMAT_H

#include "pet_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PET_RESOURCE_MANIFEST_MAGIC  0x5054524Du
#define PET_RESOURCE_MAGIC           PET_RESOURCE_MANIFEST_MAGIC
#define PET_RESOURCE_FORMAT_VERSION  PET_RESOURCE_MANIFEST_VERSION
#define PET_RESOURCE_FILE_MANIFEST   "manifest.bin"
#define PET_RESOURCE_FILE_SPRITES    "sprites.pak"
#define PET_RESOURCE_FILE_ANIM_TABLE "anim_table.bin"
#define PET_RESOURCE_FILE_FONT       "font.bin"
#define PET_RESOURCE_FILE_SFX        "sfx.pak"

typedef enum {
    PET_RESOURCE_TYPE_NONE = 0,
    PET_RESOURCE_TYPE_SPRITE = 1,
    PET_RESOURCE_TYPE_ANIMATION = 2,
    PET_RESOURCE_TYPE_ASSET_GROUP = 3,
    PET_RESOURCE_TYPE_CONFIG = 4
} pet_resource_type_t;

typedef enum {
    PET_RESOURCE_FORMAT_NONE = 0,
    PET_RESOURCE_FORMAT_RGB565 = 1,
    PET_RESOURCE_FORMAT_ANIMATION_TABLE = 2,
    PET_RESOURCE_FORMAT_GROUP_TABLE = 3,
    PET_RESOURCE_FORMAT_JSON_DEV_PLACEHOLDER = 4
} pet_resource_format_t;

typedef enum {
    PET_RESOURCE_FLAG_LOCAL_FLASH = 1u,
    PET_RESOURCE_FLAG_EXTERNAL_FLASH = 2u,
    PET_RESOURCE_FLAG_PLACEHOLDER = 4u
} pet_resource_flag_t;

typedef struct pet_resource_entry_t {
    pet_u16_t resource_id;
    pet_u8_t resource_type;
    pet_u8_t format;
    pet_u32_t offset;
    pet_u32_t size;
    pet_u32_t crc32;
    pet_u16_t width;
    pet_u16_t height;
    pet_u16_t frame_count;
    pet_u16_t flags;
    pet_u32_t reserved;
} pet_resource_entry_t;

typedef struct pet_resource_manifest_header_t {
    pet_u32_t magic;
    pet_u16_t version;
    pet_u16_t entry_count;
    pet_u32_t table_crc32;
    pet_u32_t reserved;
} pet_resource_manifest_header_t;

typedef struct pet_resource_manifest_t {
    pet_resource_manifest_header_t header;
    const pet_resource_entry_t *entries;
} pet_resource_manifest_t;

typedef struct pet_resource_lookup_result_t {
    pet_u8_t found;
    pet_u16_t index;
    const pet_resource_entry_t *entry;
} pet_resource_lookup_result_t;

typedef pet_resource_entry_t PetResourceEntry;
typedef pet_resource_manifest_header_t PetResourceManifestHeader;
typedef pet_resource_manifest_t PetResourceManifest;
typedef pet_resource_lookup_result_t PetResourceLookupResult;

PET_STATIC_ASSERT(resource_entry_size,
                  sizeof(pet_resource_entry_t) == 28u);

#ifdef __cplusplus
}
#endif

#endif
