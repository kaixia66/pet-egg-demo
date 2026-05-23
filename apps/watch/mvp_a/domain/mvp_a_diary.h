#ifndef MVP_A_DIARY_H
#define MVP_A_DIARY_H

#include "mvp_a_def.h"

mvp_a_result_t mvp_a_diary_mark(u32 flag);
mvp_a_bool_t mvp_a_diary_has(u32 flag);
u32 mvp_a_diary_get_flags(void);
const char *mvp_a_diary_get_summary(void);

#endif
