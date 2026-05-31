#ifndef PET_SAVE_JIELI_SYSCFG_BACKEND_H
#define PET_SAVE_JIELI_SYSCFG_BACKEND_H

#include "pet_save_jieli.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PET_SAVE_JIELI_SYSCFG_SLOT_CAPACITY 160u
#define PET_SAVE_JIELI_SYSCFG_MAX_PAYLOAD \
    (PET_SAVE_JIELI_SYSCFG_SLOT_CAPACITY - PET_SAVE_SLOT_HEADER_SIZE)
#define PET_SAVE_JIELI_SYSCFG_SLOT_A_ITEM_ID 206u
#define PET_SAVE_JIELI_SYSCFG_SLOT_B_ITEM_ID 207u

typedef enum {
    PET_SAVE_JIELI_SYSCFG_BACKEND_NONE = 0,
    PET_SAVE_JIELI_SYSCFG_BACKEND_FAKE,
    PET_SAVE_JIELI_SYSCFG_BACKEND_SYSCFG
} pet_save_jieli_syscfg_backend_type_t;

typedef struct {
    pet_u16_t slot_a_item_id;
    pet_u16_t slot_b_item_id;
    pet_u16_t slot_capacity;
    pet_u8_t backend_type;
    pet_u8_t selected_slot;
    pet_result_t slot_a_read_result;
    pet_result_t slot_b_read_result;
    pet_result_t last_write_result;
    pet_result_t last_readback_result;
    pet_result_t last_fallback_result;
    pet_u64_t selected_counter;
    pet_u32_t selected_payload_len;
    pet_u32_t selected_crc32;
    pet_u16_t write_case_pass_count;
    pet_u16_t write_case_fail_count;
    pet_u16_t readback_case_pass_count;
    pet_u16_t readback_case_fail_count;
    pet_u16_t fallback_case_pass_count;
    pet_u16_t fallback_case_fail_count;
    pet_u8_t real_write_verified;
    pet_u8_t non_destructive_namespace;
} pet_save_jieli_syscfg_stats_t;

pet_result_t pet_save_jieli_syscfg_load_latest(pet_u8_t *out_payload,
                                               pet_u32_t out_capacity,
                                               pet_u32_t *out_payload_size,
                                               pet_u64_t *out_counter);
pet_result_t pet_save_jieli_syscfg_write_transaction(const pet_u8_t *payload,
                                                     pet_u32_t payload_size,
                                                     pet_u64_t *out_counter);
pet_result_t pet_save_jieli_syscfg_get_last_stats(pet_save_jieli_syscfg_stats_t *out_stats);
pet_result_t pet_save_jieli_syscfg_self_test(void);

#ifdef __cplusplus
}
#endif

#endif
