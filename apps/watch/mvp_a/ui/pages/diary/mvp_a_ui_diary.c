#include "mvp_a_ui_diary.h"
#include "mvp_a_diary.h"
#include "mvp_a_lvgl_page.h"

#if LVGL_TEST_ENABLE

void mvp_a_ui_diary_create(lv_obj_t *parent, const mvp_a_ui_page_t *page)
{
    char detail[64];
    mvp_a_lvgl_page_ctx_t ctx;

    snprintf(detail, sizeof(detail), "Flags 0x%x", mvp_a_diary_get_flags());
    mvp_a_lvgl_page_create(parent, page, detail, &ctx);
    mvp_a_lvgl_page_set_visual_text(&ctx, "Diary", mvp_a_diary_get_summary());
}

#endif
