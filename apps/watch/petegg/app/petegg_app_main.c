#include "pet_app_flow.h"
#include "petegg_jieli_port.h"

int petegg_app_main_smoke(void) {
  pet_app_flow_state_t flow;

  if (petegg_jieli_port_init() != PETEGG_JIELI_OK) {
    return -1;
  }
  if (pet_app_flow_init(&flow) != PET_RESULT_OK) {
    return -2;
  }
  return (flow.current_scene == PET_SCENE_HOME) ? 0 : -3;
}
