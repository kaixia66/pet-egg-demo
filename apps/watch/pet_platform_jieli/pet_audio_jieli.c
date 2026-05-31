#include "pet_platform_jieli_internal.h"

static pet_u8_t g_pet_audio_jieli_volume = 80u;
static pet_bool_t g_pet_audio_jieli_muted = PET_FALSE;

void pet_audio_jieli_init(void)
{
    g_pet_audio_jieli_volume = 80u;
    g_pet_audio_jieli_muted = PET_FALSE;
}

pet_result_t pet_audio_jieli_play_sfx(void *ctx, pet_u32_t sfx_id, pet_audio_channel_t channel)
{
    (void)ctx;
    (void)sfx_id;
    (void)channel;
    return PET_RESULT_NOT_READY;
}

pet_result_t pet_audio_jieli_stop(void *ctx, pet_audio_channel_t channel)
{
    (void)ctx;
    (void)channel;
    return PET_RESULT_OK;
}

pet_result_t pet_audio_jieli_set_volume(pet_u8_t percent)
{
    if (percent > 100u) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    g_pet_audio_jieli_volume = percent;
    return PET_RESULT_OK;
}

pet_result_t pet_audio_jieli_set_mute(pet_bool_t muted)
{
    g_pet_audio_jieli_muted = muted ? PET_TRUE : PET_FALSE;
    return PET_RESULT_OK;
}

pet_bool_t pet_audio_jieli_is_busy(void)
{
    return PET_FALSE;
}
