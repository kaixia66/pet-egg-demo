#include "mvp_a_pet.h"
#include "mvp_a_assets.h"
#include "mvp_a_save.h"

#define MVP_A_PET_STAGE_FINAL      MVP_A_PET_STAGE_QINGLONG
#define MVP_A_PET_MOOD_MAX        100
#define MVP_A_PET_CLEAN_MAX       100
#define MVP_A_PET_ENERGY_MAX      100

static u8 mvp_a_pet_inited;
static u8 mvp_a_pet_wake_step;
static u8 mvp_a_pet_fast_growth = MVP_A_PET_FAST_GROWTH_DEFAULT;
static mvp_a_save_data_t mvp_a_pet_backup;

static const u8 mvp_a_stage_thresholds[] = {
    0, 0, 8, 20, 36, 56,
};

static u8 mvp_a_pet_clamp_add(u8 value, u8 add, u8 max)
{
    if ((u16)value + add >= max) {
        return max;
    }

    return value + add;
}

static void mvp_a_pet_add_diary(u32 flag)
{
    mvp_a_save_data_t *data = mvp_a_save_get_data();

    data->diary_flags |= flag;
}

static u32 mvp_a_pet_stage_diary_flag(mvp_a_pet_stage_t stage)
{
    switch (stage) {
    case MVP_A_PET_STAGE_CORE:
        return MVP_A_DIARY_STAGE_1_CORE;
    case MVP_A_PET_STAGE_BUN:
        return MVP_A_DIARY_STAGE_2_BUN;
    case MVP_A_PET_STAGE_WOOD:
        return MVP_A_DIARY_STAGE_3_WOOD;
    case MVP_A_PET_STAGE_COCOON:
        return MVP_A_DIARY_STAGE_4_COCOON;
    case MVP_A_PET_STAGE_QINGLONG:
        return MVP_A_DIARY_STAGE_5_QINGLONG;
    default:
        break;
    }

    return 0;
}

static mvp_a_result_t mvp_a_pet_commit_or_rollback(const mvp_a_save_data_t *backup)
{
    mvp_a_save_data_t *data = mvp_a_save_get_data();
    mvp_a_result_t ret = mvp_a_save_store();

    if (ret != MVP_A_RESULT_OK) {
        *data = *backup;
    }

    return ret;
}

static void mvp_a_pet_normalize_loaded_data(mvp_a_save_data_t *data)
{
    mvp_a_pet_stage_t stage;
    u32 diary_flags;

    if (!data) {
        return;
    }

    mvp_a_pet_backup = *data;
    if (data->pet_stage > MVP_A_PET_STAGE_FINAL) {
        data->pet_stage = MVP_A_PET_STAGE_FINAL;
    }

    stage = (mvp_a_pet_stage_t)data->pet_stage;
    diary_flags = data->diary_flags;
    if (stage >= MVP_A_PET_STAGE_CORE) {
        data->diary_flags |= MVP_A_DIARY_STAGE_1_CORE;
    }
    if (stage >= MVP_A_PET_STAGE_BUN) {
        data->diary_flags |= MVP_A_DIARY_STAGE_2_BUN;
    }
    if (stage >= MVP_A_PET_STAGE_WOOD) {
        data->diary_flags |= MVP_A_DIARY_STAGE_3_WOOD;
    }
    if (stage >= MVP_A_PET_STAGE_COCOON) {
        data->diary_flags |= MVP_A_DIARY_STAGE_4_COCOON;
    }
    if (stage >= MVP_A_PET_STAGE_QINGLONG) {
        data->diary_flags |= MVP_A_DIARY_STAGE_5_QINGLONG |
                             MVP_A_DIARY_COCOON_BREAK |
                             MVP_A_DIARY_FINAL_FORM;
    }

    if ((data->pet_stage != mvp_a_pet_backup.pet_stage) ||
        (data->diary_flags != diary_flags)) {
        mvp_a_pet_commit_or_rollback(&mvp_a_pet_backup);
    }
}

static void mvp_a_pet_set_stage(mvp_a_pet_stage_t stage)
{
    mvp_a_save_data_t *data = mvp_a_save_get_data();

    if (stage > MVP_A_PET_STAGE_FINAL) {
        stage = MVP_A_PET_STAGE_FINAL;
    }

    if (data->pet_stage == (u8)stage) {
        return;
    }

    data->pet_stage = (u8)stage;
    mvp_a_pet_add_diary(mvp_a_pet_stage_diary_flag(stage));

    if (stage >= MVP_A_PET_STAGE_QINGLONG) {
        mvp_a_pet_add_diary(MVP_A_DIARY_COCOON_BREAK | MVP_A_DIARY_FINAL_FORM);
    }
}

static void mvp_a_pet_update_stage_by_growth(void)
{
    mvp_a_save_data_t *data = mvp_a_save_get_data();
    mvp_a_pet_stage_t stage = (mvp_a_pet_stage_t)data->pet_stage;

    while ((stage < MVP_A_PET_STAGE_FINAL) &&
           (data->growth_points >= mvp_a_stage_thresholds[stage + 1])) {
        stage = (mvp_a_pet_stage_t)(stage + 1);
        mvp_a_pet_set_stage(stage);
    }
}

void mvp_a_pet_init(void)
{
    mvp_a_save_data_t *data;

    if (mvp_a_pet_inited) {
        return;
    }

    mvp_a_save_init();
    data = mvp_a_save_get_data();
    mvp_a_pet_normalize_loaded_data(data);
    mvp_a_pet_wake_step = (!data->first_wake_done && (data->pet_stage >= MVP_A_PET_STAGE_CORE)) ? 1 : 0;
    mvp_a_pet_fast_growth = MVP_A_PET_FAST_GROWTH_DEFAULT;
    mvp_a_pet_inited = 1;
}

void mvp_a_pet_reload(void)
{
    mvp_a_pet_inited = 0;
    mvp_a_pet_init();
}

void mvp_a_pet_reset_demo(void)
{
    mvp_a_save_reset();
    mvp_a_pet_reload();
}

mvp_a_result_t mvp_a_pet_first_wake_next(void)
{
    mvp_a_save_data_t *data;

    mvp_a_pet_init();
    data = mvp_a_save_get_data();

    if (data->first_wake_done) {
        return MVP_A_RESULT_OK;
    }

    mvp_a_pet_backup = *data;

    if (mvp_a_pet_wake_step == 0) {
        mvp_a_pet_wake_step = 1;
        mvp_a_pet_set_stage(MVP_A_PET_STAGE_CORE);
        mvp_a_pet_add_diary(MVP_A_DIARY_FIRST_WAKE);
        if (mvp_a_pet_commit_or_rollback(&mvp_a_pet_backup) != MVP_A_RESULT_OK) {
            mvp_a_pet_wake_step = 0;
            return MVP_A_RESULT_STORAGE_ERROR;
        }
        return MVP_A_RESULT_BUSY;
    }

    data->first_wake_done = 1;
    mvp_a_pet_add_diary(MVP_A_DIARY_FIRST_HOME);
    return mvp_a_pet_commit_or_rollback(&mvp_a_pet_backup);
}

mvp_a_result_t mvp_a_pet_add_growth(u8 points)
{
    mvp_a_save_data_t *data;

    mvp_a_pet_init();
    data = mvp_a_save_get_data();

    if (!data->first_wake_done) {
        return MVP_A_RESULT_NOT_READY;
    }

    mvp_a_pet_backup = *data;
    data->growth_points = mvp_a_pet_clamp_add(data->growth_points, points, 100);
    mvp_a_pet_update_stage_by_growth();
    return mvp_a_pet_commit_or_rollback(&mvp_a_pet_backup);
}

mvp_a_result_t mvp_a_pet_apply_care(mvp_a_care_action_t action)
{
    mvp_a_save_data_t *data;
    u8 growth = mvp_a_pet_fast_growth ? 6 : 2;

    mvp_a_pet_init();
    data = mvp_a_save_get_data();

    if (!data->first_wake_done) {
        return MVP_A_RESULT_NOT_READY;
    }

    mvp_a_pet_backup = *data;

    switch (action) {
    case MVP_A_CARE_COMPANION:
        data->pet_mood = mvp_a_pet_clamp_add(data->pet_mood, 16, MVP_A_PET_MOOD_MAX);
        mvp_a_pet_add_diary(MVP_A_DIARY_FIRST_COMPANION);
        break;
    case MVP_A_CARE_CLEAN:
        data->pet_clean = mvp_a_pet_clamp_add(data->pet_clean, 24, MVP_A_PET_CLEAN_MAX);
        mvp_a_pet_add_diary(MVP_A_DIARY_FIRST_CLEAN);
        break;
    case MVP_A_CARE_REST:
        data->pet_energy = mvp_a_pet_clamp_add(data->pet_energy, 22, MVP_A_PET_ENERGY_MAX);
        mvp_a_pet_add_diary(MVP_A_DIARY_FIRST_REST);
        break;
    default:
        return MVP_A_RESULT_INVALID_PARAM;
    }

    data->growth_points = mvp_a_pet_clamp_add(data->growth_points, growth, 100);
    mvp_a_pet_update_stage_by_growth();
    return mvp_a_pet_commit_or_rollback(&mvp_a_pet_backup);
}

mvp_a_bool_t mvp_a_pet_first_wake_done(void)
{
    mvp_a_pet_init();
    return mvp_a_save_get_const_data()->first_wake_done ? MVP_A_TRUE : MVP_A_FALSE;
}

mvp_a_pet_stage_t mvp_a_pet_get_stage(void)
{
    mvp_a_pet_init();
    return (mvp_a_pet_stage_t)mvp_a_save_get_const_data()->pet_stage;
}

u8 mvp_a_pet_get_growth(void)
{
    mvp_a_pet_init();
    return mvp_a_save_get_const_data()->growth_points;
}

u8 mvp_a_pet_get_mood(void)
{
    mvp_a_pet_init();
    return mvp_a_save_get_const_data()->pet_mood;
}

u8 mvp_a_pet_get_clean(void)
{
    mvp_a_pet_init();
    return mvp_a_save_get_const_data()->pet_clean;
}

u8 mvp_a_pet_get_energy(void)
{
    mvp_a_pet_init();
    return mvp_a_save_get_const_data()->pet_energy;
}

u32 mvp_a_pet_get_diary_flags(void)
{
    mvp_a_pet_init();
    return mvp_a_save_get_const_data()->diary_flags;
}

mvp_a_result_t mvp_a_pet_mark_diary(u32 flag)
{
    mvp_a_save_data_t *data;

    mvp_a_pet_init();
    data = mvp_a_save_get_data();
    mvp_a_pet_backup = *data;
    mvp_a_pet_add_diary(flag);
    return mvp_a_pet_commit_or_rollback(&mvp_a_pet_backup);
}

mvp_a_bool_t mvp_a_pet_fast_growth_enabled(void)
{
    mvp_a_pet_init();
    return mvp_a_pet_fast_growth ? MVP_A_TRUE : MVP_A_FALSE;
}

mvp_a_result_t mvp_a_pet_set_fast_growth(mvp_a_bool_t enable)
{
    mvp_a_pet_init();
    mvp_a_pet_fast_growth = (enable == MVP_A_TRUE);
    if (mvp_a_pet_fast_growth) {
        return mvp_a_pet_mark_diary(MVP_A_DIARY_FAST_GROWTH);
    }

    return MVP_A_RESULT_OK;
}

void mvp_a_pet_get_snapshot(mvp_a_pet_snapshot_t *snapshot)
{
    const mvp_a_save_data_t *data;

    if (!snapshot) {
        return;
    }

    mvp_a_pet_init();
    data = mvp_a_save_get_const_data();

    snapshot->first_wake_done = data->first_wake_done;
    snapshot->wake_step = mvp_a_pet_wake_step;
    snapshot->stage = (mvp_a_pet_stage_t)data->pet_stage;
    snapshot->mood = data->pet_mood;
    snapshot->clean = data->pet_clean;
    snapshot->energy = data->pet_energy;
    snapshot->growth_points = data->growth_points;
    snapshot->fast_growth = mvp_a_pet_fast_growth;
    snapshot->diary_flags = data->diary_flags;
}

const char *mvp_a_pet_get_stage_name(mvp_a_pet_stage_t stage)
{
    return mvp_a_assets_stage_name(stage);
}

const char *mvp_a_pet_get_wake_prompt(void)
{
    mvp_a_pet_init();

    if (mvp_a_save_get_const_data()->first_wake_done) {
        return "Nest Ready";
    }

    return mvp_a_pet_wake_step ? "Spirit Core" : "Spirit Egg";
}
