#include "mvp_a_ui_coop.h"
#include "mvp_a_boss.h"
#include "mvp_a_lvgl_page.h"
#include "mvp_a_pet.h"

#if LVGL_TEST_ENABLE

void mvp_a_ui_coop_create(lv_obj_t *parent, const mvp_a_ui_page_t *page)
{
    char detail[64];
    mvp_a_pet_snapshot_t pet;
    mvp_a_lvgl_page_ctx_t ctx;

    mvp_a_pet_get_snapshot(&pet);
    snprintf(detail, sizeof(detail), "Stage %u  Boss %s", pet.stage,
             mvp_a_boss_is_unlocked() ? "Ready" : "Wait");
    mvp_a_lvgl_page_create(parent, page, detail, &ctx);
    mvp_a_lvgl_page_set_visual_text(&ctx, "Coop", "Friend");
}

#endif
