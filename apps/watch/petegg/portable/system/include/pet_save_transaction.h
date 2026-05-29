#ifndef PETEGG_PORTABLE_PET_SAVE_TRANSACTION_H_
#define PETEGG_PORTABLE_PET_SAVE_TRANSACTION_H_

#include "pet_result.h"
#include "pet_save_format.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Portable System Core save transaction shell.
   This layer only copies, validates, aborts, and serializes in-memory payloads.
   It does not read or write files, Flash, platform save slots, or hardware. */
pet_result_t pet_save_transaction_begin(const pet_device_save_payload_t* base,
                                        pet_device_save_payload_t* working);
pet_result_t pet_save_transaction_validate(const pet_device_save_payload_t* working);
pet_result_t pet_save_transaction_commit_to_bytes(const pet_device_save_payload_t* working,
                                                  uint64_t counter,
                                                  uint64_t timestamp_ms,
                                                  uint8_t* out_bytes,
                                                  uint32_t out_capacity,
                                                  uint32_t* out_len);
pet_result_t pet_save_transaction_abort(pet_device_save_payload_t* working,
                                        const pet_device_save_payload_t* base);

#ifdef __cplusplus
}
#endif

#endif  /* PETEGG_PORTABLE_PET_SAVE_TRANSACTION_H_ */
