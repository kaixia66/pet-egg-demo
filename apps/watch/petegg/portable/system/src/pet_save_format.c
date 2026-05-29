#include "pet_save_format.h"

#include <string.h>

static void pet_write_u8(uint8_t** cursor, uint8_t value) {
  **cursor = value;
  *cursor += 1;
}

static void pet_write_u16(uint8_t** cursor, uint16_t value) {
  pet_write_u8(cursor, (uint8_t)(value & 0xFFu));
  pet_write_u8(cursor, (uint8_t)((value >> 8u) & 0xFFu));
}

static void pet_write_i16(uint8_t** cursor, int16_t value) {
  pet_write_u16(cursor, (uint16_t)value);
}

static void pet_write_u32(uint8_t** cursor, uint32_t value) {
  int i;
  for (i = 0; i < 4; ++i) {
    pet_write_u8(cursor, (uint8_t)((value >> (i * 8)) & 0xFFu));
  }
}

static void pet_write_u64(uint8_t** cursor, uint64_t value) {
  int i;
  for (i = 0; i < 8; ++i) {
    pet_write_u8(cursor, (uint8_t)((value >> (i * 8)) & 0xFFu));
  }
}

static void pet_write_bytes(uint8_t** cursor, const void* bytes, size_t len) {
  memcpy(*cursor, bytes, len);
  *cursor += len;
}

static uint8_t pet_read_u8(const uint8_t** cursor) {
  const uint8_t value = **cursor;
  *cursor += 1;
  return value;
}

static uint16_t pet_read_u16(const uint8_t** cursor) {
  uint16_t value = 0;
  value = (uint16_t)pet_read_u8(cursor);
  value |= (uint16_t)((uint16_t)pet_read_u8(cursor) << 8u);
  return value;
}

static int16_t pet_read_i16(const uint8_t** cursor) {
  return (int16_t)pet_read_u16(cursor);
}

static uint32_t pet_read_u32(const uint8_t** cursor) {
  uint32_t value = 0;
  int i;
  for (i = 0; i < 4; ++i) {
    value |= (uint32_t)pet_read_u8(cursor) << (i * 8);
  }
  return value;
}

static uint64_t pet_read_u64(const uint8_t** cursor) {
  uint64_t value = 0;
  int i;
  for (i = 0; i < 8; ++i) {
    value |= (uint64_t)pet_read_u8(cursor) << (i * 8);
  }
  return value;
}

static void pet_read_bytes(const uint8_t** cursor, void* out_bytes, size_t len) {
  memcpy(out_bytes, *cursor, len);
  *cursor += len;
}

static void pet_write_payload_body(const pet_device_save_payload_t* payload, uint8_t* out_bytes) {
  uint8_t* cursor = out_bytes;
  size_t i;

  pet_write_u16(&cursor, PET_SAVE_SCHEMA_VERSION);
  pet_write_u32(&cursor, payload->device_id);
  pet_write_u32(&cursor, payload->device_short_id);
  pet_write_u8(&cursor, payload->active_pet_index);
  pet_write_u8(&cursor, payload->pet_count);
  pet_write_u16(&cursor, payload->cardbag_summary_count);
  pet_write_u16(&cursor, payload->equipment_summary_count);
  pet_write_u16(&cursor, payload->home_asset_summary_count);
  pet_write_u32(&cursor, payload->battle_count);
  pet_write_u32(&cursor, payload->boss_win_count);
  pet_write_u16(&cursor, payload->gift_record_count);
  pet_write_u8(&cursor, payload->settings.volume);
  pet_write_u8(&cursor, payload->settings.brightness);
  pet_write_u32(&cursor, payload->settings.debug_flags);
  pet_write_u32(&cursor, payload->settings.last_scene_id);
  pet_write_u32(&cursor, payload->reserved0);
  pet_write_u32(&cursor, payload->reserved1);

  for (i = 0; i < PET_MAX_COUNT; ++i) {
    const pet_pet_record_t* pet = &payload->pets[i];
    pet_write_u32(&cursor, pet->pet_id);
    pet_write_u32(&cursor, pet->species_id);
    pet_write_bytes(&cursor, pet->nickname, PET_PET_NAME_MAX);
    pet_write_u8(&cursor, pet->stage);
    pet_write_u8(&cursor, pet->attribute);
    pet_write_u16(&cursor, pet->status);
    pet_write_i16(&cursor, pet->wood_factor);
    pet_write_i16(&cursor, pet->fire_factor);
    pet_write_i16(&cursor, pet->water_factor);
    pet_write_u16(&cursor, pet->care_score);
    pet_write_u16(&cursor, pet->bond_score);
    pet_write_u16(&cursor, pet->level_coefficient_milli);
    pet_write_u32(&cursor, pet->battle_count);
    pet_write_u32(&cursor, pet->boss_win_count);
    pet_write_u32(&cursor, pet->reserved0);
  }

  pet_write_u16(&cursor, payload->activated_nfc_count);
  pet_write_u16(&cursor, payload->virtual_card_count);
  pet_write_u16(&cursor, payload->equipment_count);
  pet_write_u16(&cursor, payload->home_asset_count);
  pet_write_u32(&cursor, payload->reserved2);
  pet_write_u32(&cursor, payload->reserved3);

  for (i = 0; i < PET_ACTIVATED_NFC_MAX; ++i) {
    const pet_activated_nfc_record_t* record = &payload->activated_nfc_records[i];
    pet_write_u64(&cursor, record->uid);
    pet_write_u8(&cursor, record->card_type);
    pet_write_u8(&cursor, record->reserved0);
    pet_write_u16(&cursor, record->content_id);
    pet_write_u32(&cursor, record->activated_counter);
  }

  for (i = 0; i < PET_VIRTUAL_CARD_MAX; ++i) {
    const pet_virtual_card_record_t* record = &payload->virtual_cards[i];
    pet_write_u32(&cursor, record->virtual_card_id);
    pet_write_u64(&cursor, record->source_uid);
    pet_write_u8(&cursor, record->card_type);
    pet_write_u8(&cursor, record->rarity);
    pet_write_u16(&cursor, record->content_id);
    pet_write_u16(&cursor, record->value);
    pet_write_u8(&cursor, record->status);
    pet_write_u8(&cursor, record->reserved0);
  }

  for (i = 0; i < PET_EQUIPMENT_MAX; ++i) {
    const pet_equipment_record_t* record = &payload->equipment[i];
    pet_write_u32(&cursor, record->equipment_id);
    pet_write_u64(&cursor, record->source_uid);
    pet_write_u8(&cursor, record->rarity);
    pet_write_u8(&cursor, record->reserved0);
    pet_write_u16(&cursor, record->content_id);
    pet_write_u16(&cursor, record->value);
    pet_write_u16(&cursor, record->reserved1);
  }

  for (i = 0; i < PET_HOME_ASSET_MAX; ++i) {
    const pet_home_asset_record_t* record = &payload->home_assets[i];
    pet_write_u32(&cursor, record->home_asset_id);
    pet_write_u64(&cursor, record->source_uid);
    pet_write_u8(&cursor, record->rarity);
    pet_write_u8(&cursor, record->reserved0);
    pet_write_u16(&cursor, record->content_id);
    pet_write_u16(&cursor, record->value);
    pet_write_u16(&cursor, record->reserved1);
  }
}

static void pet_read_payload_body(const uint8_t* bytes, pet_device_save_payload_t* out_payload) {
  const uint8_t* cursor = bytes;
  size_t i;
  memset(out_payload, 0, sizeof(*out_payload));

  out_payload->schema_version = pet_read_u16(&cursor);
  out_payload->device_id = pet_read_u32(&cursor);
  out_payload->device_short_id = pet_read_u32(&cursor);
  out_payload->active_pet_index = pet_read_u8(&cursor);
  out_payload->pet_count = pet_read_u8(&cursor);
  out_payload->cardbag_summary_count = pet_read_u16(&cursor);
  out_payload->equipment_summary_count = pet_read_u16(&cursor);
  out_payload->home_asset_summary_count = pet_read_u16(&cursor);
  out_payload->battle_count = pet_read_u32(&cursor);
  out_payload->boss_win_count = pet_read_u32(&cursor);
  out_payload->gift_record_count = pet_read_u16(&cursor);
  out_payload->settings.volume = pet_read_u8(&cursor);
  out_payload->settings.brightness = pet_read_u8(&cursor);
  out_payload->settings.debug_flags = pet_read_u32(&cursor);
  out_payload->settings.last_scene_id = pet_read_u32(&cursor);
  out_payload->reserved0 = pet_read_u32(&cursor);
  out_payload->reserved1 = pet_read_u32(&cursor);

  for (i = 0; i < PET_MAX_COUNT; ++i) {
    pet_pet_record_t* pet = &out_payload->pets[i];
    pet->pet_id = pet_read_u32(&cursor);
    pet->species_id = pet_read_u32(&cursor);
    pet_read_bytes(&cursor, pet->nickname, PET_PET_NAME_MAX);
    pet->stage = pet_read_u8(&cursor);
    pet->attribute = pet_read_u8(&cursor);
    pet->status = pet_read_u16(&cursor);
    pet->wood_factor = pet_read_i16(&cursor);
    pet->fire_factor = pet_read_i16(&cursor);
    pet->water_factor = pet_read_i16(&cursor);
    pet->care_score = pet_read_u16(&cursor);
    pet->bond_score = pet_read_u16(&cursor);
    pet->level_coefficient_milli = pet_read_u16(&cursor);
    pet->battle_count = pet_read_u32(&cursor);
    pet->boss_win_count = pet_read_u32(&cursor);
    pet->reserved0 = pet_read_u32(&cursor);
  }

  out_payload->activated_nfc_count = pet_read_u16(&cursor);
  out_payload->virtual_card_count = pet_read_u16(&cursor);
  out_payload->equipment_count = pet_read_u16(&cursor);
  out_payload->home_asset_count = pet_read_u16(&cursor);
  out_payload->reserved2 = pet_read_u32(&cursor);
  out_payload->reserved3 = pet_read_u32(&cursor);

  for (i = 0; i < PET_ACTIVATED_NFC_MAX; ++i) {
    pet_activated_nfc_record_t* record = &out_payload->activated_nfc_records[i];
    record->uid = pet_read_u64(&cursor);
    record->card_type = pet_read_u8(&cursor);
    record->reserved0 = pet_read_u8(&cursor);
    record->content_id = pet_read_u16(&cursor);
    record->activated_counter = pet_read_u32(&cursor);
  }

  for (i = 0; i < PET_VIRTUAL_CARD_MAX; ++i) {
    pet_virtual_card_record_t* record = &out_payload->virtual_cards[i];
    record->virtual_card_id = pet_read_u32(&cursor);
    record->source_uid = pet_read_u64(&cursor);
    record->card_type = pet_read_u8(&cursor);
    record->rarity = pet_read_u8(&cursor);
    record->content_id = pet_read_u16(&cursor);
    record->value = pet_read_u16(&cursor);
    record->status = pet_read_u8(&cursor);
    record->reserved0 = pet_read_u8(&cursor);
  }

  for (i = 0; i < PET_EQUIPMENT_MAX; ++i) {
    pet_equipment_record_t* record = &out_payload->equipment[i];
    record->equipment_id = pet_read_u32(&cursor);
    record->source_uid = pet_read_u64(&cursor);
    record->rarity = pet_read_u8(&cursor);
    record->reserved0 = pet_read_u8(&cursor);
    record->content_id = pet_read_u16(&cursor);
    record->value = pet_read_u16(&cursor);
    record->reserved1 = pet_read_u16(&cursor);
  }

  for (i = 0; i < PET_HOME_ASSET_MAX; ++i) {
    pet_home_asset_record_t* record = &out_payload->home_assets[i];
    record->home_asset_id = pet_read_u32(&cursor);
    record->source_uid = pet_read_u64(&cursor);
    record->rarity = pet_read_u8(&cursor);
    record->reserved0 = pet_read_u8(&cursor);
    record->content_id = pet_read_u16(&cursor);
    record->value = pet_read_u16(&cursor);
    record->reserved1 = pet_read_u16(&cursor);
  }
}

uint32_t pet_device_save_payload_crc32(const pet_device_save_payload_t* payload) {
  uint8_t bytes[PET_DEVICE_SAVE_PAYLOAD_SERIALIZED_SIZE];
  if (payload == 0) {
    return 0u;
  }
  pet_write_payload_body(payload, bytes);
  return pet_crc32_ieee(bytes, sizeof(bytes));
}

PetResult pet_device_save_payload_serialize(const pet_device_save_payload_t* payload,
                                            uint8_t* out_bytes,
                                            size_t out_capacity,
                                            size_t* out_len) {
  if (out_len != 0) {
    *out_len = PET_DEVICE_SAVE_PAYLOAD_SERIALIZED_SIZE;
  }
  if (payload == 0 || out_bytes == 0) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  if (out_capacity < PET_DEVICE_SAVE_PAYLOAD_SERIALIZED_SIZE) {
    return PET_RESULT_BUFFER_TOO_SMALL;
  }
  pet_write_payload_body(payload, out_bytes);
  return PET_RESULT_OK;
}

PetResult pet_device_save_payload_deserialize(const uint8_t* bytes,
                                              size_t len,
                                              pet_device_save_payload_t* out_payload) {
  if (bytes == 0 || out_payload == 0) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  if (len != PET_DEVICE_SAVE_PAYLOAD_SERIALIZED_SIZE) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  pet_read_payload_body(bytes, out_payload);
  if (out_payload->schema_version < PET_SAVE_SCHEMA_VERSION_MIN_READ ||
      out_payload->schema_version > PET_SAVE_SCHEMA_VERSION) {
    return PET_RESULT_BAD_VERSION;
  }
  if (out_payload->pet_count > PET_MAX_COUNT) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  if (out_payload->pet_count > 0u && out_payload->active_pet_index >= out_payload->pet_count) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  if (out_payload->activated_nfc_count > PET_ACTIVATED_NFC_MAX ||
      out_payload->virtual_card_count > PET_VIRTUAL_CARD_MAX ||
      out_payload->equipment_count > PET_EQUIPMENT_MAX ||
      out_payload->home_asset_count > PET_HOME_ASSET_MAX) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  return PET_RESULT_OK;
}

PetResult pet_save_slot_header_init(pet_save_slot_header_t* header,
                                    uint32_t payload_len,
                                    uint64_t counter,
                                    uint64_t timestamp_sec,
                                    uint32_t payload_crc32) {
  if (header == 0) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  memset(header, 0, sizeof(*header));
  header->magic = PET_SAVE_MAGIC;
  header->version = PET_SAVE_VERSION;
  header->schema_version = PET_SAVE_SCHEMA_VERSION;
  header->payload_type = PET_SAVE_PAYLOAD_DEVICE;
  header->payload_len = payload_len;
  header->counter = counter;
  header->timestamp_sec = timestamp_sec;
  header->crc32 = payload_crc32;
  return PET_RESULT_OK;
}

static void pet_write_header(const pet_save_slot_header_t* header, uint8_t* out_bytes) {
  uint8_t* cursor = out_bytes;
  pet_write_u32(&cursor, header->magic);
  pet_write_u16(&cursor, header->version);
  pet_write_u16(&cursor, header->schema_version);
  pet_write_u16(&cursor, header->payload_type);
  pet_write_u16(&cursor, header->reserved0);
  pet_write_u32(&cursor, header->payload_len);
  pet_write_u64(&cursor, header->counter);
  pet_write_u64(&cursor, header->timestamp_sec);
  pet_write_u32(&cursor, header->crc32);
  pet_write_bytes(&cursor, header->reserved1, sizeof(header->reserved1));
}

static void pet_read_header(const uint8_t* bytes, pet_save_slot_header_t* out_header) {
  const uint8_t* cursor = bytes;
  memset(out_header, 0, sizeof(*out_header));
  out_header->magic = pet_read_u32(&cursor);
  out_header->version = pet_read_u16(&cursor);
  out_header->schema_version = pet_read_u16(&cursor);
  out_header->payload_type = pet_read_u16(&cursor);
  out_header->reserved0 = pet_read_u16(&cursor);
  out_header->payload_len = pet_read_u32(&cursor);
  out_header->counter = pet_read_u64(&cursor);
  out_header->timestamp_sec = pet_read_u64(&cursor);
  out_header->crc32 = pet_read_u32(&cursor);
  pet_read_bytes(&cursor, out_header->reserved1, sizeof(out_header->reserved1));
}

PetResult pet_save_slot_serialize(const pet_device_save_payload_t* payload,
                                  uint64_t counter,
                                  uint64_t timestamp_sec,
                                  uint8_t* out_bytes,
                                  size_t out_capacity,
                                  size_t* out_len) {
  pet_save_slot_header_t header;
  PetResult result;
  uint32_t crc32;

  if (out_len != 0) {
    *out_len = PET_SAVE_SLOT_SERIALIZED_SIZE;
  }
  if (payload == 0 || out_bytes == 0) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  if (out_capacity < PET_SAVE_SLOT_SERIALIZED_SIZE) {
    return PET_RESULT_BUFFER_TOO_SMALL;
  }

  result = pet_device_save_payload_serialize(payload,
                                             out_bytes + PET_SAVE_SLOT_HEADER_SERIALIZED_SIZE,
                                             PET_DEVICE_SAVE_PAYLOAD_SERIALIZED_SIZE,
                                             0);
  if (result != PET_RESULT_OK) {
    return result;
  }
  crc32 = pet_crc32_ieee(out_bytes + PET_SAVE_SLOT_HEADER_SERIALIZED_SIZE,
                         PET_DEVICE_SAVE_PAYLOAD_SERIALIZED_SIZE);
  result = pet_save_slot_header_init(&header,
                                     PET_DEVICE_SAVE_PAYLOAD_SERIALIZED_SIZE,
                                     counter,
                                     timestamp_sec,
                                     crc32);
  if (result != PET_RESULT_OK) {
    return result;
  }
  pet_write_header(&header, out_bytes);
  return PET_RESULT_OK;
}

PetResult pet_save_slot_deserialize(const uint8_t* bytes,
                                    size_t len,
                                    pet_device_save_payload_t* out_payload,
                                    pet_save_slot_header_t* out_header) {
  pet_save_slot_header_t header;
  const uint8_t* payload_bytes;
  uint32_t actual_crc;
  PetResult result;

  if (bytes == 0 || out_payload == 0) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  if (len < PET_SAVE_SLOT_HEADER_SERIALIZED_SIZE) {
    return PET_RESULT_INVALID_ARGUMENT;
  }

  pet_read_header(bytes, &header);
  if (header.magic != PET_SAVE_MAGIC || header.payload_type != PET_SAVE_PAYLOAD_DEVICE) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  if (header.version != PET_SAVE_VERSION ||
      header.schema_version < PET_SAVE_SCHEMA_VERSION_MIN_READ ||
      header.schema_version > PET_SAVE_SCHEMA_VERSION) {
    return PET_RESULT_BAD_VERSION;
  }
  if (header.payload_len != PET_DEVICE_SAVE_PAYLOAD_SERIALIZED_SIZE ||
      len != PET_SAVE_SLOT_HEADER_SERIALIZED_SIZE + (size_t)header.payload_len) {
    return PET_RESULT_INVALID_ARGUMENT;
  }

  payload_bytes = bytes + PET_SAVE_SLOT_HEADER_SERIALIZED_SIZE;
  actual_crc = pet_crc32_ieee(payload_bytes, header.payload_len);
  if (actual_crc != header.crc32) {
    return PET_RESULT_BAD_CRC;
  }
  result = pet_device_save_payload_deserialize(payload_bytes, header.payload_len, out_payload);
  if (result != PET_RESULT_OK) {
    return result;
  }
  if (out_header != 0) {
    *out_header = header;
  }
  return PET_RESULT_OK;
}
