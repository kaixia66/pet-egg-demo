#ifndef PET_SAVE_JIELI_MEMORY_BACKEND_H
#define PET_SAVE_JIELI_MEMORY_BACKEND_H

#include "pet_save_jieli_backend.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    pet_u8_t *slot_a;
    pet_u32_t slot_a_capacity;
    pet_u8_t *slot_b;
    pet_u32_t slot_b_capacity;
    pet_save_jieli_fault_t fault;
    pet_bool_t low_battery_block;
} pet_save_jieli_memory_backend_t;

pet_result_t pet_save_jieli_memory_backend_init(pet_save_jieli_memory_backend_t *backend,
                                                pet_u8_t *slot_a,
                                                pet_u32_t slot_a_capacity,
                                                pet_u8_t *slot_b,
                                                pet_u32_t slot_b_capacity);
void pet_save_jieli_memory_backend_clear(pet_save_jieli_memory_backend_t *backend);
void pet_save_jieli_memory_backend_set_fault(pet_save_jieli_memory_backend_t *backend,
                                             pet_save_jieli_fault_t fault);
void pet_save_jieli_memory_backend_set_low_battery_block(pet_save_jieli_memory_backend_t *backend,
                                                        pet_bool_t blocked);

#ifdef __cplusplus
}
#endif

#endif
