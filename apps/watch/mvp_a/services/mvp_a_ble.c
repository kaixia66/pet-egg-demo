#include "mvp_a_ble.h"
#include "mvp_a_platform.h"

mvp_a_result_t mvp_a_ble_start_pair(void)
{
    return mvp_a_platform_ble_start_pair();
}

mvp_a_result_t mvp_a_ble_send(const void *data, u16 len)
{
    return mvp_a_platform_ble_send(data, len);
}

mvp_a_result_t mvp_a_ble_poll(void *data, u16 *len)
{
    return mvp_a_platform_ble_poll(data, len);
}
