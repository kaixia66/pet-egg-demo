#include "mvp_a_debug.h"
#include "mvp_a_pet.h"
#include "mvp_a_save.h"
#include "pet2d_scene.h"

static mvp_a_debug_action_t mvp_a_debug_selected = MVP_A_DEBUG_VIEW_SAVE;

void mvp_a_debug_init(void)
{
    mvp_a_debug_selected = MVP_A_DEBUG_VIEW_SAVE;
}

void mvp_a_debug_select_next(void)
{
    mvp_a_debug_selected = (mvp_a_debug_action_t)(mvp_a_debug_selected + 1);
    if (mvp_a_debug_selected >= MVP_A_DEBUG_MAX) {
        mvp_a_debug_selected = MVP_A_DEBUG_VIEW_SAVE;
    }
}

void mvp_a_debug_select_prev(void)
{
    if (mvp_a_debug_selected == MVP_A_DEBUG_VIEW_SAVE) {
        mvp_a_debug_selected = (mvp_a_debug_action_t)(MVP_A_DEBUG_MAX - 1);
    } else {
        mvp_a_debug_selected = (mvp_a_debug_action_t)(mvp_a_debug_selected - 1);
    }
}

mvp_a_debug_action_t mvp_a_debug_get_selected(void)
{
    return mvp_a_debug_selected;
}

mvp_a_result_t mvp_a_debug_execute_selected(void)
{
    switch (mvp_a_debug_selected) {
    case MVP_A_DEBUG_VIEW_SAVE:
        return MVP_A_RESULT_OK;
    case MVP_A_DEBUG_RESET_SAVE:
        return mvp_a_save_reset();
    case MVP_A_DEBUG_FAST_GROWTH:
        return mvp_a_pet_set_fast_growth(mvp_a_pet_fast_growth_enabled() ? MVP_A_FALSE : MVP_A_TRUE);
    case MVP_A_DEBUG_PET2D_SCENE:
        return (pet2d_scene_enter_test() == PET_RESULT_OK) ? MVP_A_RESULT_OK : MVP_A_RESULT_NOT_READY;
    default:
        break;
    }

    return MVP_A_RESULT_INVALID_PARAM;
}

const char *mvp_a_debug_get_action_name(mvp_a_debug_action_t action)
{
    switch (action) {
    case MVP_A_DEBUG_VIEW_SAVE:
        return "View";
    case MVP_A_DEBUG_RESET_SAVE:
        return "Clear";
    case MVP_A_DEBUG_FAST_GROWTH:
        return "Fast";
    case MVP_A_DEBUG_PET2D_SCENE:
        return "P18 Scene";
    default:
        break;
    }

    return "Debug";
}

const char *mvp_a_debug_get_prompt(void)
{
    const mvp_a_save_data_t *data;

    switch (mvp_a_debug_selected) {
    case MVP_A_DEBUG_VIEW_SAVE:
        data = mvp_a_save_get_const_data();
        if (!data) {
            return "No Save";
        }
        return mvp_a_pet_get_stage_name((mvp_a_pet_stage_t)data->pet_stage);
    case MVP_A_DEBUG_RESET_SAVE:
        return "Confirm Clear";
    case MVP_A_DEBUG_FAST_GROWTH:
        return mvp_a_pet_fast_growth_enabled() ? "Fast On" : "Fast Off";
    case MVP_A_DEBUG_PET2D_SCENE:
        return "Pet2D Scene";
    default:
        break;
    }

    return "Debug";
}
