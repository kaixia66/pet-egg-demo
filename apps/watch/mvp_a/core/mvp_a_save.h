#ifndef MVP_A_SAVE_H
#define MVP_A_SAVE_H

#include "mvp_a_def.h"

#define MVP_A_SAVE_VM_ID            40

mvp_a_result_t mvp_a_save_init(void);
mvp_a_result_t mvp_a_save_load(void);
mvp_a_result_t mvp_a_save_store(void);
mvp_a_result_t mvp_a_save_reset(void);
void mvp_a_save_make_default(mvp_a_save_data_t *data);
mvp_a_save_data_t *mvp_a_save_get_data(void);
const mvp_a_save_data_t *mvp_a_save_get_const_data(void);
mvp_a_bool_t mvp_a_save_is_loaded(void);
u32 mvp_a_save_checksum(const mvp_a_save_data_t *data);
mvp_a_bool_t mvp_a_save_validate(const mvp_a_save_data_t *data);

#endif
