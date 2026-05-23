#include "mvp_a_ui_card.h"
#include "mvp_a_card.h"
#include "mvp_a_lvgl_page.h"

#if LVGL_TEST_ENABLE

void mvp_a_ui_card_create(lv_obj_t *parent, const mvp_a_ui_page_t *page)
{
    char detail[64];
    u8 count = mvp_a_card_get_count();
    mvp_a_lvgl_page_ctx_t ctx;

    snprintf(detail, sizeof(detail), "Stored %u / %u", count, MVP_A_CARD_MAX);
    mvp_a_lvgl_page_create(parent, page, detail, &ctx);
    mvp_a_lvgl_page_set_visual_text(&ctx, "Card", count ? "Ready" : "Empty");
}

#endif
