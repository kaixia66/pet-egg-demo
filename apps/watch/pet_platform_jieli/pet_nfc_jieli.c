#include "pet_platform_jieli_internal.h"

void pet_nfc_jieli_init(void)
{
}

pet_result_t pet_nfc_jieli_start_card_scan(void *ctx)
{
    (void)ctx;
    return PET_RESULT_NOT_READY;
}

pet_result_t pet_nfc_jieli_start_pair_scan(void *ctx)
{
    (void)ctx;
    return PET_RESULT_NOT_READY;
}

pet_result_t pet_nfc_jieli_poll_card(void *ctx, pet_nfc_card_t *card)
{
    (void)ctx;

    if (card == 0) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    return PET_RESULT_NOT_READY;
}

pet_result_t pet_nfc_jieli_poll_pair(void *ctx, pet_nfc_pair_t *pair)
{
    (void)ctx;

    if (pair == 0) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    return PET_RESULT_NOT_READY;
}
