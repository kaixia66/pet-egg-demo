#include "mvp_a_ui.h"
#include "mvp_a_app.h"
#include "mvp_a_debug.h"
#include "mvp_a_diary.h"
#include "mvp_a_home.h"
#include "mvp_a_pet.h"
#include "mvp_a_training.h"
#include "mvp_a_ui_draw.h"
#include "key_event_deal.h"

static const char *mvp_a_ui_scene_name(mvp_a_scene_t scene)
{
    switch (scene) {
    case MVP_A_SCENE_BOOT:
        return "BOOT";
    case MVP_A_SCENE_HOME:
        return "HOME";
    case MVP_A_SCENE_CARE:
        return "CARE";
    case MVP_A_SCENE_TRAINING:
        return "TRAINING";
    case MVP_A_SCENE_CARD_BAG:
        return "CARD_BAG";
    case MVP_A_SCENE_NFC_READ:
        return "NFC_READ";
    case MVP_A_SCENE_COOP_WAIT:
        return "COOP_WAIT";
    case MVP_A_SCENE_BOSS:
        return "BOSS";
    case MVP_A_SCENE_DIARY:
        return "DIARY";
    case MVP_A_SCENE_DEBUG:
        return "DEBUG";
    default:
        break;
    }

    return "UNKNOWN";
}

static const char *mvp_a_ui_key_name(mvp_a_key_t key)
{
    switch (key) {
    case MVP_A_KEY_CONFIRM:
        return "CONFIRM";
    case MVP_A_KEY_BACK:
        return "BACK";
    case MVP_A_KEY_UP:
        return "UP";
    case MVP_A_KEY_DOWN:
        return "DOWN";
    default:
        break;
    }

    return "UNKNOWN";
}

static const mvp_a_ui_page_t mvp_a_pages[] = {
    {MVP_A_SCENE_BOOT,     "Boot",    "Wake Egg",    "Confirm", 0x18c3, 0xff80},
    {MVP_A_SCENE_HOME,     "Home",    "Qinglong",    "Care",    0x2228, 0x56a4},
    {MVP_A_SCENE_CARE,     "Growth",  "Stage View",  "Grow",    0x1b86, 0x7e8f},
    {MVP_A_SCENE_TRAINING, "Train",   "Courage",     "Start",   0x2985, 0xfde0},
    {MVP_A_SCENE_CARD_BAG, "CardBag", "Cards",       "Open",    0x210d, 0x867f},
    {MVP_A_SCENE_NFC_READ, "NFC",     "Tap Card",    "Read",    0x2148, 0x07ff},
    {MVP_A_SCENE_COOP_WAIT,"Coop",    "Bump Friend", "Wait",    0x190c, 0x7fe0},
    {MVP_A_SCENE_BOSS,     "Boss",    "Rock Spirit", "Fight",   0x28a4, 0xf800},
    {MVP_A_SCENE_DIARY,    "Diary",   "Memory",      "View",    0x2922, 0xffe0},
    {MVP_A_SCENE_DEBUG,    "Debug",   "Dev Tools",   "Test",    0x2104, 0x07e0},
};

static const char *mvp_a_ui_result_text(mvp_a_result_t result)
{
    switch (result) {
    case MVP_A_RESULT_OK:
        return NULL;
    case MVP_A_RESULT_STORAGE_ERROR:
        return "Save Err";
    case MVP_A_RESULT_STORAGE_FULL:
        return "Full";
    case MVP_A_RESULT_NOT_READY:
        return "Not Ready";
    default:
        break;
    }

    return "Err";
}

const mvp_a_ui_page_t *mvp_a_ui_get_page(mvp_a_scene_t scene)
{
    int i;
    static mvp_a_ui_page_t runtime_page;
    const mvp_a_ui_page_t *page = NULL;

    for (i = 0; i < (int)(sizeof(mvp_a_pages) / sizeof(mvp_a_pages[0])); i++) {
        if (mvp_a_pages[i].scene == scene) {
            page = &mvp_a_pages[i];
            break;
        }
    }

    if (!page) {
        page = &mvp_a_pages[1];
    }

    runtime_page = *page;

    switch (scene) {
    case MVP_A_SCENE_BOOT:
        runtime_page.prompt = mvp_a_pet_get_wake_prompt();
        runtime_page.action = "Wake";
        break;
    case MVP_A_SCENE_HOME:
        runtime_page.prompt = mvp_a_home_get_prompt();
        runtime_page.action = "Care";
        break;
    case MVP_A_SCENE_CARE:
        runtime_page.title = "Nest";
        runtime_page.prompt = mvp_a_home_get_action_prompt();
        runtime_page.action = "Do";
        break;
    case MVP_A_SCENE_TRAINING:
        runtime_page.prompt = mvp_a_training_get_prompt();
        runtime_page.action = mvp_a_pet_fast_growth_enabled() ? "FastGo" : "Start";
        break;
    case MVP_A_SCENE_DIARY:
        runtime_page.prompt = mvp_a_diary_get_summary();
        runtime_page.action = "Flags";
        break;
    case MVP_A_SCENE_DEBUG:
        runtime_page.prompt = mvp_a_debug_get_prompt();
        runtime_page.action = mvp_a_debug_get_action_name(mvp_a_debug_get_selected());
        break;
    default:
        break;
    }

    if (mvp_a_ui_result_text(mvp_a_app_get_last_result())) {
        runtime_page.action = mvp_a_ui_result_text(mvp_a_app_get_last_result());
    }

    return &runtime_page;
}

void mvp_a_ui_render(void *draw_ctx)
{
    static mvp_a_scene_t last_scene = MVP_A_SCENE_MAX;
    mvp_a_scene_t scene = mvp_a_app_get_scene();
    const mvp_a_ui_page_t *page = mvp_a_ui_get_page(scene);

    if (last_scene != scene) {
        printf("[MVP_A] render scene=%s(%d) title=%s prompt=%s action=%s\n",
               mvp_a_ui_scene_name(scene), scene, page->title, page->prompt, page->action);
        last_scene = scene;
    }

    mvp_a_ui_draw_page(draw_ctx, page);
}

mvp_a_result_t mvp_a_ui_key_event(mvp_a_key_t key, mvp_a_key_event_t event)
{
    mvp_a_result_t ret;

    printf("[MVP_A] ui key=%s(%d) event=%d scene=%s(%d)\n",
           mvp_a_ui_key_name(key), key, event,
           mvp_a_ui_scene_name(mvp_a_app_get_scene()), mvp_a_app_get_scene());
    ret = mvp_a_app_key_event(key, event);
    printf("[MVP_A] ui key result=%d scene=%s(%d)\n",
           ret, mvp_a_ui_scene_name(mvp_a_app_get_scene()), mvp_a_app_get_scene());

    return ret;
}

mvp_a_bool_t mvp_a_ui_handle_system_key(int key_event)
{
    mvp_a_key_t key;

    switch (key_event) {
    case KEY_CHANGE_MODE:
    case KEY_UI_HOME:
        key = MVP_A_KEY_CONFIRM;
        break;
    case KEY_UI_SHORTCUT:
    case KEY_MUSIC_PP:
    case KEY_CALL_HANG_UP:
        key = MVP_A_KEY_BACK;
        break;
    case KEY_UI_PLUS:
    case KEY_CHANGE_PAGE:
    case KEY_MUSIC_NEXT:
    case KEY_VOL_UP:
        key = MVP_A_KEY_UP;
        break;
    case KEY_UI_MINUS:
    case KEY_MUSIC_PREV:
    case KEY_VOL_DOWN:
        key = MVP_A_KEY_DOWN;
        break;
    default:
        return MVP_A_FALSE;
    }

    printf("[MVP_A] system key=%d mapped=%s(%d)\n",
           key_event, mvp_a_ui_key_name(key), key);
    mvp_a_ui_key_event(key, MVP_A_KEY_EVENT_CLICK);
    return MVP_A_TRUE;
}
