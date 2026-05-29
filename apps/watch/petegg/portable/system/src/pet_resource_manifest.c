#include "pet_resource_manifest.h"

#include "pet_crc32.h"

static void pet_manifest_write_u8(uint8_t* bytes, uint32_t* offset, uint8_t value) {
  bytes[*offset] = value;
  *offset += 1u;
}

static void pet_manifest_write_u16(uint8_t* bytes, uint32_t* offset, uint16_t value) {
  pet_manifest_write_u8(bytes, offset, (uint8_t)(value & 0xFFu));
  pet_manifest_write_u8(bytes, offset, (uint8_t)((value >> 8u) & 0xFFu));
}

static void pet_manifest_write_u32(uint8_t* bytes, uint32_t* offset, uint32_t value) {
  pet_manifest_write_u8(bytes, offset, (uint8_t)(value & 0xFFu));
  pet_manifest_write_u8(bytes, offset, (uint8_t)((value >> 8u) & 0xFFu));
  pet_manifest_write_u8(bytes, offset, (uint8_t)((value >> 16u) & 0xFFu));
  pet_manifest_write_u8(bytes, offset, (uint8_t)((value >> 24u) & 0xFFu));
}

static void pet_manifest_serialize_entry(const pet_resource_entry_t* entry, uint8_t* bytes) {
  uint32_t offset = 0u;
  pet_manifest_write_u16(bytes, &offset, entry->resource_id);
  pet_manifest_write_u8(bytes, &offset, entry->resource_type);
  pet_manifest_write_u8(bytes, &offset, entry->format);
  pet_manifest_write_u32(bytes, &offset, entry->offset);
  pet_manifest_write_u32(bytes, &offset, entry->size);
  pet_manifest_write_u32(bytes, &offset, entry->crc32);
  pet_manifest_write_u16(bytes, &offset, entry->width);
  pet_manifest_write_u16(bytes, &offset, entry->height);
  pet_manifest_write_u16(bytes, &offset, entry->frame_count);
  pet_manifest_write_u16(bytes, &offset, entry->flags);
  pet_manifest_write_u32(bytes, &offset, entry->reserved);
}

uint32_t pet_resource_manifest_table_crc32(const pet_resource_entry_t* entries,
                                           uint16_t entry_count) {
  uint16_t i;
  uint32_t crc = 0xFFFFFFFFu;
  uint8_t bytes[28];
  if (entries == 0 && entry_count != 0u) {
    return 0u;
  }
  for (i = 0u; i < entry_count; ++i) {
    uint32_t entry_crc;
    pet_manifest_serialize_entry(&entries[i], bytes);
    entry_crc = pet_crc32_ieee(bytes, sizeof(bytes));
    crc ^= entry_crc;
    crc = (crc >> 1u) | (crc << 31u);
  }
  return crc ^ 0xFFFFFFFFu;
}

pet_result_t pet_resource_manifest_validate(const pet_resource_manifest_t* manifest) {
  uint16_t i;
  if (manifest == 0) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  if (manifest->header.magic != PET_RESOURCE_MANIFEST_MAGIC) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  if (manifest->header.version != PET_RESOURCE_MANIFEST_VERSION) {
    return PET_RESULT_BAD_VERSION;
  }
  if (manifest->header.entry_count != 0u && manifest->entries == 0) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  if (manifest->header.table_crc32 !=
      pet_resource_manifest_table_crc32(manifest->entries, manifest->header.entry_count)) {
    return PET_RESULT_BAD_CRC;
  }
  for (i = 0u; i < manifest->header.entry_count; ++i) {
    uint16_t j;
    if (manifest->entries[i].resource_id == 0u ||
        manifest->entries[i].resource_type == PET_RESOURCE_TYPE_NONE) {
      return PET_RESULT_INVALID_ARGUMENT;
    }
    for (j = (uint16_t)(i + 1u); j < manifest->header.entry_count; ++j) {
      if (manifest->entries[i].resource_id == manifest->entries[j].resource_id) {
        return PET_RESULT_DUPLICATE;
      }
    }
  }
  return PET_RESULT_OK;
}

pet_result_t pet_resource_manifest_find(const pet_resource_manifest_t* manifest,
                                        uint16_t resource_id,
                                        const pet_resource_entry_t** out_entry) {
  uint16_t i;
  pet_result_t result;
  if (out_entry == 0) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  *out_entry = 0;
  result = pet_resource_manifest_validate(manifest);
  if (result != PET_RESULT_OK) {
    return result;
  }
  for (i = 0u; i < manifest->header.entry_count; ++i) {
    if (manifest->entries[i].resource_id == resource_id) {
      *out_entry = &manifest->entries[i];
      return PET_RESULT_OK;
    }
  }
  return PET_RESULT_INVALID_ARGUMENT;
}

pet_result_t pet_resource_manifest_find_by_type(const pet_resource_manifest_t* manifest,
                                                uint8_t resource_type,
                                                uint16_t index,
                                                const pet_resource_entry_t** out_entry) {
  uint16_t i;
  uint16_t seen = 0u;
  pet_result_t result;
  if (out_entry == 0) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  *out_entry = 0;
  result = pet_resource_manifest_validate(manifest);
  if (result != PET_RESULT_OK) {
    return result;
  }
  for (i = 0u; i < manifest->header.entry_count; ++i) {
    if (manifest->entries[i].resource_type == resource_type) {
      if (seen == index) {
        *out_entry = &manifest->entries[i];
        return PET_RESULT_OK;
      }
      ++seen;
    }
  }
  return PET_RESULT_INVALID_ARGUMENT;
}
