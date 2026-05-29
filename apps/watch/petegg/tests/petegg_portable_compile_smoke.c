#include "pet_anim.h"
#include "pet_app_flow.h"
#include "pet_battle.h"
#include "pet_boss.h"
#include "pet_bt_session.h"
#include "pet_card_activation.h"
#include "pet_care.h"
#include "pet_config.h"
#include "pet_crc32.h"
#include "pet_dirty_rect.h"
#include "pet_display_profile.h"
#include "pet_draw.h"
#include "pet_framebuffer.h"
#include "pet_growth.h"
#include "pet_input.h"
#include "pet_model.h"
#include "pet_nfc_pair_payload.h"
#include "pet_nfc_payload.h"
#include "pet_packet.h"
#include "pet_platform.h"
#include "pet_pool.h"
#include "pet_qte.h"
#include "pet_render_perf.h"
#include "pet_resource_manifest.h"
#include "pet_result.h"
#include "pet_reward.h"
#include "pet_save_format.h"
#include "pet_save_transaction.h"
#include "pet_species_table.h"
#include "pet_sprite.h"
#include "pet_types.h"
#include "pet_unlock_table.h"

#include <string.h>

int petegg_portable_compile_smoke(void) {
  pet_device_save_payload_t save;
  uint8_t pet_index = 0u;
  pet_pet_record_t* active_pet = 0;
  pet_care_request_t care_request;
  pet_care_result_t care_result;
  uint8_t stage_changed = 0u;
  pet_nfc_card_payload_t card_payload;
  pet_nfc_card_validation_result_t validation;
  uint8_t is_duplicate = 1u;
  pet_packet_t packet;
  pet_app_flow_state_t flow;
  uint16_t pixels[16];
  pet_framebuffer_t framebuffer;
  pet_rect_t rect;

  if (pet_species_count() != PET_SPECIES_COUNT || PET_SPECIES_COUNT != PET_MAX_COUNT) {
    return -1;
  }

  memset(&save, 0, sizeof(save));
  save.schema_version = PET_SAVE_SCHEMA_VERSION;
  save.device_id = 1u;
  save.device_short_id = 0x50455431u;
  if (pet_pool_init_empty(&save) != PET_RESULT_OK) {
    return -2;
  }
  if (pet_pool_add_stage0_from_species(&save,
                                       PET_SPECIES_QING_LONG,
                                       10001u,
                                       1u,
                                       &pet_index) != PET_RESULT_OK) {
    return -3;
  }
  if (pet_index != 0u || pet_pool_get_active(&save, &active_pet) != PET_RESULT_OK) {
    return -4;
  }

  memset(&care_request, 0, sizeof(care_request));
  care_request.action_type = PET_CARE_ACTION_FEED_DAILY;
  care_request.now_ms = 1000u;
  care_request.intensity = 1u;
  if (pet_care_preview(active_pet, &care_request, &care_result) != PET_RESULT_OK) {
    return -5;
  }
  if (pet_growth_try_advance_stage(active_pet, &stage_changed, 0, 0) != PET_RESULT_OK) {
    return -6;
  }

  memset(&card_payload, 0, sizeof(card_payload));
  card_payload.uid = 0x1122334455667788ull;
  card_payload.card_type = PET_NFC_CARD_FOOD;
  card_payload.rarity = 1u;
  card_payload.content_id = 2001u;
  card_payload.value = 1u;
  card_payload.flags = PET_NFC_CARD_FLAG_TEST_PAYLOAD;
  card_payload.mock_signature = pet_nfc_card_payload_expected_mock_signature(&card_payload);
  if (pet_nfc_card_payload_validate(&card_payload, &validation) != PET_RESULT_OK) {
    return -7;
  }
  if (pet_card_activation_check_duplicate(&save, card_payload.uid, &is_duplicate) !=
      PET_RESULT_OK) {
    return -8;
  }
  if (is_duplicate != 0u) {
    return -9;
  }

  if (pet_packet_build(PET_PACKET_HELLO, 1u, 0u, 0, 0u, 0u, &packet) != PET_RESULT_OK) {
    return -10;
  }
  if (pet_packet_validate(&packet) != PET_PACKET_STATUS_OK) {
    return -11;
  }

  if (pet_app_flow_init(&flow) != PET_RESULT_OK || flow.current_scene != PET_SCENE_HOME) {
    return -12;
  }

  if (pet_framebuffer_init_rgb565(&framebuffer,
                                  4u,
                                  4u,
                                  4u,
                                  pixels,
                                  (uint32_t)sizeof(pixels)) != PET_RESULT_OK) {
    return -13;
  }
  rect.x = 1;
  rect.y = 1;
  rect.width = 2u;
  rect.height = 2u;
  if (pet_draw_fill_rect(&framebuffer, &rect, 0x07E0u) != PET_RESULT_OK) {
    return -14;
  }

  return 0;
}
