#ifndef MVP_A_BOSS_H
#define MVP_A_BOSS_H

#include "mvp_a_def.h"

mvp_a_result_t mvp_a_boss_unlock(void);
mvp_a_result_t mvp_a_boss_record_win(void);
mvp_a_bool_t mvp_a_boss_is_unlocked(void);
u8 mvp_a_boss_get_win_count(void);

#endif
