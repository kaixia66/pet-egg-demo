#include "mvp_a_platform.h"
#include "mvp_a_save.h"
#include "system/timer.h"
#include "syscfg_id.h"

void mvp_a_platform_init(void)
{
}

u32 mvp_a_platform_get_ms(void)
{
    return timer_get_ms();
}

mvp_a_result_t mvp_a_platform_screen_clear(void *draw_ctx, u16 color)
{
    (void)draw_ctx;
    (void)color;
    return MVP_A_RESULT_OK;
}

mvp_a_result_t mvp_a_platform_screen_fill_rect(void *draw_ctx, const mvp_a_draw_rect_t *rect)
{
    (void)draw_ctx;
    (void)rect;
    return MVP_A_RESULT_OK;
}

mvp_a_result_t mvp_a_platform_screen_draw_text(void *draw_ctx, int x, int y, const char *text, u16 color)
{
    (void)draw_ctx;
    (void)x;
    (void)y;
    (void)text;
    (void)color;
    return MVP_A_RESULT_OK;
}

mvp_a_result_t mvp_a_platform_storage_load(mvp_a_save_data_t *data)
{
    int ret;

    if (!data) {
        return MVP_A_RESULT_INVALID_PARAM;
    }

    ret = syscfg_read(MVP_A_SAVE_VM_ID, data, sizeof(*data));
    if (ret == sizeof(*data)) {
        return MVP_A_RESULT_OK;
    }

    if (ret <= 0) {
        return MVP_A_RESULT_NOT_FOUND;
    }

    return MVP_A_RESULT_STORAGE_ERROR;
}

mvp_a_result_t mvp_a_platform_storage_save(const mvp_a_save_data_t *data)
{
    int ret;

    if (!data) {
        return MVP_A_RESULT_INVALID_PARAM;
    }

    ret = syscfg_write(MVP_A_SAVE_VM_ID, (void *)data, sizeof(*data));
    if (ret == sizeof(*data)) {
        return MVP_A_RESULT_OK;
    }

    return MVP_A_RESULT_STORAGE_ERROR;
}

mvp_a_result_t mvp_a_platform_nfc_start_read(void)
{
    return MVP_A_RESULT_NOT_READY;
}

mvp_a_result_t mvp_a_platform_nfc_poll(mvp_a_nfc_card_t *card)
{
    (void)card;
    return MVP_A_RESULT_NOT_READY;
}

mvp_a_result_t mvp_a_platform_ble_start_pair(void)
{
    return MVP_A_RESULT_NOT_READY;
}

mvp_a_result_t mvp_a_platform_ble_send(const void *data, u16 len)
{
    (void)data;
    (void)len;
    return MVP_A_RESULT_NOT_READY;
}

mvp_a_result_t mvp_a_platform_ble_poll(void *data, u16 *len)
{
    (void)data;
    (void)len;
    return MVP_A_RESULT_NOT_READY;
}
