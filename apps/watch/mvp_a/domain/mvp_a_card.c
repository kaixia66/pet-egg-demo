#include "mvp_a_card.h"
#include "mvp_a_save.h"

#include <string.h>

static void mvp_a_card_copy_text(char *dst, const char *src, u8 len)
{
    if (!dst || !len) {
        return;
    }

    memset(dst, 0, len);
    if (src) {
        strncpy(dst, src, len - 1);
    }
}

static mvp_a_bool_t mvp_a_card_id_exists(const mvp_a_save_data_t *data, const char *card_id)
{
    char normalized_id[MVP_A_CARD_ID_LEN];
    u8 i;

    if (!data) {
        return MVP_A_FALSE;
    }

    mvp_a_card_copy_text(normalized_id, card_id, sizeof(normalized_id));
    for (i = 0; (i < data->card_count) && (i < MVP_A_CARD_MAX); i++) {
        if (strncmp(data->cards[i].card_id, normalized_id, MVP_A_CARD_ID_LEN) == 0) {
            return MVP_A_TRUE;
        }
    }

    return MVP_A_FALSE;
}

mvp_a_result_t mvp_a_card_add(const char *card_id, const char *card_name, mvp_a_card_type_t type)
{
    mvp_a_save_data_t backup;
    mvp_a_save_data_t *data = mvp_a_save_get_data();
    mvp_a_card_data_t *card;
    mvp_a_result_t ret;

    if (!data) {
        return MVP_A_RESULT_NOT_READY;
    }

    if (type >= MVP_A_CARD_TYPE_MAX) {
        return MVP_A_RESULT_INVALID_PARAM;
    }

    if (mvp_a_card_id_exists(data, card_id)) {
        return MVP_A_RESULT_DUPLICATE;
    }

    if (data->card_count >= MVP_A_CARD_MAX) {
        return MVP_A_RESULT_STORAGE_FULL;
    }

    backup = *data;
    card = &data->cards[data->card_count];
    memset(card, 0, sizeof(*card));
    mvp_a_card_copy_text(card->card_id, card_id, MVP_A_CARD_ID_LEN);
    mvp_a_card_copy_text(card->card_name, card_name, MVP_A_CARD_NAME_LEN);
    card->card_type = (u8)type;
    card->version = 1;
    card->used = 1;
    data->card_count++;

    ret = mvp_a_save_store();
    if (ret != MVP_A_RESULT_OK) {
        *data = backup;
    }

    return ret;
}

u8 mvp_a_card_get_count(void)
{
    const mvp_a_save_data_t *data = mvp_a_save_get_const_data();

    if (!data) {
        return 0;
    }

    return data->card_count;
}

const mvp_a_card_data_t *mvp_a_card_get(u8 index)
{
    const mvp_a_save_data_t *data = mvp_a_save_get_const_data();

    if (!data || (index >= data->card_count) || (index >= MVP_A_CARD_MAX)) {
        return NULL;
    }

    return &data->cards[index];
}

mvp_a_result_t mvp_a_card_clear(void)
{
    mvp_a_save_data_t backup;
    mvp_a_save_data_t *data = mvp_a_save_get_data();
    mvp_a_result_t ret;

    if (!data) {
        return MVP_A_RESULT_NOT_READY;
    }

    backup = *data;
    data->card_count = 0;
    memset(data->cards, 0, sizeof(data->cards));

    ret = mvp_a_save_store();
    if (ret != MVP_A_RESULT_OK) {
        *data = backup;
    }

    return ret;
}
