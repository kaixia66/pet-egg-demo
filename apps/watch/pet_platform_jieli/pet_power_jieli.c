#include "pet_platform_jieli_internal.h"

void pet_power_jieli_init(void)
{
}

pet_result_t pet_power_jieli_get_battery_percent(void *ctx, pet_u8_t *percent)
{
    pet_u8_t fake_percent;
    pet_u16_t fake_voltage;

    (void)ctx;

    if (percent == 0) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

#if PET_JIELI_TEST_MODE_ENABLED
    if (pet_debug_jieli_get_fake_battery(&fake_percent, &fake_voltage) == PET_TRUE) {
        (void)fake_voltage;
        *percent = fake_percent;
        return PET_RESULT_OK;
    }
#else
    (void)fake_percent;
    (void)fake_voltage;
#endif

    /* TODO(P2+): connect to get_vbat_percent or the confirmed SDK power API. */
    *percent = 100u;
    return PET_RESULT_OK;
}

pet_result_t pet_power_jieli_get_battery_voltage_mv(void *ctx, pet_u16_t *voltage_mv)
{
    pet_u8_t fake_percent;
    pet_u16_t fake_voltage;

    (void)ctx;

    if (voltage_mv == 0) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

#if PET_JIELI_TEST_MODE_ENABLED
    if (pet_debug_jieli_get_fake_battery(&fake_percent, &fake_voltage) == PET_TRUE) {
        (void)fake_percent;
        *voltage_mv = fake_voltage;
        return PET_RESULT_OK;
    }
#else
    (void)fake_percent;
    (void)fake_voltage;
#endif

    /* TODO(P2+): connect to vbat_check/power APIs once ownership is confirmed. */
    *voltage_mv = 0u;
    return PET_RESULT_OK;
}

pet_bool_t pet_power_jieli_is_low_power(void)
{
    return PET_FALSE;
}
