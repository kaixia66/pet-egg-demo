#include "app_config.h"
#include "lvgl.h"
#include "mvp_a_app.h"
#include "mvp_a_lvgl_shell.h"
#include "mvp_a_ui.h"
#include "mvp_a_ui_boot.h"
#include "mvp_a_ui_boss.h"
#include "mvp_a_ui_card.h"
#include "mvp_a_ui_coop.h"
#include "mvp_a_ui_debug.h"
#include "mvp_a_ui_diary.h"
#include "mvp_a_ui_growth.h"
#include "mvp_a_ui_home.h"
#include "mvp_a_ui_nfc.h"
#include "mvp_a_ui_training.h"

#if LVGL_TEST_ENABLE

static u8 mvp_a_refresh_pending;
static mvp_a_scene_t mvp_a_rendered_scene = MVP_A_SCENE_MAX;

static void mvp_a_lvgl_shell_render_scene(void)
{
    lv_obj_t *scr = lv_scr_act();
    mvp_a_scene_t scene = mvp_a_app_get_scene();
    const mvp_a_ui_page_t *page = mvp_a_ui_get_page(scene);

    if (!scr || !page) {
        return;
    }

    lv_obj_clean(scr);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x050a12), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);

    switch (scene) {
    case MVP_A_SCENE_BOOT:
        mvp_a_ui_boot_create(scr, page);
        break;
    case MVP_A_SCENE_HOME:
        mvp_a_ui_home_create(scr, page);
        break;
    case MVP_A_SCENE_CARE:
        mvp_a_ui_growth_create(scr, page);
        break;
    case MVP_A_SCENE_TRAINING:
        mvp_a_ui_training_create(scr, page);
        break;
    case MVP_A_SCENE_CARD_BAG:
        mvp_a_ui_card_create(scr, page);
        break;
    case MVP_A_SCENE_NFC_READ:
        mvp_a_ui_nfc_create(scr, page);
        break;
    case MVP_A_SCENE_COOP_WAIT:
        mvp_a_ui_coop_create(scr, page);
        break;
    case MVP_A_SCENE_BOSS:
        mvp_a_ui_boss_create(scr, page);
        break;
    case MVP_A_SCENE_DIARY:
        mvp_a_ui_diary_create(scr, page);
        break;
    case MVP_A_SCENE_DEBUG:
        mvp_a_ui_debug_create(scr, page);
        break;
    default:
        mvp_a_ui_home_create(scr, page);
        break;
    }

    mvp_a_rendered_scene = scene;
    lv_obj_invalidate(scr);
    printf("[MVP_A][LVGL] render scene=%d title=%s prompt=%s action=%s\n",
           scene, page->title, page->prompt, page->action);
}

void mvp_a_lvgl_shell_create(void)
{
    printf("[MVP_A][LVGL] shell create start\n");
    mvp_a_app_init();
    mvp_a_app_set_active(MVP_A_TRUE);
    mvp_a_rendered_scene = MVP_A_SCENE_MAX;
    mvp_a_lvgl_shell_render_scene();
}

void mvp_a_lvgl_shell_request_refresh(void)
{
    mvp_a_refresh_pending = 1;
}

void mvp_a_lvgl_shell_tick(void)
{
    if (!mvp_a_refresh_pending &&
        (mvp_a_rendered_scene == mvp_a_app_get_scene())) {
        return;
    }

    mvp_a_refresh_pending = 0;
    mvp_a_lvgl_shell_render_scene();
}

#endif
