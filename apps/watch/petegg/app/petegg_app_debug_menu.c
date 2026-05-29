#include "petegg_jieli_port.h"

int petegg_app_debug_menu_inject_ok_click(void) {
  return petegg_jieli_debug_inject_fake_key(PET_KEY_OK, PET_KEY_EVENT_CLICK);
}
