#include "mvp_a_ui_growth.h"
#include "mvp_a_assets.h"
#include "mvp_a_home.h"
#include "mvp_a_image_assets.h"
#include "mvp_a_lvgl_page.h"
#include "mvp_a_pet.h"

#if LVGL_TEST_ENABLE

void mvp_a_ui_growth_create(lv_obj_t *parent, const mvp_a_ui_page_t *page)
{
    char detail[64];
    const mvp_a_asset_ref_t *background;
    const mvp_a_asset_ref_t *character;
    const mvp_a_asset_ref_t *component;
    const void *background_img;
    const void *character_img;
    mvp_a_pet_snapshot_t pet;
    mvp_a_lvgl_page_ctx_t ctx;

    mvp_a_pet_get_snapshot(&pet);
    background = mvp_a_assets_background_for_scene(MVP_A_SCENE_GROWTH);
    character = mvp_a_assets_character_for_stage(pet.stage);
    component = mvp_a_assets_ui_component(MVP_A_UI_COMPONENT_PROMPT_NOTICE);
    background_img = mvp_a_image_assets_get(background->id);
    character_img = mvp_a_image_assets_get(character->id);

    snprintf(detail, sizeof(detail), "%s %s %s",
             background->name, character->name, component->name);

    mvp_a_lvgl_page_create(parent, page, detail, &ctx);
    mvp_a_lvgl_page_set_background_image(&ctx, background_img);
    if (character_img) {
        mvp_a_lvgl_page_set_visual_image(&ctx, character_img);
    } else {
        mvp_a_lvgl_page_set_visual_text(&ctx,
                                        mvp_a_home_get_action_name(mvp_a_home_get_selected_action()),
                                        mvp_a_pet_get_stage_name(pet.stage));
    }
}

#endif
