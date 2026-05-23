#ifndef MVP_A_HOME_H
#define MVP_A_HOME_H

#include "mvp_a_def.h"

void mvp_a_home_init(void);
mvp_a_care_action_t mvp_a_home_get_selected_action(void);
void mvp_a_home_select_next_action(void);
void mvp_a_home_select_prev_action(void);
mvp_a_result_t mvp_a_home_apply_selected_action(void);
const char *mvp_a_home_get_action_name(mvp_a_care_action_t action);
const char *mvp_a_home_get_prompt(void);
const char *mvp_a_home_get_action_prompt(void);

#endif
