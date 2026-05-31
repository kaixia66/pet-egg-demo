#include "pet_save_jieli_memory_backend.h"

static void pet_save_jieli_zero_bytes(pet_u8_t *bytes, pet_u32_t len)
{
    pet_u32_t i;

    if (bytes == 0) {
        return;
    }

    for (i = 0u; i < len; ++i) {
        bytes[i] = 0u;
    }
}

pet_result_t pet_save_jieli_memory_backend_init(pet_save_jieli_memory_backend_t *backend,
                                                pet_u8_t *slot_a,
                                                pet_u32_t slot_a_capacity,
                                                pet_u8_t *slot_b,
                                                pet_u32_t slot_b_capacity)
{
    if ((backend == 0) || (slot_a == 0) || (slot_b == 0) ||
        (slot_a_capacity < PET_SAVE_SLOT_HEADER_SIZE) ||
        (slot_b_capacity < PET_SAVE_SLOT_HEADER_SIZE)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    backend->slot_a = slot_a;
    backend->slot_a_capacity = slot_a_capacity;
    backend->slot_b = slot_b;
    backend->slot_b_capacity = slot_b_capacity;
    backend->fault = PET_SAVE_JIELI_FAULT_NONE;
    backend->low_battery_block = PET_FALSE;
    return PET_RESULT_OK;
}

void pet_save_jieli_memory_backend_clear(pet_save_jieli_memory_backend_t *backend)
{
    if (backend == 0) {
        return;
    }

    pet_save_jieli_zero_bytes(backend->slot_a, backend->slot_a_capacity);
    pet_save_jieli_zero_bytes(backend->slot_b, backend->slot_b_capacity);
    backend->fault = PET_SAVE_JIELI_FAULT_NONE;
    backend->low_battery_block = PET_FALSE;
}

void pet_save_jieli_memory_backend_set_fault(pet_save_jieli_memory_backend_t *backend,
                                             pet_save_jieli_fault_t fault)
{
    if (backend == 0) {
        return;
    }

    backend->fault = fault;
}

void pet_save_jieli_memory_backend_set_low_battery_block(pet_save_jieli_memory_backend_t *backend,
                                                        pet_bool_t blocked)
{
    if (backend == 0) {
        return;
    }

    backend->low_battery_block = blocked;
}
