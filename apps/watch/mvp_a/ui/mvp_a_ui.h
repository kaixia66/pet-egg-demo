#ifndef MVP_A_UI_H
#define MVP_A_UI_H

#include "mvp_a_def.h"

typedef struct mvp_a_ui_page {
    mvp_a_scene_t scene;
    const char *title;
    const char *prompt;
    const char *action;
    u16 bg_color;
    u16 accent_color;
} mvp_a_ui_page_t;

void mvp_a_ui_render(void *draw_ctx);
mvp_a_result_t mvp_a_ui_key_event(mvp_a_key_t key, mvp_a_key_event_t event);
mvp_a_bool_t mvp_a_ui_handle_system_key(int key_event);
const mvp_a_ui_page_t *mvp_a_ui_get_page(mvp_a_scene_t scene);

#endif
