#ifndef MVP_A_DEBUG_H
#define MVP_A_DEBUG_H

#include "mvp_a_def.h"

typedef enum {
    MVP_A_DEBUG_VIEW_SAVE = 0,
    MVP_A_DEBUG_RESET_SAVE,
    MVP_A_DEBUG_FAST_GROWTH,
    MVP_A_DEBUG_PET2D_SCENE,
    MVP_A_DEBUG_P19_PERF32,
    MVP_A_DEBUG_P19_PERF64,
    MVP_A_DEBUG_P19_PERF96,
    MVP_A_DEBUG_P21_SAVE_AB,
    MVP_A_DEBUG_MAX,
} mvp_a_debug_action_t;

void mvp_a_debug_init(void);
void mvp_a_debug_select_next(void);
void mvp_a_debug_select_prev(void);
mvp_a_debug_action_t mvp_a_debug_get_selected(void);
mvp_a_result_t mvp_a_debug_execute_selected(void);
const char *mvp_a_debug_get_prompt(void);
const char *mvp_a_debug_get_action_name(mvp_a_debug_action_t action);

#endif
