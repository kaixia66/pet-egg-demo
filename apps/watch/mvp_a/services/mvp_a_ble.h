#ifndef MVP_A_BLE_H
#define MVP_A_BLE_H

#include "mvp_a_def.h"

mvp_a_result_t mvp_a_ble_start_pair(void);
mvp_a_result_t mvp_a_ble_send(const void *data, u16 len);
mvp_a_result_t mvp_a_ble_poll(void *data, u16 *len);

#endif
