#include "mvp_a_ui_nfc.h"
#include "mvp_a_card.h"
#include "mvp_a_lvgl_page.h"

#if LVGL_TEST_ENABLE

void mvp_a_ui_nfc_create(lv_obj_t *parent, const mvp_a_ui_page_t *page)
{
    char detail[64];
    mvp_a_lvgl_page_ctx_t ctx;

    snprintf(detail, sizeof(detail), "Card slots %u / %u",
             mvp_a_card_get_count(), MVP_A_CARD_MAX);
    mvp_a_lvgl_page_create(parent, page, detail, &ctx);
    mvp_a_lvgl_page_set_visual_text(&ctx, "NFC", "Reader");
}

#endif
