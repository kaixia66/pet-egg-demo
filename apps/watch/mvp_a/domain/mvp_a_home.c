#include "mvp_a_home.h"
#include "mvp_a_pet.h"

static mvp_a_care_action_t mvp_a_home_selected = MVP_A_CARE_COMPANION;

void mvp_a_home_init(void)
{
    mvp_a_home_selected = MVP_A_CARE_COMPANION;
}

mvp_a_care_action_t mvp_a_home_get_selected_action(void)
{
    return mvp_a_home_selected;
}

void mvp_a_home_select_next_action(void)
{
    mvp_a_home_selected = (mvp_a_care_action_t)(mvp_a_home_selected + 1);
    if (mvp_a_home_selected >= MVP_A_CARE_MAX) {
        mvp_a_home_selected = MVP_A_CARE_COMPANION;
    }
}

void mvp_a_home_select_prev_action(void)
{
    if (mvp_a_home_selected == MVP_A_CARE_COMPANION) {
        mvp_a_home_selected = (mvp_a_care_action_t)(MVP_A_CARE_MAX - 1);
    } else {
        mvp_a_home_selected = (mvp_a_care_action_t)(mvp_a_home_selected - 1);
    }
}

mvp_a_result_t mvp_a_home_apply_selected_action(void)
{
    return mvp_a_pet_apply_care(mvp_a_home_selected);
}

const char *mvp_a_home_get_action_name(mvp_a_care_action_t action)
{
    switch (action) {
    case MVP_A_CARE_COMPANION:
        return "Companion";
    case MVP_A_CARE_CLEAN:
        return "Clean";
    case MVP_A_CARE_REST:
        return "Rest";
    default:
        break;
    }

    return "Care";
}

const char *mvp_a_home_get_prompt(void)
{
    return mvp_a_pet_get_stage_name(mvp_a_pet_get_stage());
}

const char *mvp_a_home_get_action_prompt(void)
{
    return mvp_a_home_get_action_name(mvp_a_home_selected);
}
