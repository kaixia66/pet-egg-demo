#include "pet_care.h"

#include <string.h>

typedef struct pet_care_delta_t {
  int16_t hunger;
  int16_t mood;
  int16_t health;
  int16_t bond;
  int16_t growth;
  int16_t care_score;
  int16_t bond_score;
  int16_t fatigue;
} pet_care_delta_t;

static void pet_care_zero_result(pet_care_result_t* out_result, uint8_t status) {
  if (out_result != 0) {
    memset(out_result, 0, sizeof(*out_result));
    out_result->status = status;
  }
}

static uint16_t pet_care_clamp_score(int32_t value, uint8_t* in_out_clamped) {
  if (value < (int32_t)PET_CARE_SCORE_MIN) {
    if (in_out_clamped != 0) {
      *in_out_clamped = 1u;
    }
    return PET_CARE_SCORE_MIN;
  }
  if (value > (int32_t)PET_CARE_SCORE_MAX) {
    if (in_out_clamped != 0) {
      *in_out_clamped = 1u;
    }
    return PET_CARE_SCORE_MAX;
  }
  return (uint16_t)value;
}

static uint16_t pet_care_clamp_fatigue(int32_t value, uint8_t* in_out_clamped) {
  if (value < (int32_t)PET_CARE_FATIGUE_MIN) {
    if (in_out_clamped != 0) {
      *in_out_clamped = 1u;
    }
    return PET_CARE_FATIGUE_MIN;
  }
  if (value > (int32_t)PET_CARE_FATIGUE_MAX) {
    if (in_out_clamped != 0) {
      *in_out_clamped = 1u;
    }
    return PET_CARE_FATIGUE_MAX;
  }
  return (uint16_t)value;
}

static uint16_t pet_care_request_intensity(const pet_care_request_t* request) {
  if (request->intensity == 0u) {
    return 1u;
  }
  if (request->intensity > 3u) {
    return 3u;
  }
  return request->intensity;
}

static pet_result_t pet_care_action_delta(uint8_t action_type, pet_care_delta_t* out_delta) {
  memset(out_delta, 0, sizeof(*out_delta));
  switch (action_type) {
    case PET_CARE_ACTION_FEED_DAILY:
      out_delta->hunger = 25;
      out_delta->mood = 5;
      out_delta->growth = 10;
      out_delta->care_score = 20;
      out_delta->fatigue = 5;
      return PET_RESULT_OK;
    case PET_CARE_ACTION_PLAY_COMPANION:
      out_delta->mood = 20;
      out_delta->bond = 25;
      out_delta->growth = 12;
      out_delta->bond_score = 20;
      out_delta->fatigue = 10;
      return PET_RESULT_OK;
    case PET_CARE_ACTION_CLEAN_HOME:
      out_delta->health = 20;
      out_delta->growth = 8;
      out_delta->care_score = 15;
      out_delta->fatigue = 3;
      return PET_RESULT_OK;
    case PET_CARE_ACTION_REPAIR_CORE:
      out_delta->health = 25;
      out_delta->mood = 5;
      out_delta->care_score = 25;
      out_delta->fatigue = 8;
      return PET_RESULT_OK;
    case PET_CARE_ACTION_REST:
      out_delta->fatigue = -30;
      return PET_RESULT_OK;
    case PET_CARE_ACTION_WAKE:
      return PET_RESULT_OK;
    default:
      return PET_RESULT_UNSUPPORTED;
  }
}

static int16_t pet_care_scale_delta(int16_t delta, uint16_t intensity, uint8_t limited) {
  int32_t value = (int32_t)delta * (int32_t)intensity;
  if (limited != 0u && value > 0) {
    value = (value + 1) / 2;
  }
  if (value > 32767) {
    return 32767;
  }
  if (value < -32768) {
    return -32768;
  }
  return (int16_t)value;
}

uint16_t pet_care_fatigue(const pet_pet_record_t* pet) {
  if (pet == 0) {
    return 0u;
  }
  return (uint16_t)((pet->reserved0 & PET_GROWTH_FATIGUE_MASK) >>
                    PET_GROWTH_FATIGUE_SHIFT);
}

pet_result_t pet_care_set_fatigue(pet_pet_record_t* pet, uint16_t fatigue) {
  uint16_t clamped = fatigue;
  if (pet == 0) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  if (clamped > PET_CARE_FATIGUE_MAX) {
    clamped = PET_CARE_FATIGUE_MAX;
  }
  pet->reserved0 =
      (pet->reserved0 & PET_GROWTH_PROGRESS_MASK) |
      (((uint32_t)clamped << PET_GROWTH_FATIGUE_SHIFT) & PET_GROWTH_FATIGUE_MASK);
  if (clamped >= PET_CARE_FATIGUE_LIMITED_THRESHOLD) {
    pet->status |= PET_STATUS_FLAG_FATIGUED;
  } else if (clamped < PET_CARE_FATIGUE_CLEAR_THRESHOLD) {
    pet->status = (uint16_t)(pet->status & ~PET_STATUS_FLAG_FATIGUED);
  }
  return PET_RESULT_OK;
}

static pet_result_t pet_care_run(const pet_pet_record_t* pet,
                                 const pet_care_request_t* request,
                                 pet_care_result_t* out_result,
                                 pet_pet_record_t* out_next) {
  pet_care_delta_t base_delta;
  pet_pet_record_t next;
  uint16_t intensity;
  uint16_t old_growth;
  uint16_t old_fatigue;
  uint16_t new_fatigue;
  uint8_t clamped = 0u;
  uint8_t limited = 0u;
  uint8_t stage_changed = 0u;
  uint8_t old_stage;
  uint8_t new_stage;

  pet_care_zero_result(out_result, PET_CARE_STATUS_INVALID_ARG);
  if (pet == 0 || request == 0 || out_result == 0 || out_next == 0 ||
      pet_model_validate(pet) != PET_RESULT_OK) {
    return PET_RESULT_INVALID_ARGUMENT;
  }
  if (pet_care_action_delta(request->action_type, &base_delta) != PET_RESULT_OK) {
    pet_care_zero_result(out_result, PET_CARE_STATUS_INVALID_ACTION);
    return PET_RESULT_UNSUPPORTED;
  }

  next = *pet;
  old_stage = next.stage;
  new_stage = next.stage;
  out_result->status = PET_CARE_STATUS_OK;
  out_result->old_stage = old_stage;
  out_result->new_stage = new_stage;
  old_growth = pet_growth_progress(&next);
  old_fatigue = pet_care_fatigue(&next);

  if (request->action_type == PET_CARE_ACTION_WAKE) {
    if ((next.status & (PET_STATUS_FLAG_SLEEPING | PET_STATUS_FLAG_DORMANT)) == 0u) {
      out_result->status = PET_CARE_STATUS_ALREADY_AWAKE;
      *out_next = next;
      return PET_RESULT_OK;
    }
    next.status =
        (uint16_t)(next.status & ~(PET_STATUS_FLAG_SLEEPING | PET_STATUS_FLAG_DORMANT));
    out_result->flags |= PET_CARE_RESULT_FLAG_WOKE;
    *out_next = next;
    return PET_RESULT_OK;
  }

  limited = (uint8_t)((old_fatigue >= PET_CARE_FATIGUE_LIMITED_THRESHOLD ||
                       (next.status & PET_STATUS_FLAG_FATIGUED) != 0u) &&
                      request->action_type != PET_CARE_ACTION_REST);
  intensity = pet_care_request_intensity(request);

  out_result->hunger_delta = pet_care_scale_delta(base_delta.hunger, intensity, limited);
  out_result->mood_delta = pet_care_scale_delta(base_delta.mood, intensity, limited);
  out_result->health_delta = pet_care_scale_delta(base_delta.health, intensity, limited);
  out_result->bond_delta = pet_care_scale_delta(base_delta.bond, intensity, limited);
  out_result->growth_delta = pet_care_scale_delta(base_delta.growth, intensity, limited);
  out_result->care_score_delta =
      pet_care_scale_delta(base_delta.care_score, intensity, limited);
  out_result->bond_score_delta =
      pet_care_scale_delta(base_delta.bond_score, intensity, limited);
  out_result->fatigue_delta = pet_care_scale_delta(base_delta.fatigue, intensity, 0u);

  next.care_score = pet_care_clamp_score(
      (int32_t)next.care_score + (int32_t)out_result->care_score_delta, &clamped);
  next.bond_score = pet_care_clamp_score(
      (int32_t)next.bond_score + (int32_t)out_result->bond_score_delta, &clamped);
  new_fatigue = pet_care_clamp_fatigue(
      (int32_t)old_fatigue + (int32_t)out_result->fatigue_delta, &clamped);
  pet_care_set_fatigue(&next, new_fatigue);

  if (out_result->growth_delta > 0) {
    if (pet_growth_apply_progress(&next, (uint16_t)out_result->growth_delta,
                                  &stage_changed) != PET_RESULT_OK) {
      pet_care_zero_result(out_result, PET_CARE_STATUS_INVALID_ARG);
      return PET_RESULT_INVALID_ARGUMENT;
    }
  } else {
    pet_growth_set_progress(&next, old_growth);
  }
  new_stage = next.stage;
  out_result->stage_changed = stage_changed;
  out_result->new_stage = new_stage;
  if (stage_changed != 0u) {
    out_result->flags |= PET_CARE_RESULT_FLAG_STAGE_CHANGED;
  }
  if (limited != 0u) {
    out_result->flags |= PET_CARE_RESULT_FLAG_LIMITED_BY_FATIGUE;
    out_result->status = PET_CARE_STATUS_LIMITED_BY_FATIGUE;
  }
  if (clamped != 0u) {
    out_result->flags |= PET_CARE_RESULT_FLAG_CLAMPED;
    if (out_result->status == PET_CARE_STATUS_OK) {
      out_result->status = PET_CARE_STATUS_CLAMPED;
    }
  }
  *out_next = next;
  return PET_RESULT_OK;
}

pet_result_t pet_care_preview(const pet_pet_record_t* pet,
                              const pet_care_request_t* request,
                              pet_care_result_t* out_result) {
  pet_pet_record_t next;
  return pet_care_run(pet, request, out_result, &next);
}

pet_result_t pet_care_apply(pet_pet_record_t* pet,
                            const pet_care_request_t* request,
                            pet_care_result_t* out_result) {
  pet_pet_record_t next;
  pet_result_t result = pet_care_run(pet, request, out_result, &next);
  if (result == PET_RESULT_OK && pet != 0) {
    *pet = next;
  }
  return result;
}

const char* pet_care_action_name(uint8_t action_type) {
  switch (action_type) {
    case PET_CARE_ACTION_FEED_DAILY:
      return "feed_daily";
    case PET_CARE_ACTION_PLAY_COMPANION:
      return "play_companion";
    case PET_CARE_ACTION_CLEAN_HOME:
      return "clean_home";
    case PET_CARE_ACTION_REPAIR_CORE:
      return "repair_core";
    case PET_CARE_ACTION_REST:
      return "rest";
    case PET_CARE_ACTION_WAKE:
      return "wake";
    default:
      return "unknown";
  }
}

const char* pet_care_status_name(uint8_t status) {
  switch (status) {
    case PET_CARE_STATUS_OK:
      return "ok";
    case PET_CARE_STATUS_INVALID_ARG:
      return "invalid_arg";
    case PET_CARE_STATUS_INVALID_ACTION:
      return "invalid_action";
    case PET_CARE_STATUS_LIMITED_BY_FATIGUE:
      return "limited_by_fatigue";
    case PET_CARE_STATUS_ALREADY_AWAKE:
      return "already_awake";
    case PET_CARE_STATUS_NOT_SLEEPING:
      return "not_sleeping";
    case PET_CARE_STATUS_CLAMPED:
      return "clamped";
    default:
      return "unknown";
  }
}
