#include "mvp_a_ui_training.h"
#include "mvp_a_lvgl_page.h"
#include "mvp_a_pet.h"
#include "mvp_a_training.h"

#if LVGL_TEST_ENABLE

void mvp_a_ui_training_create(lv_obj_t *parent, const mvp_a_ui_page_t *page)
{
    char detail[64];
    mvp_a_pet_snapshot_t pet;
    mvp_a_training_type_t type = mvp_a_training_get_selected_type();
    mvp_a_lvgl_page_ctx_t ctx;

    mvp_a_pet_get_snapshot(&pet);
    snprintf(detail, sizeof(detail), "Energy %u  %s", pet.energy,
             mvp_a_pet_fast_growth_enabled() ? "Fast" : "Normal");
    mvp_a_lvgl_page_create(parent, page, detail, &ctx);
    mvp_a_lvgl_page_set_visual_text(&ctx, "Train", mvp_a_training_get_type_name(type));
}

#endif
