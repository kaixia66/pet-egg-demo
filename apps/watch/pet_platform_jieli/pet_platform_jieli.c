#include "pet_platform_jieli_internal.h"

extern unsigned int timer_get_ms(void);

typedef struct {
    pet_bool_t initialized;
} pet_platform_jieli_state_t;

static pet_platform_jieli_state_t g_pet_platform_jieli_state;

static pet_u32_t pet_platform_jieli_millis(void *ctx)
{
    (void)ctx;
    return (pet_u32_t)timer_get_ms();
}

static pet_u32_t pet_platform_jieli_now_sec(void *ctx)
{
    (void)ctx;
    /* TODO(P2): wire to RTC once the board RTC ownership and epoch policy are confirmed. */
    return 0u;
}

static void pet_platform_jieli_copy_string(char *dst, pet_size_t dst_len, const char *src)
{
    pet_size_t i;

    if ((dst == 0) || (dst_len == 0u)) {
        return;
    }

    for (i = 0u; (i + 1u < dst_len) && (src != 0) && (src[i] != '\0'); ++i) {
        dst[i] = src[i];
    }
    dst[i] = '\0';
}

static pet_result_t pet_platform_jieli_get_device_identity(void *ctx,
                                                           pet_device_identity_t *identity)
{
    (void)ctx;

    if (identity == 0) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    pet_platform_jieli_copy_string(identity->device_id, sizeof(identity->device_id),
                                   "PETEGG-701N-P2");
    pet_platform_jieli_copy_string(identity->model, sizeof(identity->model), "AC701N_DEV");
    identity->hardware_version = PET_JIELI_HARDWARE_REV_701N_DEV;
    identity->firmware_version = PET_JIELI_FIRMWARE_VERSION_P2_STUB;
    return PET_RESULT_OK;
}

static const pet_platform_t g_pet_platform_jieli = {
    pet_platform_jieli_millis,
    pet_platform_jieli_now_sec,
    pet_platform_jieli_get_device_identity,
    pet_display_jieli_get_profile,
    pet_input_jieli_poll_key_event,
    pet_display_jieli_acquire,
    pet_display_jieli_release,
    pet_display_jieli_flush,
    pet_display_jieli_wait,
    pet_display_jieli_set_brightness,
    pet_display_jieli_sleep,
    pet_display_jieli_wakeup,
    pet_storage_jieli_read,
    pet_storage_jieli_write_atomic,
    pet_audio_jieli_play_sfx,
    pet_audio_jieli_stop,
    pet_ble_jieli_send_packet,
    pet_ble_jieli_poll_packet,
    pet_nfc_jieli_start_card_scan,
    pet_nfc_jieli_start_pair_scan,
    pet_nfc_jieli_poll_card,
    pet_nfc_jieli_poll_pair,
    pet_power_jieli_get_battery_percent,
    pet_power_jieli_get_battery_voltage_mv,
    &g_pet_platform_jieli_state
};

void pet_platform_jieli_init(void)
{
    if (g_pet_platform_jieli_state.initialized == PET_TRUE) {
        return;
    }

    pet_display_jieli_init();
    pet_input_jieli_init();
    pet_storage_jieli_init();
    pet_audio_jieli_init();
    pet_ble_jieli_init();
    pet_nfc_jieli_init();
    pet_power_jieli_init();
    pet_debug_jieli_init();
    g_pet_platform_jieli_state.initialized = PET_TRUE;
}

const pet_platform_t *pet_platform_jieli_get(void)
{
    pet_platform_jieli_init();
    return &g_pet_platform_jieli;
}
