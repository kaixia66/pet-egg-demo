#ifndef MVP_A_PLATFORM_H
#define MVP_A_PLATFORM_H

#include "mvp_a_def.h"

typedef struct {
    int x;
    int y;
    int w;
    int h;
    u16 color;
} mvp_a_draw_rect_t;

typedef struct {
    char card_id[MVP_A_CARD_ID_LEN];
    char card_name[MVP_A_CARD_NAME_LEN];
    u8 card_type;
    u8 version;
} mvp_a_nfc_card_t;

void mvp_a_platform_init(void);
u32 mvp_a_platform_get_ms(void);
mvp_a_result_t mvp_a_platform_screen_clear(void *draw_ctx, u16 color);
mvp_a_result_t mvp_a_platform_screen_fill_rect(void *draw_ctx, const mvp_a_draw_rect_t *rect);
mvp_a_result_t mvp_a_platform_screen_draw_text(void *draw_ctx, int x, int y, const char *text, u16 color);
mvp_a_result_t mvp_a_platform_storage_load(mvp_a_save_data_t *data);
mvp_a_result_t mvp_a_platform_storage_save(const mvp_a_save_data_t *data);
mvp_a_result_t mvp_a_platform_nfc_start_read(void);
mvp_a_result_t mvp_a_platform_nfc_poll(mvp_a_nfc_card_t *card);
mvp_a_result_t mvp_a_platform_ble_start_pair(void);
mvp_a_result_t mvp_a_platform_ble_send(const void *data, u16 len);
mvp_a_result_t mvp_a_platform_ble_poll(void *data, u16 *len);

#endif
