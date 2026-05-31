#ifndef PET_PLATFORM_H
#define PET_PLATFORM_H

#include "pet_display_profile.h"
#include "pet_key.h"
#include "pet_protocol.h"
#include "pet_save_format.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PET_DEVICE_ID_MAX 32u
#define PET_DEVICE_MODEL_MAX 24u
#define PET_NFC_CARD_ID_MAX 16u

typedef enum {
    PET_STORAGE_AREA_SAVE = 0,
    PET_STORAGE_AREA_RESOURCE,
    PET_STORAGE_AREA_SETTINGS
} pet_storage_area_t;

typedef enum {
    PET_AUDIO_CHANNEL_SFX = 0,
    PET_AUDIO_CHANNEL_UI,
    PET_AUDIO_CHANNEL_AMBIENT
} pet_audio_channel_t;

typedef pet_u32_t (*PetPlatformMillisFn)(void *user);
typedef void (*PetPlatformLogFn)(void *user, const char *message);
typedef PetResult (*PetPlatformSaveReadFn)(void *user,
                                           pet_u8_t slot,
                                           pet_u8_t *out_bytes,
                                           pet_size_t out_capacity,
                                           pet_size_t *out_len);
typedef PetResult (*PetPlatformSaveWriteFn)(void *user,
                                            pet_u8_t slot,
                                            const pet_u8_t *bytes,
                                            pet_size_t len);

typedef struct PetPlatformCallbacks {
    void *user;
    PetPlatformMillisFn millis;
    PetPlatformLogFn log;
    PetPlatformSaveReadFn save_read;
    PetPlatformSaveWriteFn save_write;
} PetPlatformCallbacks;

typedef struct {
    char device_id[PET_DEVICE_ID_MAX];
    char model[PET_DEVICE_MODEL_MAX];
    pet_u32_t hardware_version;
    pet_u32_t firmware_version;
} pet_device_identity_t;

typedef struct {
    pet_u16_t x;
    pet_u16_t y;
    pet_u16_t width;
    pet_u16_t height;
} pet_display_rect_t;

typedef struct {
    pet_u8_t uid[PET_PROTOCOL_NFC_UID_MAX];
    pet_u8_t uid_len;
    pet_u8_t type;
    pet_u8_t reserved;
    pet_u32_t card_id;
} pet_nfc_card_t;

typedef struct {
    pet_u8_t peer_device_id[PET_PROTOCOL_DEVICE_ID_LEN];
    pet_u8_t nonce[PET_PROTOCOL_NONCE_LEN];
    pet_u8_t role;
    pet_u8_t reserved[3];
} pet_nfc_pair_t;

typedef struct pet_platform {
    pet_u32_t (*millis)(void *ctx);
    pet_u32_t (*now_sec)(void *ctx);
    pet_result_t (*get_device_identity)(void *ctx, pet_device_identity_t *identity);
    pet_result_t (*get_display_profile)(void *ctx, pet_display_profile_t *profile);
    pet_result_t (*poll_key_event)(void *ctx, pet_key_event_t *event);

    pet_result_t (*display_acquire)(void *ctx, pet_display_owner_t owner, pet_u32_t timeout_ms);
    pet_result_t (*display_release)(void *ctx, pet_display_owner_t owner);
    pet_result_t (*display_flush)(void *ctx, const pet_display_rect_t *rect,
                                  const void *rgb565_pixels, pet_u32_t stride_bytes);
    pet_result_t (*display_wait)(void *ctx, pet_u32_t timeout_ms);
    pet_result_t (*display_set_brightness)(void *ctx, pet_u8_t percent);
    pet_result_t (*display_sleep)(void *ctx);
    pet_result_t (*display_wakeup)(void *ctx);

    pet_result_t (*storage_read)(void *ctx, pet_storage_area_t area, pet_u32_t offset,
                                 void *dst, pet_u32_t len);
    pet_result_t (*storage_write_atomic)(void *ctx, pet_storage_area_t area, pet_u32_t offset,
                                         const void *src, pet_u32_t len);

    pet_result_t (*audio_play_sfx)(void *ctx, pet_u32_t sfx_id, pet_audio_channel_t channel);
    pet_result_t (*audio_stop)(void *ctx, pet_audio_channel_t channel);

    pet_result_t (*ble_send_packet)(void *ctx, const pet_packet_t *packet);
    pet_result_t (*ble_poll_packet)(void *ctx, pet_packet_t *packet);

    pet_result_t (*nfc_start_card_scan)(void *ctx);
    pet_result_t (*nfc_start_pair_scan)(void *ctx);
    pet_result_t (*nfc_poll_card)(void *ctx, pet_nfc_card_t *card);
    pet_result_t (*nfc_poll_pair)(void *ctx, pet_nfc_pair_t *pair);

    pet_result_t (*power_get_battery_percent)(void *ctx, pet_u8_t *percent);
    pet_result_t (*power_get_battery_voltage_mv)(void *ctx, pet_u16_t *voltage_mv);

    void *ctx;
} pet_platform_t;

#if defined(PET_DEBUG)
pet_result_t pet_debug_inject_key_event(const pet_platform_t *platform, const pet_key_event_t *event);
pet_result_t pet_debug_inject_packet(const pet_platform_t *platform, const pet_packet_t *packet);
pet_result_t pet_debug_inject_nfc_card(const pet_platform_t *platform, const pet_nfc_card_t *card);
#endif

#ifdef __cplusplus
}
#endif

#endif
