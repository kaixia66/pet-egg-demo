#include "mvp_a_training.h"
#include "mvp_a_pet.h"

static mvp_a_training_type_t mvp_a_training_selected = MVP_A_TRAINING_COURAGE;

void mvp_a_training_init(void)
{
    mvp_a_training_selected = MVP_A_TRAINING_COURAGE;
}

mvp_a_training_type_t mvp_a_training_get_selected_type(void)
{
    return mvp_a_training_selected;
}

void mvp_a_training_select_next(void)
{
    mvp_a_training_selected = (mvp_a_training_type_t)(mvp_a_training_selected + 1);
    if (mvp_a_training_selected >= MVP_A_TRAINING_MAX) {
        mvp_a_training_selected = MVP_A_TRAINING_COURAGE;
    }
}

void mvp_a_training_select_prev(void)
{
    if (mvp_a_training_selected == MVP_A_TRAINING_COURAGE) {
        mvp_a_training_selected = (mvp_a_training_type_t)(MVP_A_TRAINING_MAX - 1);
    } else {
        mvp_a_training_selected = (mvp_a_training_type_t)(mvp_a_training_selected - 1);
    }
}

mvp_a_result_t mvp_a_training_start_selected(void)
{
    mvp_a_result_t ret;
    u8 growth = mvp_a_pet_fast_growth_enabled() ? 10 : 3;

    if (!mvp_a_pet_first_wake_done()) {
        return MVP_A_RESULT_NOT_READY;
    }

    ret = mvp_a_pet_mark_diary(MVP_A_DIARY_FIRST_TRAINING);
    if (ret != MVP_A_RESULT_OK) {
        return ret;
    }

    return mvp_a_pet_add_growth(growth);
}

const char *mvp_a_training_get_type_name(mvp_a_training_type_t type)
{
    switch (type) {
    case MVP_A_TRAINING_COURAGE:
        return "Courage";
    case MVP_A_TRAINING_GUARD:
        return "Guard";
    default:
        break;
    }

    return "Train";
}

const char *mvp_a_training_get_prompt(void)
{
    return mvp_a_training_get_type_name(mvp_a_training_selected);
}
