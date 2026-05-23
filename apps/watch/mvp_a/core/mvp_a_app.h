#ifndef MVP_A_APP_H
#define MVP_A_APP_H

#include "mvp_a_def.h"

void mvp_a_app_init(void);
void mvp_a_app_deinit(void);
void mvp_a_app_tick(void);
mvp_a_bool_t mvp_a_app_is_active(void);
void mvp_a_app_set_active(mvp_a_bool_t active);
mvp_a_result_t mvp_a_app_key_event(mvp_a_key_t key, mvp_a_key_event_t event);
mvp_a_result_t mvp_a_app_scene_switch(mvp_a_scene_t scene);
mvp_a_scene_t mvp_a_app_get_scene(void);
mvp_a_result_t mvp_a_app_get_last_result(void);

#endif
