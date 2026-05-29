#ifndef PETEGG_PORTABLE_PET_POOL_H_
#define PETEGG_PORTABLE_PET_POOL_H_

#include "pet_model.h"
#include "pet_result.h"
#include "pet_save_format.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Device-local pet pool helpers for pet_device_save_payload_t. Duplicate species are
   allowed as separate pet instances; PET_MAX_COUNT remains the hard local capacity. */
pet_result_t pet_pool_init_empty(pet_device_save_payload_t* save);
pet_result_t pet_pool_add_pet(pet_device_save_payload_t* save,
                              uint16_t species_id,
                              uint32_t pet_instance_id,
                              const char* nickname,
                              uint8_t set_active,
                              uint8_t* out_index);
pet_result_t pet_pool_add_stage0_from_species(pet_device_save_payload_t* save,
                                              uint16_t species_id,
                                              uint32_t pet_instance_id,
                                              uint8_t set_active,
                                              uint8_t* out_index);
pet_result_t pet_pool_get_active(pet_device_save_payload_t* save,
                                 pet_pet_record_t** out_pet);
pet_result_t pet_pool_get_active_const(const pet_device_save_payload_t* save,
                                       const pet_pet_record_t** out_pet);
pet_result_t pet_pool_set_active(pet_device_save_payload_t* save, uint8_t index);
pet_result_t pet_pool_find_by_instance_id(pet_device_save_payload_t* save,
                                          uint32_t pet_instance_id,
                                          pet_pet_record_t** out_pet,
                                          uint8_t* out_index);
pet_result_t pet_pool_find_by_species_id(pet_device_save_payload_t* save,
                                         uint16_t species_id,
                                         pet_pet_record_t** out_pet,
                                         uint8_t* out_index);
pet_result_t pet_pool_validate(const pet_device_save_payload_t* save);

#ifdef __cplusplus
}
#endif

#endif  /* PETEGG_PORTABLE_PET_POOL_H_ */
