#ifndef PETEGG_PORTABLE_PET_APP_FLOW_H_
#define PETEGG_PORTABLE_PET_APP_FLOW_H_

#include "pet_input.h"
#include "pet_result.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Application Flow owns product scene transitions, display ownership, input dispatch, and
   action requests only. It is not Portable System Core or Portable Game Core, and it does
   not call platform, simulator, storage, transport, rendering, or game formula services. */
#define PET_SCENE_HOME 1u
#define PET_SCENE_CARE_MENU 2u
#define PET_SCENE_CARD_BAG 3u
#define PET_SCENE_NFC_CARD_READY 4u
#define PET_SCENE_NFC_CARD_RESULT 5u
#define PET_SCENE_BOSS_TOUCH_READY 6u
#define PET_SCENE_BOSS_FRIEND_FOUND 7u
#define PET_SCENE_BOSS_INTRO 8u
#define PET_SCENE_BOSS_ACTION_SELECT 9u
#define PET_SCENE_BOSS_QTE 10u
#define PET_SCENE_BOSS_RESULT 11u
#define PET_SCENE_BATTLE_READY 12u
#define PET_SCENE_BATTLE_ACTION_SELECT 13u
#define PET_SCENE_BATTLE_QTE 14u
#define PET_SCENE_BATTLE_RESULT 15u
#define PET_SCENE_SETTINGS 16u
#define PET_SCENE_ERROR 17u

#define PET_DISPLAY_OWNER_NONE 0u
#define PET_DISPLAY_OWNER_SYSTEM_UI 1u
#define PET_DISPLAY_OWNER_PET2D 2u
#define PET_DISPLAY_OWNER_NATIVE_TEST 3u

#define PET_APP_EVENT_NONE 0u
#define PET_APP_EVENT_KEY 1u
#define PET_APP_EVENT_NFC_CARD_DETECTED 2u
#define PET_APP_EVENT_NFC_PAIR_SUCCESS 3u
#define PET_APP_EVENT_BT_CONNECTED 4u
#define PET_APP_EVENT_BT_DISCONNECTED 5u
#define PET_APP_EVENT_BOSS_STARTED 6u
#define PET_APP_EVENT_BOSS_ROUND_DONE 7u
#define PET_APP_EVENT_ERROR 8u
#define PET_APP_EVENT_TIMEOUT 9u
#define PET_APP_EVENT_LOW_BATTERY 10u

#define PET_APP_FLOW_OK 0u
#define PET_APP_FLOW_INVALID_ARG 1u
#define PET_APP_FLOW_INVALID_SCENE 2u
#define PET_APP_FLOW_TRANSITION_BLOCKED 3u
#define PET_APP_FLOW_UNSUPPORTED_EVENT 4u

#define PET_APP_ACTION_NONE 0u
#define PET_APP_ACTION_REQUEST_CARE_APPLY (1u << 0)
#define PET_APP_ACTION_REQUEST_CARD_ACTIVATION (1u << 1)
#define PET_APP_ACTION_REQUEST_BOSS_START (1u << 2)
#define PET_APP_ACTION_REQUEST_BOSS_ACTION_SELECT (1u << 3)
#define PET_APP_ACTION_QTE_HIT (1u << 4)
#define PET_APP_ACTION_REQUEST_BATTLE_START (1u << 5)
#define PET_APP_ACTION_REQUEST_SAVE (1u << 6)
#define PET_APP_ACTION_PLAY_SFX (1u << 7)
#define PET_APP_ACTION_RENDER_REFRESH (1u << 8)
#define PET_APP_ACTION_OPEN_CARD_DETAIL (1u << 9)
#define PET_APP_ACTION_REQUEST_CARD_USE (1u << 10)
#define PET_APP_ACTION_REQUEST_SETTINGS_APPLY (1u << 11)

typedef struct pet_app_flow_state_t {
  uint16_t current_scene;
  uint16_t previous_scene;
  uint8_t display_owner;
  uint8_t selected_index;
  uint8_t last_event_type;
  uint8_t last_key;
  uint16_t error_code;
  uint16_t flags;
  uint32_t scene_enter_count;
  uint32_t scene_tick_count;
  uint32_t reserved[4];
} pet_app_flow_state_t;

typedef struct pet_app_flow_event_t {
  uint8_t event_type;
  PetKeyEvent key;
  uint32_t event_param_u32;
  uint16_t event_param_u16;
  uint16_t event_flags;
} pet_app_flow_event_t;

typedef struct pet_app_flow_result_t {
  uint8_t status;
  uint16_t old_scene;
  uint16_t new_scene;
  uint8_t display_owner_before;
  uint8_t display_owner_after;
  uint8_t transition_happened;
  uint32_t action_flags;
  uint16_t error_code;
} pet_app_flow_result_t;

typedef pet_app_flow_state_t PetAppFlowState;
typedef pet_app_flow_event_t PetAppFlowEvent;
typedef pet_app_flow_result_t PetAppFlowResult;

pet_result_t pet_app_flow_init(pet_app_flow_state_t* state);
pet_result_t pet_app_flow_dispatch_event(pet_app_flow_state_t* state,
                                         const pet_app_flow_event_t* event,
                                         pet_app_flow_result_t* out_result);
pet_result_t pet_app_flow_set_scene(pet_app_flow_state_t* state,
                                    uint16_t scene_id,
                                    pet_app_flow_result_t* out_result);
uint8_t pet_app_flow_scene_display_owner(uint16_t scene_id);
const char* pet_app_flow_scene_name(uint16_t scene_id);
const char* pet_app_flow_display_owner_name(uint8_t owner);
const char* pet_app_flow_event_name(uint8_t event_type);
const char* pet_app_flow_status_name(uint8_t status);

#ifdef __cplusplus
}
#endif

#endif  /* PETEGG_PORTABLE_PET_APP_FLOW_H_ */
