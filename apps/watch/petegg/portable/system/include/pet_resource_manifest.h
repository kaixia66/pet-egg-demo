#ifndef PETEGG_PORTABLE_PET_RESOURCE_MANIFEST_H_
#define PETEGG_PORTABLE_PET_RESOURCE_MANIFEST_H_

#include "pet_config.h"
#include "pet_result.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Portable resource manifest ABI.
   The manifest is a little-endian index for local or external-Flash resources. It does
   not read files, read Flash, or call platform services. */
#define PET_RESOURCE_MANIFEST_MAGIC 0x5054524Du

#define PET_RESOURCE_TYPE_NONE 0u
#define PET_RESOURCE_TYPE_SPRITE 1u
#define PET_RESOURCE_TYPE_ANIMATION 2u
#define PET_RESOURCE_TYPE_ASSET_GROUP 3u
#define PET_RESOURCE_TYPE_CONFIG 4u

#define PET_RESOURCE_FORMAT_NONE 0u
#define PET_RESOURCE_FORMAT_RGB565 1u
#define PET_RESOURCE_FORMAT_ANIMATION_TABLE 2u
#define PET_RESOURCE_FORMAT_GROUP_TABLE 3u
#define PET_RESOURCE_FORMAT_JSON_DEV_PLACEHOLDER 4u

#define PET_RESOURCE_FLAG_LOCAL_FLASH 1u
#define PET_RESOURCE_FLAG_EXTERNAL_FLASH 2u
#define PET_RESOURCE_FLAG_PLACEHOLDER 4u

typedef struct pet_resource_entry_t {
  uint16_t resource_id;
  uint8_t resource_type;
  uint8_t format;
  uint32_t offset;
  uint32_t size;
  uint32_t crc32;
  uint16_t width;
  uint16_t height;
  uint16_t frame_count;
  uint16_t flags;
  uint32_t reserved;
} pet_resource_entry_t;

typedef struct pet_resource_manifest_header_t {
  uint32_t magic;
  uint16_t version;
  uint16_t entry_count;
  uint32_t table_crc32;
  uint32_t reserved;
} pet_resource_manifest_header_t;

typedef struct pet_resource_manifest_t {
  pet_resource_manifest_header_t header;
  const pet_resource_entry_t* entries;
} pet_resource_manifest_t;

typedef struct pet_resource_lookup_result_t {
  uint8_t found;
  uint16_t index;
  const pet_resource_entry_t* entry;
} pet_resource_lookup_result_t;

typedef pet_resource_entry_t PetResourceEntry;
typedef pet_resource_manifest_header_t PetResourceManifestHeader;
typedef pet_resource_manifest_t PetResourceManifest;
typedef pet_resource_lookup_result_t PetResourceLookupResult;

uint32_t pet_resource_manifest_table_crc32(const pet_resource_entry_t* entries,
                                           uint16_t entry_count);
pet_result_t pet_resource_manifest_validate(const pet_resource_manifest_t* manifest);
pet_result_t pet_resource_manifest_find(const pet_resource_manifest_t* manifest,
                                        uint16_t resource_id,
                                        const pet_resource_entry_t** out_entry);
pet_result_t pet_resource_manifest_find_by_type(const pet_resource_manifest_t* manifest,
                                                uint8_t resource_type,
                                                uint16_t index,
                                                const pet_resource_entry_t** out_entry);

#ifdef __cplusplus
}
#endif

#endif  /* PETEGG_PORTABLE_PET_RESOURCE_MANIFEST_H_ */
