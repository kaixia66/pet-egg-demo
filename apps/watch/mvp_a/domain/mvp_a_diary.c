#include "mvp_a_diary.h"
#include "mvp_a_save.h"

mvp_a_result_t mvp_a_diary_mark(u32 flag)
{
    mvp_a_save_data_t backup;
    mvp_a_save_data_t *data = mvp_a_save_get_data();
    mvp_a_result_t ret;

    if (!data) {
        return MVP_A_RESULT_NOT_READY;
    }

    backup = *data;
    data->diary_flags |= flag;
    ret = mvp_a_save_store();
    if (ret != MVP_A_RESULT_OK) {
        *data = backup;
    }

    return ret;
}

mvp_a_bool_t mvp_a_diary_has(u32 flag)
{
    const mvp_a_save_data_t *data = mvp_a_save_get_const_data();

    if (!data) {
        return MVP_A_FALSE;
    }

    return (data->diary_flags & flag) ? MVP_A_TRUE : MVP_A_FALSE;
}

u32 mvp_a_diary_get_flags(void)
{
    const mvp_a_save_data_t *data = mvp_a_save_get_const_data();

    if (!data) {
        return 0;
    }

    return data->diary_flags;
}

const char *mvp_a_diary_get_summary(void)
{
    u32 flags = mvp_a_diary_get_flags();

    if (flags & MVP_A_DIARY_STAGE_5_QINGLONG) {
        return "Qinglong Ready";
    }
    if (flags & MVP_A_DIARY_STAGE_4_COCOON) {
        return "Cocoon";
    }
    if (flags & MVP_A_DIARY_FIRST_TRAINING) {
        return "Training";
    }
    if (flags & MVP_A_DIARY_FIRST_COMPANION) {
        return "Companion";
    }
    if (flags & MVP_A_DIARY_FIRST_HOME) {
        return "Nest Open";
    }
    if (flags & MVP_A_DIARY_FIRST_WAKE) {
        return "Wake Core";
    }

    return "No Diary";
}
