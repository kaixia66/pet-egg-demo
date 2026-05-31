#ifndef PET_SAVE_JIELI_H
#define PET_SAVE_JIELI_H

#include "pet_save_format.h"
#include "pet_save_jieli_memory_backend.h"

#ifdef __cplusplus
extern "C" {
#endif

pet_result_t pet_save_jieli_open_memory_backend(pet_save_jieli_memory_backend_t *backend);
void pet_save_jieli_close(void);
pet_result_t pet_save_jieli_load_latest(pet_u8_t *out_payload,
                                        pet_u32_t out_capacity,
                                        pet_u32_t *out_payload_size,
                                        pet_u64_t *out_counter);
pet_result_t pet_save_jieli_write_transaction(const pet_u8_t *payload,
                                              pet_u32_t payload_size,
                                              pet_u64_t *out_counter);
pet_result_t pet_save_jieli_validate_slot(const pet_u8_t *slot,
                                          pet_u32_t slot_capacity,
                                          pet_save_slot_header_t *out_header);
pet_result_t pet_save_jieli_choose_latest_slot(PetSaveSlot *out_slot,
                                               pet_save_slot_header_t *out_header);
pet_u32_t pet_save_jieli_crc32(const pet_u8_t *data, pet_u32_t len);
pet_result_t pet_save_jieli_self_test(void);

#ifdef __cplusplus
}
#endif

#endif
