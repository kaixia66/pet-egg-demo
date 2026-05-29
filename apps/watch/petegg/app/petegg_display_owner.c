#include "pet_app_flow.h"

uint8_t petegg_display_owner_for_scene(uint16_t scene_id) {
  return pet_app_flow_scene_display_owner(scene_id);
}
