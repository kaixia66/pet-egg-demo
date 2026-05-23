#ifndef MVP_A_UI_DEBUG_H
#define MVP_A_UI_DEBUG_H

#include "app_config.h"
#include "lvgl.h"
#include "mvp_a_def.h"
#include "mvp_a_ui.h"

#if LVGL_TEST_ENABLE
void mvp_a_ui_debug_create(lv_obj_t *parent, const mvp_a_ui_page_t *page);
#endif

#endif
