#ifndef MVP_A_TRAINING_H
#define MVP_A_TRAINING_H

#include "mvp_a_def.h"

void mvp_a_training_init(void);
mvp_a_training_type_t mvp_a_training_get_selected_type(void);
void mvp_a_training_select_next(void);
void mvp_a_training_select_prev(void);
mvp_a_result_t mvp_a_training_start_selected(void);
const char *mvp_a_training_get_type_name(mvp_a_training_type_t type);
const char *mvp_a_training_get_prompt(void);

#endif
