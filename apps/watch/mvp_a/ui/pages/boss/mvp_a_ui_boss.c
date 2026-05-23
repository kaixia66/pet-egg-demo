#include "mvp_a_ui_boss.h"
#include "mvp_a_boss.h"
#include "mvp_a_lvgl_page.h"

#if LVGL_TEST_ENABLE

void mvp_a_ui_boss_create(lv_obj_t *parent, const mvp_a_ui_page_t *page)
{
    char detail[64];
    mvp_a_lvgl_page_ctx_t ctx;

    snprintf(detail, sizeof(detail), "Wins %u  %s",
             mvp_a_boss_get_win_count(),
             mvp_a_boss_is_unlocked() ? "Unlocked" : "Locked");
    mvp_a_lvgl_page_create(parent, page, detail, &ctx);
    mvp_a_lvgl_page_set_visual_text(&ctx, "Boss", mvp_a_boss_is_unlocked() ? "Fight" : "Locked");
}

#endif
