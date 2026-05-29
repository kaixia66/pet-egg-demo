#include "pet_app_flow.h"

#include <string.h>

#define PET_APP_FLOW_HOME_SELECTION_HOME 0u
#define PET_APP_FLOW_HOME_SELECTION_BOSS 1u
#define PET_APP_FLOW_HOME_SELECTION_COUNT 2u
#define PET_APP_FLOW_CARE_ACTION_COUNT 6u
#define PET_APP_FLOW_CARD_BAG_SELECTION_COUNT 4u
#define PET_APP_FLOW_BOSS_ACTION_COUNT 4u
#define PET_APP_FLOW_BATTLE_ACTION_COUNT 4u

static uint8_t pet_app_flow_scene_valid(uint16_t scene_id) {
  return scene_id >= PET_SCENE_HOME && scene_id <= PET_SCENE_ERROR;
}

static uint8_t pet_app_flow_is_boss_scene(uint16_t scene_id) {
  return scene_id == PET_SCENE_BOSS_TOUCH_READY || scene_id == PET_SCENE_BOSS_FRIEND_FOUND ||
         scene_id == PET_SCENE_BOSS_INTRO || scene_id == PET_SCENE_BOSS_ACTION_SELECT ||
         scene_id == PET_SCENE_BOSS_QTE || scene_id == PET_SCENE_BOSS_RESULT;
}

static uint8_t pet_app_flow_is_battle_scene(uint16_t scene_id) {
  return scene_id == PET_SCENE_BATTLE_READY || scene_id == PET_SCENE_BATTLE_ACTION_SELECT ||
         scene_id == PET_SCENE_BATTLE_QTE || scene_id == PET_SCENE_BATTLE_RESULT;
}

static void pet_app_flow_zero_result(pet_app_flow_result_t* result,
                                     const pet_app_flow_state_t* state,
                                     uint8_t status) {
  if (result == 0) {
    return;
  }
  memset(result, 0, sizeof(*result));
  result->status = status;
  if (state != 0) {
    result->old_scene = state->current_scene;
    result->new_scene = state->current_scene;
    result->display_owner_before = state->display_owner;
    result->display_owner_after = state->display_owner;
    result->error_code = state->error_code;
  }
}

static uint8_t pet_app_flow_next_index(uint8_t selected_index, uint8_t count) {
  if (count == 0u) {
    return 0u;
  }
  return (uint8_t)((selected_index + 1u) % count);
}

static uint8_t pet_app_flow_previous_index(uint8_t selected_index, uint8_t count) {
  if (count == 0u) {
    return 0u;
  }
  return selected_index == 0u ? (uint8_t)(count - 1u) : (uint8_t)(selected_index - 1u);
}

static void pet_app_flow_apply_scene(pet_app_flow_state_t* state,
                                     uint16_t scene_id,
                                     pet_app_flow_result_t* result) {
  const uint16_t old_scene = state->current_scene;
  const uint8_t old_owner = state->display_owner;
  if (old_scene != scene_id) {
    state->previous_scene = old_scene;
    state->current_scene = scene_id;
    state->display_owner = pet_app_flow_scene_display_owner(scene_id);
    state->selected_index = 0u;
    state->scene_tick_count = 0u;
    state->scene_enter_count += 1u;
  }
  if (result != 0) {
    result->old_scene = old_scene;
    result->new_scene = state->current_scene;
    result->display_owner_before = old_owner;
    result->display_owner_after = state->display_owner;
    result->transition_happened = old_scene != state->current_scene ? 1u : 0u;
  }
}

static uint8_t pet_app_flow_key_click(const pet_app_flow_event_t* event) {
  return event->event_type == PET_APP_EVENT_KEY && event->key.type == PET_KEY_EVENT_CLICK;
}

static pet_result_t pet_app_flow_handle_home(pet_app_flow_state_t* state,
                                             const pet_app_flow_event_t* event,
                                             pet_app_flow_result_t* result) {
  if (event->event_type == PET_APP_EVENT_LOW_BATTERY) {
    result->action_flags |= PET_APP_ACTION_PLAY_SFX | PET_APP_ACTION_RENDER_REFRESH;
    return PET_RESULT_OK;
  }
  if (pet_app_flow_key_click(event) == 0u) {
    result->status = PET_APP_FLOW_UNSUPPORTED_EVENT;
    return PET_RESULT_UNSUPPORTED;
  }
  switch (event->key.key) {
    case PET_KEY_LEFT_UP:
      state->selected_index =
          pet_app_flow_previous_index(state->selected_index, PET_APP_FLOW_HOME_SELECTION_COUNT);
      result->action_flags |= PET_APP_ACTION_RENDER_REFRESH;
      return PET_RESULT_OK;
    case PET_KEY_RIGHT_DOWN:
      state->selected_index =
          pet_app_flow_next_index(state->selected_index, PET_APP_FLOW_HOME_SELECTION_COUNT);
      result->action_flags |= PET_APP_ACTION_RENDER_REFRESH;
      return PET_RESULT_OK;
    case PET_KEY_OK:
      if (state->selected_index == PET_APP_FLOW_HOME_SELECTION_BOSS) {
        pet_app_flow_apply_scene(state, PET_SCENE_BOSS_TOUCH_READY, result);
      } else {
        pet_app_flow_apply_scene(state, PET_SCENE_CARE_MENU, result);
      }
      result->action_flags |= PET_APP_ACTION_RENDER_REFRESH;
      return PET_RESULT_OK;
    case PET_KEY_CANCEL:
      pet_app_flow_apply_scene(state, PET_SCENE_SETTINGS, result);
      result->action_flags |= PET_APP_ACTION_RENDER_REFRESH;
      return PET_RESULT_OK;
    default:
      break;
  }
  result->status = PET_APP_FLOW_UNSUPPORTED_EVENT;
  return PET_RESULT_UNSUPPORTED;
}

static pet_result_t pet_app_flow_handle_care_menu(pet_app_flow_state_t* state,
                                                  const pet_app_flow_event_t* event,
                                                  pet_app_flow_result_t* result) {
  if (pet_app_flow_key_click(event) == 0u) {
    result->status = PET_APP_FLOW_UNSUPPORTED_EVENT;
    return PET_RESULT_UNSUPPORTED;
  }
  switch (event->key.key) {
    case PET_KEY_LEFT_UP:
      state->selected_index =
          pet_app_flow_previous_index(state->selected_index, PET_APP_FLOW_CARE_ACTION_COUNT);
      result->action_flags |= PET_APP_ACTION_RENDER_REFRESH;
      return PET_RESULT_OK;
    case PET_KEY_RIGHT_DOWN:
      state->selected_index =
          pet_app_flow_next_index(state->selected_index, PET_APP_FLOW_CARE_ACTION_COUNT);
      result->action_flags |= PET_APP_ACTION_RENDER_REFRESH;
      return PET_RESULT_OK;
    case PET_KEY_OK:
      result->action_flags |= PET_APP_ACTION_REQUEST_CARE_APPLY;
      return PET_RESULT_OK;
    case PET_KEY_CANCEL:
      pet_app_flow_apply_scene(state, PET_SCENE_HOME, result);
      result->action_flags |= PET_APP_ACTION_RENDER_REFRESH;
      return PET_RESULT_OK;
    default:
      break;
  }
  result->status = PET_APP_FLOW_UNSUPPORTED_EVENT;
  return PET_RESULT_UNSUPPORTED;
}

static pet_result_t pet_app_flow_handle_card_bag(pet_app_flow_state_t* state,
                                                 const pet_app_flow_event_t* event,
                                                 pet_app_flow_result_t* result) {
  if (pet_app_flow_key_click(event) == 0u) {
    result->status = PET_APP_FLOW_UNSUPPORTED_EVENT;
    return PET_RESULT_UNSUPPORTED;
  }
  switch (event->key.key) {
    case PET_KEY_LEFT_UP:
      state->selected_index =
          pet_app_flow_previous_index(state->selected_index, PET_APP_FLOW_CARD_BAG_SELECTION_COUNT);
      result->action_flags |= PET_APP_ACTION_RENDER_REFRESH;
      return PET_RESULT_OK;
    case PET_KEY_RIGHT_DOWN:
      state->selected_index =
          pet_app_flow_next_index(state->selected_index, PET_APP_FLOW_CARD_BAG_SELECTION_COUNT);
      result->action_flags |= PET_APP_ACTION_RENDER_REFRESH;
      return PET_RESULT_OK;
    case PET_KEY_OK:
      result->action_flags |= PET_APP_ACTION_OPEN_CARD_DETAIL | PET_APP_ACTION_REQUEST_CARD_USE;
      return PET_RESULT_OK;
    case PET_KEY_CANCEL:
      pet_app_flow_apply_scene(state, PET_SCENE_HOME, result);
      result->action_flags |= PET_APP_ACTION_RENDER_REFRESH;
      return PET_RESULT_OK;
    default:
      break;
  }
  result->status = PET_APP_FLOW_UNSUPPORTED_EVENT;
  return PET_RESULT_UNSUPPORTED;
}

static pet_result_t pet_app_flow_handle_nfc_ready(pet_app_flow_state_t* state,
                                                  const pet_app_flow_event_t* event,
                                                  pet_app_flow_result_t* result) {
  if (event->event_type == PET_APP_EVENT_NFC_CARD_DETECTED) {
    pet_app_flow_apply_scene(state, PET_SCENE_NFC_CARD_RESULT, result);
    result->action_flags |= PET_APP_ACTION_REQUEST_CARD_ACTIVATION | PET_APP_ACTION_RENDER_REFRESH;
    return PET_RESULT_OK;
  }
  if (event->event_type == PET_APP_EVENT_ERROR) {
    state->error_code = event->event_param_u16;
    pet_app_flow_apply_scene(state, PET_SCENE_ERROR, result);
    result->action_flags |= PET_APP_ACTION_RENDER_REFRESH;
    return PET_RESULT_OK;
  }
  if (pet_app_flow_key_click(event) != 0u && event->key.key == PET_KEY_CANCEL) {
    pet_app_flow_apply_scene(state, PET_SCENE_HOME, result);
    result->action_flags |= PET_APP_ACTION_RENDER_REFRESH;
    return PET_RESULT_OK;
  }
  result->status = PET_APP_FLOW_UNSUPPORTED_EVENT;
  return PET_RESULT_UNSUPPORTED;
}

static pet_result_t pet_app_flow_handle_result_to_home(pet_app_flow_state_t* state,
                                                       const pet_app_flow_event_t* event,
                                                       pet_app_flow_result_t* result) {
  if (pet_app_flow_key_click(event) != 0u &&
      (event->key.key == PET_KEY_OK || event->key.key == PET_KEY_CANCEL)) {
    pet_app_flow_apply_scene(state, PET_SCENE_HOME, result);
    result->action_flags |= PET_APP_ACTION_RENDER_REFRESH;
    return PET_RESULT_OK;
  }
  result->status = PET_APP_FLOW_UNSUPPORTED_EVENT;
  return PET_RESULT_UNSUPPORTED;
}

static pet_result_t pet_app_flow_handle_boss_touch_ready(pet_app_flow_state_t* state,
                                                         const pet_app_flow_event_t* event,
                                                         pet_app_flow_result_t* result) {
  if (event->event_type == PET_APP_EVENT_NFC_PAIR_SUCCESS) {
    pet_app_flow_apply_scene(state, PET_SCENE_BOSS_FRIEND_FOUND, result);
    result->action_flags |= PET_APP_ACTION_RENDER_REFRESH | PET_APP_ACTION_PLAY_SFX;
    return PET_RESULT_OK;
  }
  if (event->event_type == PET_APP_EVENT_ERROR) {
    state->error_code = event->event_param_u16;
    pet_app_flow_apply_scene(state, PET_SCENE_ERROR, result);
    result->action_flags |= PET_APP_ACTION_RENDER_REFRESH;
    return PET_RESULT_OK;
  }
  if (pet_app_flow_key_click(event) != 0u) {
    if (event->key.key == PET_KEY_CANCEL) {
      pet_app_flow_apply_scene(state, PET_SCENE_HOME, result);
      result->action_flags |= PET_APP_ACTION_RENDER_REFRESH;
      return PET_RESULT_OK;
    }
    if (event->key.key == PET_KEY_OK) {
      result->action_flags |= PET_APP_ACTION_RENDER_REFRESH;
      return PET_RESULT_OK;
    }
  }
  result->status = PET_APP_FLOW_UNSUPPORTED_EVENT;
  return PET_RESULT_UNSUPPORTED;
}

static pet_result_t pet_app_flow_handle_boss_friend_found(pet_app_flow_state_t* state,
                                                          const pet_app_flow_event_t* event,
                                                          pet_app_flow_result_t* result) {
  if (event->event_type == PET_APP_EVENT_BT_DISCONNECTED) {
    state->error_code = PET_APP_EVENT_BT_DISCONNECTED;
    pet_app_flow_apply_scene(state, PET_SCENE_ERROR, result);
    result->action_flags |= PET_APP_ACTION_RENDER_REFRESH;
    return PET_RESULT_OK;
  }
  if (pet_app_flow_key_click(event) != 0u) {
    if (event->key.key == PET_KEY_OK) {
      pet_app_flow_apply_scene(state, PET_SCENE_BOSS_INTRO, result);
      result->action_flags |= PET_APP_ACTION_REQUEST_BOSS_START | PET_APP_ACTION_RENDER_REFRESH;
      return PET_RESULT_OK;
    }
    if (event->key.key == PET_KEY_CANCEL) {
      pet_app_flow_apply_scene(state, PET_SCENE_HOME, result);
      result->action_flags |= PET_APP_ACTION_RENDER_REFRESH;
      return PET_RESULT_OK;
    }
  }
  result->status = PET_APP_FLOW_UNSUPPORTED_EVENT;
  return PET_RESULT_UNSUPPORTED;
}

static pet_result_t pet_app_flow_handle_boss_intro(pet_app_flow_state_t* state,
                                                   const pet_app_flow_event_t* event,
                                                   pet_app_flow_result_t* result) {
  if (pet_app_flow_key_click(event) != 0u) {
    if (event->key.key == PET_KEY_OK) {
      pet_app_flow_apply_scene(state, PET_SCENE_BOSS_ACTION_SELECT, result);
      result->action_flags |= PET_APP_ACTION_RENDER_REFRESH;
      return PET_RESULT_OK;
    }
    if (event->key.key == PET_KEY_CANCEL) {
      pet_app_flow_apply_scene(state, PET_SCENE_HOME, result);
      result->action_flags |= PET_APP_ACTION_RENDER_REFRESH;
      return PET_RESULT_OK;
    }
  }
  result->status = PET_APP_FLOW_UNSUPPORTED_EVENT;
  return PET_RESULT_UNSUPPORTED;
}

static pet_result_t pet_app_flow_handle_action_select(pet_app_flow_state_t* state,
                                                      const pet_app_flow_event_t* event,
                                                      pet_app_flow_result_t* result,
                                                      uint8_t is_boss) {
  if (event->event_type == PET_APP_EVENT_BOSS_ROUND_DONE) {
    pet_app_flow_apply_scene(state, is_boss != 0u ? PET_SCENE_BOSS_QTE : PET_SCENE_BATTLE_QTE,
                             result);
    result->action_flags |= PET_APP_ACTION_RENDER_REFRESH;
    return PET_RESULT_OK;
  }
  if (event->event_type == PET_APP_EVENT_BT_DISCONNECTED || event->event_type == PET_APP_EVENT_ERROR) {
    state->error_code = event->event_type == PET_APP_EVENT_ERROR ? event->event_param_u16
                                                                 : event->event_type;
    pet_app_flow_apply_scene(state, PET_SCENE_ERROR, result);
    result->action_flags |= PET_APP_ACTION_RENDER_REFRESH;
    return PET_RESULT_OK;
  }
  if (pet_app_flow_key_click(event) == 0u) {
    result->status = PET_APP_FLOW_UNSUPPORTED_EVENT;
    return PET_RESULT_UNSUPPORTED;
  }
  switch (event->key.key) {
    case PET_KEY_LEFT_UP:
      state->selected_index = pet_app_flow_previous_index(
          state->selected_index, is_boss != 0u ? PET_APP_FLOW_BOSS_ACTION_COUNT
                                               : PET_APP_FLOW_BATTLE_ACTION_COUNT);
      result->action_flags |= PET_APP_ACTION_RENDER_REFRESH;
      return PET_RESULT_OK;
    case PET_KEY_RIGHT_DOWN:
      state->selected_index = pet_app_flow_next_index(
          state->selected_index, is_boss != 0u ? PET_APP_FLOW_BOSS_ACTION_COUNT
                                               : PET_APP_FLOW_BATTLE_ACTION_COUNT);
      result->action_flags |= PET_APP_ACTION_RENDER_REFRESH;
      return PET_RESULT_OK;
    case PET_KEY_OK:
      result->action_flags |= is_boss != 0u ? PET_APP_ACTION_REQUEST_BOSS_ACTION_SELECT
                                            : PET_APP_ACTION_REQUEST_BATTLE_START;
      return PET_RESULT_OK;
    case PET_KEY_CANCEL:
      pet_app_flow_apply_scene(state, PET_SCENE_HOME, result);
      result->action_flags |= PET_APP_ACTION_RENDER_REFRESH;
      return PET_RESULT_OK;
    default:
      break;
  }
  result->status = PET_APP_FLOW_UNSUPPORTED_EVENT;
  return PET_RESULT_UNSUPPORTED;
}

static pet_result_t pet_app_flow_handle_qte(pet_app_flow_state_t* state,
                                            const pet_app_flow_event_t* event,
                                            pet_app_flow_result_t* result,
                                            uint8_t is_boss) {
  if (event->event_type == PET_APP_EVENT_BOSS_ROUND_DONE) {
    pet_app_flow_apply_scene(state, is_boss != 0u ? PET_SCENE_BOSS_RESULT
                                                  : PET_SCENE_BATTLE_RESULT,
                             result);
    result->action_flags |= PET_APP_ACTION_RENDER_REFRESH;
    return PET_RESULT_OK;
  }
  if (event->event_type == PET_APP_EVENT_BT_DISCONNECTED || event->event_type == PET_APP_EVENT_ERROR) {
    state->error_code = event->event_type == PET_APP_EVENT_ERROR ? event->event_param_u16
                                                                 : event->event_type;
    pet_app_flow_apply_scene(state, PET_SCENE_ERROR, result);
    result->action_flags |= PET_APP_ACTION_RENDER_REFRESH;
    return PET_RESULT_OK;
  }
  if (pet_app_flow_key_click(event) != 0u && event->key.key == PET_KEY_OK) {
    result->action_flags |= PET_APP_ACTION_QTE_HIT;
    return PET_RESULT_OK;
  }
  result->status = PET_APP_FLOW_UNSUPPORTED_EVENT;
  return PET_RESULT_UNSUPPORTED;
}

static pet_result_t pet_app_flow_handle_battle_ready(pet_app_flow_state_t* state,
                                                     const pet_app_flow_event_t* event,
                                                     pet_app_flow_result_t* result) {
  if (pet_app_flow_key_click(event) != 0u) {
    if (event->key.key == PET_KEY_OK) {
      pet_app_flow_apply_scene(state, PET_SCENE_BATTLE_ACTION_SELECT, result);
      result->action_flags |= PET_APP_ACTION_REQUEST_BATTLE_START | PET_APP_ACTION_RENDER_REFRESH;
      return PET_RESULT_OK;
    }
    if (event->key.key == PET_KEY_CANCEL) {
      pet_app_flow_apply_scene(state, PET_SCENE_HOME, result);
      result->action_flags |= PET_APP_ACTION_RENDER_REFRESH;
      return PET_RESULT_OK;
    }
  }
  result->status = PET_APP_FLOW_UNSUPPORTED_EVENT;
  return PET_RESULT_UNSUPPORTED;
}

static pet_result_t pet_app_flow_handle_settings(pet_app_flow_state_t* state,
                                                 const pet_app_flow_event_t* event,
                                                 pet_app_flow_result_t* result) {
  if (pet_app_flow_key_click(event) == 0u) {
    result->status = PET_APP_FLOW_UNSUPPORTED_EVENT;
    return PET_RESULT_UNSUPPORTED;
  }
  if (event->key.key == PET_KEY_OK) {
    result->action_flags |= PET_APP_ACTION_REQUEST_SETTINGS_APPLY;
    return PET_RESULT_OK;
  }
  if (event->key.key == PET_KEY_CANCEL) {
    pet_app_flow_apply_scene(state, state->previous_scene == 0u ? PET_SCENE_HOME
                                                                : state->previous_scene,
                             result);
    result->action_flags |= PET_APP_ACTION_RENDER_REFRESH;
    return PET_RESULT_OK;
  }
  result->status = PET_APP_FLOW_UNSUPPORTED_EVENT;
  return PET_RESULT_UNSUPPORTED;
}

static pet_result_t pet_app_flow_handle_error(pet_app_flow_state_t* state,
                                              const pet_app_flow_event_t* event,
                                              pet_app_flow_result_t* result) {
  if (event->event_type == PET_APP_EVENT_ERROR) {
    state->error_code = event->event_param_u16;
    result->error_code = state->error_code;
    return PET_RESULT_OK;
  }
  if (pet_app_flow_key_click(event) != 0u &&
      (event->key.key == PET_KEY_OK || event->key.key == PET_KEY_CANCEL)) {
    pet_app_flow_apply_scene(state, PET_SCENE_HOME, result);
    result->action_flags |= PET_APP_ACTION_RENDER_REFRESH;
    return PET_RESULT_OK;
  }
  result->status = PET_APP_FLOW_UNSUPPORTED_EVENT;
  return PET_RESULT_UNSUPPORTED;
}

pet_result_t pet_app_flow_init(pet_app_flow_state_t* state) {
  if (state == 0) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  memset(state, 0, sizeof(*state));
  state->current_scene = PET_SCENE_HOME;
  state->previous_scene = PET_SCENE_HOME;
  state->display_owner = PET_DISPLAY_OWNER_PET2D;
  state->scene_enter_count = 1u;
  return PET_RESULT_OK;
}

pet_result_t pet_app_flow_set_scene(pet_app_flow_state_t* state,
                                    uint16_t scene_id,
                                    pet_app_flow_result_t* out_result) {
  if (state == 0) {
    pet_app_flow_zero_result(out_result, 0, PET_APP_FLOW_INVALID_ARG);
    return PET_RESULT_INVALID_ARGUMENT;
  }
  pet_app_flow_zero_result(out_result, state, PET_APP_FLOW_OK);
  if (pet_app_flow_scene_valid(scene_id) == 0u) {
    if (out_result != 0) {
      out_result->status = PET_APP_FLOW_INVALID_SCENE;
    }
    return PET_RESULT_INVALID_ARGUMENT;
  }
  pet_app_flow_apply_scene(state, scene_id, out_result);
  return PET_RESULT_OK;
}

pet_result_t pet_app_flow_dispatch_event(pet_app_flow_state_t* state,
                                         const pet_app_flow_event_t* event,
                                         pet_app_flow_result_t* out_result) {
  pet_result_t result;
  if (state == 0 || event == 0) {
    pet_app_flow_zero_result(out_result, state, PET_APP_FLOW_INVALID_ARG);
    return PET_RESULT_INVALID_ARGUMENT;
  }
  pet_app_flow_zero_result(out_result, state, PET_APP_FLOW_OK);
  if (pet_app_flow_scene_valid(state->current_scene) == 0u) {
    if (out_result != 0) {
      out_result->status = PET_APP_FLOW_INVALID_SCENE;
    }
    return PET_RESULT_INVALID_ARGUMENT;
  }
  state->last_event_type = event->event_type;
  state->last_key = (uint8_t)event->key.key;
  state->scene_tick_count += 1u;

  switch (state->current_scene) {
    case PET_SCENE_HOME:
      result = pet_app_flow_handle_home(state, event, out_result);
      break;
    case PET_SCENE_CARE_MENU:
      result = pet_app_flow_handle_care_menu(state, event, out_result);
      break;
    case PET_SCENE_CARD_BAG:
      result = pet_app_flow_handle_card_bag(state, event, out_result);
      break;
    case PET_SCENE_NFC_CARD_READY:
      result = pet_app_flow_handle_nfc_ready(state, event, out_result);
      break;
    case PET_SCENE_NFC_CARD_RESULT:
    case PET_SCENE_BOSS_RESULT:
    case PET_SCENE_BATTLE_RESULT:
      result = pet_app_flow_handle_result_to_home(state, event, out_result);
      break;
    case PET_SCENE_BOSS_TOUCH_READY:
      result = pet_app_flow_handle_boss_touch_ready(state, event, out_result);
      break;
    case PET_SCENE_BOSS_FRIEND_FOUND:
      result = pet_app_flow_handle_boss_friend_found(state, event, out_result);
      break;
    case PET_SCENE_BOSS_INTRO:
      result = pet_app_flow_handle_boss_intro(state, event, out_result);
      break;
    case PET_SCENE_BOSS_ACTION_SELECT:
      result = pet_app_flow_handle_action_select(state, event, out_result, 1u);
      break;
    case PET_SCENE_BOSS_QTE:
      result = pet_app_flow_handle_qte(state, event, out_result, 1u);
      break;
    case PET_SCENE_BATTLE_READY:
      result = pet_app_flow_handle_battle_ready(state, event, out_result);
      break;
    case PET_SCENE_BATTLE_ACTION_SELECT:
      result = pet_app_flow_handle_action_select(state, event, out_result, 0u);
      break;
    case PET_SCENE_BATTLE_QTE:
      result = pet_app_flow_handle_qte(state, event, out_result, 0u);
      break;
    case PET_SCENE_SETTINGS:
      result = pet_app_flow_handle_settings(state, event, out_result);
      break;
    case PET_SCENE_ERROR:
      result = pet_app_flow_handle_error(state, event, out_result);
      break;
    default:
      if (out_result != 0) {
        out_result->status = PET_APP_FLOW_INVALID_SCENE;
      }
      return PET_RESULT_INVALID_ARGUMENT;
  }

  if (out_result != 0) {
    out_result->new_scene = state->current_scene;
    out_result->display_owner_after = state->display_owner;
    out_result->error_code = state->error_code;
  }
  return result;
}

uint8_t pet_app_flow_scene_display_owner(uint16_t scene_id) {
  if (scene_id == PET_SCENE_HOME || pet_app_flow_is_boss_scene(scene_id) != 0u ||
      pet_app_flow_is_battle_scene(scene_id) != 0u) {
    return PET_DISPLAY_OWNER_PET2D;
  }
  if (scene_id == PET_SCENE_CARE_MENU || scene_id == PET_SCENE_CARD_BAG ||
      scene_id == PET_SCENE_NFC_CARD_READY || scene_id == PET_SCENE_NFC_CARD_RESULT ||
      scene_id == PET_SCENE_SETTINGS || scene_id == PET_SCENE_ERROR) {
    return PET_DISPLAY_OWNER_SYSTEM_UI;
  }
  return PET_DISPLAY_OWNER_NONE;
}

const char* pet_app_flow_scene_name(uint16_t scene_id) {
  switch (scene_id) {
    case PET_SCENE_HOME:
      return "home";
    case PET_SCENE_CARE_MENU:
      return "care_menu";
    case PET_SCENE_CARD_BAG:
      return "card_bag";
    case PET_SCENE_NFC_CARD_READY:
      return "nfc_card_ready";
    case PET_SCENE_NFC_CARD_RESULT:
      return "nfc_card_result";
    case PET_SCENE_BOSS_TOUCH_READY:
      return "boss_touch_ready";
    case PET_SCENE_BOSS_FRIEND_FOUND:
      return "boss_friend_found";
    case PET_SCENE_BOSS_INTRO:
      return "boss_intro";
    case PET_SCENE_BOSS_ACTION_SELECT:
      return "boss_action_select";
    case PET_SCENE_BOSS_QTE:
      return "boss_qte";
    case PET_SCENE_BOSS_RESULT:
      return "boss_result";
    case PET_SCENE_BATTLE_READY:
      return "battle_ready";
    case PET_SCENE_BATTLE_ACTION_SELECT:
      return "battle_action_select";
    case PET_SCENE_BATTLE_QTE:
      return "battle_qte";
    case PET_SCENE_BATTLE_RESULT:
      return "battle_result";
    case PET_SCENE_SETTINGS:
      return "settings";
    case PET_SCENE_ERROR:
      return "error";
    default:
      return "unknown";
  }
}

const char* pet_app_flow_display_owner_name(uint8_t owner) {
  switch (owner) {
    case PET_DISPLAY_OWNER_NONE:
      return "none";
    case PET_DISPLAY_OWNER_SYSTEM_UI:
      return "system_ui";
    case PET_DISPLAY_OWNER_PET2D:
      return "pet2d";
    case PET_DISPLAY_OWNER_NATIVE_TEST:
      return "native_test";
    default:
      return "unknown";
  }
}

const char* pet_app_flow_event_name(uint8_t event_type) {
  switch (event_type) {
    case PET_APP_EVENT_NONE:
      return "none";
    case PET_APP_EVENT_KEY:
      return "key";
    case PET_APP_EVENT_NFC_CARD_DETECTED:
      return "nfc_card_detected";
    case PET_APP_EVENT_NFC_PAIR_SUCCESS:
      return "nfc_pair_success";
    case PET_APP_EVENT_BT_CONNECTED:
      return "bt_connected";
    case PET_APP_EVENT_BT_DISCONNECTED:
      return "bt_disconnected";
    case PET_APP_EVENT_BOSS_STARTED:
      return "boss_started";
    case PET_APP_EVENT_BOSS_ROUND_DONE:
      return "boss_round_done";
    case PET_APP_EVENT_ERROR:
      return "error";
    case PET_APP_EVENT_TIMEOUT:
      return "timeout";
    case PET_APP_EVENT_LOW_BATTERY:
      return "low_battery";
    default:
      return "unknown";
  }
}

const char* pet_app_flow_status_name(uint8_t status) {
  switch (status) {
    case PET_APP_FLOW_OK:
      return "ok";
    case PET_APP_FLOW_INVALID_ARG:
      return "invalid_arg";
    case PET_APP_FLOW_INVALID_SCENE:
      return "invalid_scene";
    case PET_APP_FLOW_TRANSITION_BLOCKED:
      return "transition_blocked";
    case PET_APP_FLOW_UNSUPPORTED_EVENT:
      return "unsupported_event";
    default:
      return "unknown";
  }
}
