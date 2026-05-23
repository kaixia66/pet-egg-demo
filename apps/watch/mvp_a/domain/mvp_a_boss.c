#include "mvp_a_boss.h"
#include "mvp_a_save.h"

static mvp_a_result_t mvp_a_boss_commit_or_rollback(const mvp_a_save_data_t *backup)
{
    mvp_a_save_data_t *data = mvp_a_save_get_data();
    mvp_a_result_t ret = mvp_a_save_store();

    if (ret != MVP_A_RESULT_OK) {
        *data = *backup;
    }

    return ret;
}

mvp_a_result_t mvp_a_boss_unlock(void)
{
    mvp_a_save_data_t backup;
    mvp_a_save_data_t *data = mvp_a_save_get_data();

    if (!data) {
        return MVP_A_RESULT_NOT_READY;
    }

    backup = *data;
    data->boss_unlocked = 1;
    data->diary_flags |= MVP_A_DIARY_FIRST_COOP_BOSS;
    return mvp_a_boss_commit_or_rollback(&backup);
}

mvp_a_result_t mvp_a_boss_record_win(void)
{
    mvp_a_save_data_t backup;
    mvp_a_save_data_t *data = mvp_a_save_get_data();

    if (!data) {
        return MVP_A_RESULT_NOT_READY;
    }

    backup = *data;
    data->boss_unlocked = 1;
    if (data->boss_win_count < 0xff) {
        data->boss_win_count++;
    }
    data->diary_flags |= MVP_A_DIARY_FIRST_COOP_BOSS | MVP_A_DIARY_FIRST_BOSS_WIN;
    return mvp_a_boss_commit_or_rollback(&backup);
}

mvp_a_bool_t mvp_a_boss_is_unlocked(void)
{
    const mvp_a_save_data_t *data = mvp_a_save_get_const_data();

    if (!data) {
        return MVP_A_FALSE;
    }

    return data->boss_unlocked ? MVP_A_TRUE : MVP_A_FALSE;
}

u8 mvp_a_boss_get_win_count(void)
{
    const mvp_a_save_data_t *data = mvp_a_save_get_const_data();

    if (!data) {
        return 0;
    }

    return data->boss_win_count;
}
