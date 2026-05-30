#include "mvp_a_app.h"
#include "mvp_a_debug.h"
#include "mvp_a_home.h"
#include "mvp_a_platform.h"
#include "mvp_a_pet.h"
#include "mvp_a_save.h"
#include "mvp_a_training.h"
#include "pet2d_scene.h"

static u8 mvp_a_inited;
static u8 mvp_a_active;
static mvp_a_scene_t mvp_a_scene = MVP_A_SCENE_HOME;
static mvp_a_result_t mvp_a_last_result = MVP_A_RESULT_OK;

static pet_key_t mvp_a_app_map_pet_key(mvp_a_key_t key)
{
    switch (key) {
    case MVP_A_KEY_UP:
        return PET_KEY_LEFT_UP;
    case MVP_A_KEY_DOWN:
        return PET_KEY_RIGHT_DOWN;
    case MVP_A_KEY_CONFIRM:
        return PET_KEY_OK;
    case MVP_A_KEY_BACK:
        return PET_KEY_CANCEL;
    default:
        break;
    }
    return PET_KEY_MAX;
}

static mvp_a_result_t mvp_a_app_pet_result_to_mvp(pet_result_t ret)
{
    switch (ret) {
    case PET_RESULT_OK:
    case PET_RESULT_UNSUPPORTED:
        return MVP_A_RESULT_OK;
    case PET_RESULT_BUSY:
        return MVP_A_RESULT_BUSY;
    case PET_RESULT_NOT_READY:
        return MVP_A_RESULT_NOT_READY;
    case PET_RESULT_NOT_FOUND:
        return MVP_A_RESULT_NOT_FOUND;
    case PET_RESULT_INVALID_ARGUMENT:
        return MVP_A_RESULT_INVALID_PARAM;
    case PET_RESULT_TIMEOUT:
        return MVP_A_RESULT_TIMEOUT;
    default:
        break;
    }
    return MVP_A_RESULT_ERROR;
}

static const char *mvp_a_scene_name(mvp_a_scene_t scene)
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

static mvp_a_scene_t mvp_a_prev_scene(mvp_a_scene_t scene)
{
    if (scene <= MVP_A_SCENE_BOOT) {
        return (mvp_a_scene_t)(MVP_A_SCENE_MAX - 1);
    }

    if ((scene == MVP_A_SCENE_HOME) && mvp_a_pet_first_wake_done()) {
        return (mvp_a_scene_t)(MVP_A_SCENE_MAX - 1);
    }

    return (mvp_a_scene_t)(scene - 1);
}

static mvp_a_scene_t mvp_a_next_scene(mvp_a_scene_t scene)
{
    scene = (mvp_a_scene_t)(scene + 1);
    if (scene >= MVP_A_SCENE_MAX) {
        return mvp_a_pet_first_wake_done() ? MVP_A_SCENE_HOME : MVP_A_SCENE_BOOT;
    }

    return scene;
}

void mvp_a_app_init(void)
{
    mvp_a_result_t ret;

    if (mvp_a_inited) {
        return;
    }

    mvp_a_platform_init();
    ret = mvp_a_save_load();
    mvp_a_last_result = ret;
    printf("[MVP_A] app init save_load=%d\n", ret);
    if (ret == MVP_A_RESULT_OK) {
        mvp_a_pet_init();
    }
    mvp_a_home_init();
    mvp_a_training_init();
    mvp_a_debug_init();
    mvp_a_scene = mvp_a_pet_first_wake_done() ? MVP_A_SCENE_HOME : MVP_A_SCENE_BOOT;
    printf("[MVP_A] app init done scene=%s(%d) first_wake=%d\n",
           mvp_a_scene_name(mvp_a_scene), mvp_a_scene, mvp_a_pet_first_wake_done());
    mvp_a_inited = 1;
}

void mvp_a_app_deinit(void)
{
    mvp_a_active = 0;
    mvp_a_inited = 0;
}

void mvp_a_app_tick(void)
{
    pet_result_t ret;

    if (!mvp_a_inited) {
        return;
    }
    ret = pet2d_scene_tick(mvp_a_platform_get_ms());
    if (ret != PET_RESULT_OK) {
        mvp_a_last_result = mvp_a_app_pet_result_to_mvp(ret);
    }
}

mvp_a_bool_t mvp_a_app_is_active(void)
{
    return mvp_a_active ? MVP_A_TRUE : MVP_A_FALSE;
}

void mvp_a_app_set_active(mvp_a_bool_t active)
{
    mvp_a_active = (active == MVP_A_TRUE);
}

mvp_a_result_t mvp_a_app_key_event(mvp_a_key_t key, mvp_a_key_event_t event)
{
    mvp_a_result_t ret;
    pet_key_event_t pet_event;
    pet_result_t pet_ret;

    if (!mvp_a_inited) {
        mvp_a_app_init();
    }

    if ((key >= MVP_A_KEY_MAX) || (event >= MVP_A_KEY_EVENT_MAX)) {
        return MVP_A_RESULT_INVALID_PARAM;
    }

    if (pet2d_scene_is_active()) {
        pet_event.key = mvp_a_app_map_pet_key(key);
        pet_event.type = (event == MVP_A_KEY_EVENT_CLICK) ? PET_KEY_EVENT_CLICK : PET_KEY_EVENT_UP;
        pet_event.timestamp_ms = mvp_a_platform_get_ms();
        pet_event.hold_ms = 0u;
        pet_event.repeat_count = 0u;
        pet_event.raw_code = 0u;
        if (pet_event.key == PET_KEY_MAX) {
            return MVP_A_RESULT_INVALID_PARAM;
        }
        pet_ret = pet2d_scene_handle_key(&pet_event);
        ret = mvp_a_app_pet_result_to_mvp(pet_ret);
        mvp_a_last_result = ret;
        return ret;
    }

    if (event != MVP_A_KEY_EVENT_CLICK) {
        return MVP_A_RESULT_OK;
    }

    printf("[MVP_A] app key=%d event=%d scene_before=%s(%d)\n",
           key, event, mvp_a_scene_name(mvp_a_scene), mvp_a_scene);

    switch (mvp_a_scene) {
    case MVP_A_SCENE_BOOT:
        if (key == MVP_A_KEY_CONFIRM) {
            ret = mvp_a_pet_first_wake_next();
            if (ret == MVP_A_RESULT_OK) {
                ret = mvp_a_app_scene_switch(MVP_A_SCENE_HOME);
            }
            mvp_a_last_result = ret;
            return ret;
        }
        return MVP_A_RESULT_OK;
    case MVP_A_SCENE_HOME:
        if (key == MVP_A_KEY_CONFIRM) {
            ret = mvp_a_app_scene_switch(MVP_A_SCENE_CARE);
            mvp_a_last_result = ret;
            return ret;
        }
        break;
    case MVP_A_SCENE_CARE:
        if (key == MVP_A_KEY_UP) {
            mvp_a_home_select_prev_action();
            mvp_a_last_result = MVP_A_RESULT_OK;
            return MVP_A_RESULT_OK;
        }
        if (key == MVP_A_KEY_DOWN) {
            mvp_a_home_select_next_action();
            mvp_a_last_result = MVP_A_RESULT_OK;
            return MVP_A_RESULT_OK;
        }
        if (key == MVP_A_KEY_CONFIRM) {
            ret = mvp_a_home_apply_selected_action();
            mvp_a_last_result = ret;
            return ret;
        }
        if (key == MVP_A_KEY_BACK) {
            ret = mvp_a_app_scene_switch(MVP_A_SCENE_HOME);
            mvp_a_last_result = ret;
            return ret;
        }
        break;
    case MVP_A_SCENE_TRAINING:
        if (key == MVP_A_KEY_UP) {
            mvp_a_training_select_prev();
            mvp_a_last_result = MVP_A_RESULT_OK;
            return MVP_A_RESULT_OK;
        }
        if (key == MVP_A_KEY_DOWN) {
            mvp_a_training_select_next();
            mvp_a_last_result = MVP_A_RESULT_OK;
            return MVP_A_RESULT_OK;
        }
        if (key == MVP_A_KEY_CONFIRM) {
            ret = mvp_a_training_start_selected();
            mvp_a_last_result = ret;
            return ret;
        }
        if (key == MVP_A_KEY_BACK) {
            ret = mvp_a_app_scene_switch(MVP_A_SCENE_HOME);
            mvp_a_last_result = ret;
            return ret;
        }
        break;
    case MVP_A_SCENE_DEBUG:
        if (key == MVP_A_KEY_UP) {
            mvp_a_debug_select_prev();
            mvp_a_last_result = MVP_A_RESULT_OK;
            return MVP_A_RESULT_OK;
        }
        if (key == MVP_A_KEY_DOWN) {
            mvp_a_debug_select_next();
            mvp_a_last_result = MVP_A_RESULT_OK;
            return MVP_A_RESULT_OK;
        }
        if (key == MVP_A_KEY_CONFIRM) {
            ret = mvp_a_debug_execute_selected();
            mvp_a_last_result = ret;
            if ((ret == MVP_A_RESULT_OK) && (mvp_a_debug_get_selected() == MVP_A_DEBUG_RESET_SAVE)) {
                mvp_a_pet_reload();
                mvp_a_scene = MVP_A_SCENE_BOOT;
            }
            return ret;
        }
        break;
    default:
        break;
    }

    switch (key) {
    case MVP_A_KEY_UP:
        ret = mvp_a_app_scene_switch(mvp_a_prev_scene(mvp_a_scene));
        mvp_a_last_result = ret;
        return ret;
    case MVP_A_KEY_DOWN:
        ret = mvp_a_app_scene_switch(mvp_a_next_scene(mvp_a_scene));
        mvp_a_last_result = ret;
        return ret;
    case MVP_A_KEY_CONFIRM:
        mvp_a_last_result = MVP_A_RESULT_OK;
        return MVP_A_RESULT_OK;
    case MVP_A_KEY_BACK:
        ret = mvp_a_app_scene_switch(MVP_A_SCENE_HOME);
        mvp_a_last_result = ret;
        return ret;
    default:
        break;
    }

    return MVP_A_RESULT_OK;
}

mvp_a_result_t mvp_a_app_scene_switch(mvp_a_scene_t scene)
{
    if (scene >= MVP_A_SCENE_MAX) {
        return MVP_A_RESULT_INVALID_PARAM;
    }

    if (!mvp_a_inited) {
        mvp_a_app_init();
    }

    if ((scene == MVP_A_SCENE_HOME) && !mvp_a_pet_first_wake_done()) {
        scene = MVP_A_SCENE_BOOT;
    }

    printf("[MVP_A] scene switch %s(%d) -> %s(%d)\n",
           mvp_a_scene_name(mvp_a_scene), mvp_a_scene, mvp_a_scene_name(scene), scene);
    mvp_a_scene = scene;
    return MVP_A_RESULT_OK;
}

mvp_a_scene_t mvp_a_app_get_scene(void)
{
    return mvp_a_scene;
}

mvp_a_result_t mvp_a_app_get_last_result(void)
{
    return mvp_a_last_result;
}
