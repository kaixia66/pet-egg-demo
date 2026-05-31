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
#include "pet_platform_jieli.h"

#if LVGL_TEST_ENABLE

static u8 mvp_a_refresh_pending;
static mvp_a_scene_t mvp_a_rendered_scene = MVP_A_SCENE_MAX;
static u8 mvp_a_lvgl_owner_acquired;

static mvp_a_bool_t mvp_a_lvgl_shell_acquire_display_owner(void)
{
    const pet_platform_t *platform;
    pet_result_t ret;

    if (mvp_a_lvgl_owner_acquired &&
        (pet_display_jieli_get_owner() == PET_DISPLAY_OWNER_LVGL_SYSTEM_UI)) {
        return MVP_A_TRUE;
    }

    platform = pet_platform_jieli_get();
    if ((platform == 0) || (platform->display_acquire == 0)) {
        printf("[MVP_A][LVGL_OWNER] acquire unavailable\n");
        mvp_a_lvgl_owner_acquired = 0u;
        return MVP_A_FALSE;
    }

    ret = platform->display_acquire(platform->ctx, PET_DISPLAY_OWNER_LVGL_SYSTEM_UI, 0u);
    if (ret != PET_RESULT_OK) {
        printf("[MVP_A][LVGL_OWNER] acquire failed ret=%d owner=%d\n",
               ret, pet_display_jieli_get_owner());
        mvp_a_lvgl_owner_acquired = 0u;
        return MVP_A_FALSE;
    }

    mvp_a_lvgl_owner_acquired = 1u;
    printf("[MVP_A][LVGL_OWNER] acquired owner=%d\n", pet_display_jieli_get_owner());
    return MVP_A_TRUE;
}

static mvp_a_bool_t mvp_a_lvgl_shell_has_display_owner(void)
{
    pet_display_owner_t owner = pet_display_jieli_get_owner();

    if (owner == PET_DISPLAY_OWNER_LVGL_SYSTEM_UI) {
        return MVP_A_TRUE;
    }

    if (mvp_a_lvgl_owner_acquired) {
        printf("[MVP_A][LVGL_OWNER] owner lost current=%d\n", owner);
    }
    mvp_a_lvgl_owner_acquired = 0u;
    return MVP_A_FALSE;
}

static void mvp_a_lvgl_shell_render_scene(void)
{
    lv_obj_t *scr = lv_scr_act();
    mvp_a_scene_t scene = mvp_a_app_get_scene();
    const mvp_a_ui_page_t *page = mvp_a_ui_get_page(scene);

    if (!mvp_a_lvgl_shell_has_display_owner() &&
        !mvp_a_lvgl_shell_acquire_display_owner()) {
        printf("[MVP_A][LVGL_OWNER] render skipped scene=%d owner=%d\n",
               scene, pet_display_jieli_get_owner());
        return;
    }

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
    lv_refr_now(NULL);
    printf("[MVP_A][LVGL] render scene=%d title=%s prompt=%s action=%s\n",
           scene, page->title, page->prompt, page->action);
}

void mvp_a_lvgl_shell_create(void)
{
    printf("[MVP_A][LVGL] shell create start\n");
    mvp_a_app_init();
    mvp_a_app_set_active(MVP_A_TRUE);
    mvp_a_rendered_scene = MVP_A_SCENE_MAX;
    (void)mvp_a_lvgl_shell_acquire_display_owner();
    mvp_a_lvgl_shell_render_scene();
}

void mvp_a_lvgl_shell_request_refresh(void)
{
    mvp_a_refresh_pending = 1;
}

void mvp_a_lvgl_shell_tick(void)
{
    mvp_a_app_tick();

    if (!mvp_a_refresh_pending &&
        (mvp_a_rendered_scene == mvp_a_app_get_scene())) {
        return;
    }

    mvp_a_refresh_pending = 0;
    mvp_a_lvgl_shell_render_scene();
}

void mvp_a_lvgl_shell_release_display_owner(void)
{
    const pet_platform_t *platform = pet_platform_jieli_get();
    pet_display_owner_t owner = pet_display_jieli_get_owner();
    pet_result_t ret;

    if ((platform == 0) || (platform->display_release == 0)) {
        printf("[MVP_A][LVGL_OWNER] release unavailable\n");
        mvp_a_lvgl_owner_acquired = 0u;
        return;
    }

    if (owner != PET_DISPLAY_OWNER_LVGL_SYSTEM_UI) {
        printf("[MVP_A][LVGL_OWNER] release skipped owner=%d\n", owner);
        mvp_a_lvgl_owner_acquired = 0u;
        return;
    }

    ret = platform->display_release(platform->ctx, PET_DISPLAY_OWNER_LVGL_SYSTEM_UI);
    printf("[MVP_A][LVGL_OWNER] release ret=%d owner=%d\n",
           ret, pet_display_jieli_get_owner());
    if (ret == PET_RESULT_OK) {
        mvp_a_lvgl_owner_acquired = 0u;
    }
}

#endif
