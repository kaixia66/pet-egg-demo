#ifndef PETEGG_JIELI_PORT_H_
#define PETEGG_JIELI_PORT_H_

#include "pet_input.h"
#include "pet_nfc_payload.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PETEGG_JIELI_OK 0
#define PETEGG_JIELI_ERR_INVALID_ARG 1
#define PETEGG_JIELI_ERR_UNSUPPORTED 2

typedef struct petegg_jieli_battery_state_t {
  uint16_t millivolts;
  uint8_t percent;
  uint8_t charging;
  uint8_t low_battery;
} petegg_jieli_battery_state_t;

int petegg_jieli_port_init(void);
int petegg_jieli_poll_input(PetKeyEvent* out_event);
int petegg_jieli_display_flush_stub(uint16_t x,
                                    uint16_t y,
                                    uint16_t width,
                                    uint16_t height,
                                    const uint16_t* rgb565);
int petegg_jieli_storage_read_stub(uint8_t slot,
                                   uint8_t* out_bytes,
                                   uint32_t capacity,
                                   uint32_t* out_len);
int petegg_jieli_storage_write_stub(uint8_t slot, const uint8_t* bytes, uint32_t len);
int petegg_jieli_resource_read_stub(uint16_t resource_id,
                                    uint32_t offset,
                                    uint8_t* out_bytes,
                                    uint32_t len);
int petegg_jieli_nfc_poll_stub(pet_nfc_card_payload_t* out_card, uint8_t* out_has_card);
int petegg_jieli_ble_send_stub(const uint8_t* bytes, uint16_t len);
int petegg_jieli_audio_play_stub(uint16_t event_id);
int petegg_jieli_power_state_stub(petegg_jieli_battery_state_t* out_state);
int petegg_jieli_debug_inject_fake_nfc(const pet_nfc_card_payload_t* payload);
int petegg_jieli_debug_inject_fake_key(uint8_t key, uint8_t event_type);
int petegg_jieli_debug_inject_fake_battery(uint16_t millivolts,
                                           uint8_t percent,
                                           uint8_t charging,
                                           uint8_t low_battery);

#ifdef __cplusplus
}
#endif

#endif  /* PETEGG_JIELI_PORT_H_ */
