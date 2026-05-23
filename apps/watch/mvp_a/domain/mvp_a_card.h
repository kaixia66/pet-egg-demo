#ifndef MVP_A_CARD_H
#define MVP_A_CARD_H

#include "mvp_a_def.h"

mvp_a_result_t mvp_a_card_add(const char *card_id, const char *card_name, mvp_a_card_type_t type);
u8 mvp_a_card_get_count(void);
const mvp_a_card_data_t *mvp_a_card_get(u8 index);
mvp_a_result_t mvp_a_card_clear(void);

#endif
