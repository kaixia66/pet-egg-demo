#include "mvp_a_save.h"
#include "mvp_a_platform.h"

#include <string.h>

static mvp_a_save_data_t mvp_a_save_data;
static u8 mvp_a_save_loaded;

static u32 mvp_a_save_checksum_bytes(const u8 *data, u16 len)
{
    u32 hash = 2166136261UL;
    u16 i;

    for (i = 0; i < len; i++) {
        hash ^= data[i];
        hash *= 16777619UL;
    }

    return hash;
}

u32 mvp_a_save_checksum(const mvp_a_save_data_t *data)
{
    if (!data) {
        return 0;
    }

    return mvp_a_save_checksum_bytes((const u8 *)data, sizeof(*data) - sizeof(data->checksum));
}

void mvp_a_save_make_default(mvp_a_save_data_t *data)
{
    if (!data) {
        return;
    }

    memset(data, 0, sizeof(*data));
    data->magic = MVP_A_SAVE_MAGIC;
    data->version = MVP_A_SAVE_VERSION;
    data->data_len = sizeof(*data);
    data->initialized = 1;
    data->pet_stage = MVP_A_PET_STAGE_EGG;
    data->pet_mood = 55;
    data->pet_energy = 60;
    data->pet_clean = 60;
    data->checksum = mvp_a_save_checksum(data);
}

mvp_a_bool_t mvp_a_save_validate(const mvp_a_save_data_t *data)
{
    if (!data) {
        return MVP_A_FALSE;
    }

    if ((data->magic != MVP_A_SAVE_MAGIC) ||
        (data->version != MVP_A_SAVE_VERSION) ||
        (data->data_len != sizeof(*data)) ||
        (data->checksum != mvp_a_save_checksum(data))) {
        return MVP_A_FALSE;
    }

    return MVP_A_TRUE;
}

mvp_a_result_t mvp_a_save_load(void)
{
    mvp_a_result_t ret;

    ret = mvp_a_platform_storage_load(&mvp_a_save_data);
    printf("[MVP_A] save load ret=%d\n", ret);
    if (ret == MVP_A_RESULT_OK) {
        if (!mvp_a_save_validate(&mvp_a_save_data)) {
            printf("[MVP_A] save invalid, make default\n");
            mvp_a_save_make_default(&mvp_a_save_data);
            mvp_a_save_loaded = 1;
            return mvp_a_save_store();
        }

        mvp_a_save_loaded = 1;
        printf("[MVP_A] save loaded stage=%d growth=%d flags=0x%x\n",
               mvp_a_save_data.pet_stage, mvp_a_save_data.growth_points, mvp_a_save_data.diary_flags);
        return MVP_A_RESULT_OK;
    }

    if (ret == MVP_A_RESULT_NOT_FOUND) {
        printf("[MVP_A] save not found, make default\n");
        mvp_a_save_make_default(&mvp_a_save_data);
        mvp_a_save_loaded = 1;
        return mvp_a_save_store();
    }

    mvp_a_save_make_default(&mvp_a_save_data);
    mvp_a_save_loaded = 1;
    return ret;
}

mvp_a_result_t mvp_a_save_init(void)
{
    if (mvp_a_save_loaded) {
        return MVP_A_RESULT_OK;
    }

    return mvp_a_save_load();
}

mvp_a_result_t mvp_a_save_store(void)
{
    mvp_a_save_data.magic = MVP_A_SAVE_MAGIC;
    mvp_a_save_data.version = MVP_A_SAVE_VERSION;
    mvp_a_save_data.data_len = sizeof(mvp_a_save_data);
    mvp_a_save_data.initialized = 1;
    mvp_a_save_data.checksum = mvp_a_save_checksum(&mvp_a_save_data);

    {
        mvp_a_result_t ret = mvp_a_platform_storage_save(&mvp_a_save_data);
        printf("[MVP_A] save store ret=%d stage=%d growth=%d flags=0x%x\n",
               ret, mvp_a_save_data.pet_stage, mvp_a_save_data.growth_points, mvp_a_save_data.diary_flags);
        return ret;
    }
}

mvp_a_result_t mvp_a_save_reset(void)
{
    mvp_a_save_make_default(&mvp_a_save_data);
    mvp_a_save_loaded = 1;
    printf("[MVP_A] save reset\n");
    return mvp_a_save_store();
}

mvp_a_save_data_t *mvp_a_save_get_data(void)
{
    mvp_a_save_init();
    return &mvp_a_save_data;
}

const mvp_a_save_data_t *mvp_a_save_get_const_data(void)
{
    mvp_a_save_init();
    return &mvp_a_save_data;
}

mvp_a_bool_t mvp_a_save_is_loaded(void)
{
    return mvp_a_save_loaded ? MVP_A_TRUE : MVP_A_FALSE;
}
