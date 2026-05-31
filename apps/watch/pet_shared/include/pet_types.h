#ifndef PET_TYPES_H
#define PET_TYPES_H

#include <stddef.h>
#include <stdint.h>

#if !defined(__cplusplus)
#include <stdbool.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define PET_SHARED_INTERFACE_VERSION_MAJOR 1u
#define PET_SHARED_INTERFACE_VERSION_MINOR 0u
#define PET_SHARED_INTERFACE_VERSION_PATCH 0u
#define PET_VERSION_MAKE(major, minor, patch) \
    ((((uint32_t)(major)) << 24) | (((uint32_t)(minor)) << 16) | ((uint32_t)(patch)))
#define PET_SHARED_INTERFACE_VERSION \
    PET_VERSION_MAKE(PET_SHARED_INTERFACE_VERSION_MAJOR, PET_SHARED_INTERFACE_VERSION_MINOR, \
                     PET_SHARED_INTERFACE_VERSION_PATCH)

#define PET_PORTABLE_API_VERSION 1u
#define PET_MAX_COUNT 18u
#define PET_DEVICE_NAME_MAX 16u
#define PET_PET_NAME_MAX 16u
#define PET_ACTIVATED_NFC_MAX 256u
#define PET_VIRTUAL_CARD_MAX 256u
#define PET_CARD_MAX_COUNT PET_VIRTUAL_CARD_MAX
#define PET_EQUIPMENT_MAX 128u
#define PET_HOME_ASSET_MAX 128u
#define PET_PACKET_MAX_PAYLOAD 64u
#define PET_SAVE_SLOT_HEADER_SIZE 64u
#define PET_SPECIES_COUNT 18u
#define PET_SPECIES_NAME_MAX 24u
#define PET_RESOURCE_MANIFEST_VERSION 1u

typedef uint8_t pet_u8_t;
typedef int8_t pet_i8_t;
typedef uint16_t pet_u16_t;
typedef int16_t pet_i16_t;
typedef uint32_t pet_u32_t;
typedef int32_t pet_i32_t;
typedef uint64_t pet_u64_t;
typedef int64_t pet_i64_t;
typedef size_t pet_size_t;
typedef bool pet_bool_t;

#define PET_FALSE ((pet_bool_t)false)
#define PET_TRUE  ((pet_bool_t)true)

typedef enum {
    PET_RESULT_OK = 0,
    PET_RESULT_INVALID_ARGUMENT = 1,
    PET_RESULT_BUFFER_TOO_SMALL = 2,
    PET_RESULT_UNSUPPORTED = 3,
    PET_RESULT_DUPLICATE = 4,
    PET_RESULT_FULL = 5,
    PET_RESULT_BAD_CRC = 6,
    PET_RESULT_BAD_VERSION = 7,
    PET_RESULT_STORAGE_ERROR = 8,
    PET_RESULT_NOT_READY = 9,
    PET_RESULT_BUSY = 10,
    PET_RESULT_TIMEOUT = 11,
    PET_RESULT_NOT_FOUND = 12,
    PET_RESULT_AGAIN = 13,
    PET_RESULT_ERROR = 255
} pet_result_t;

#define PET_RESULT_INVALID_PARAM    PET_RESULT_INVALID_ARGUMENT
#define PET_RESULT_STORAGE_FULL     PET_RESULT_FULL
#define PET_RESULT_CRC_ERROR        PET_RESULT_BAD_CRC
#define PET_RESULT_VERSION_MISMATCH PET_RESULT_BAD_VERSION

typedef enum PetDeviceId {
    PET_DEVICE_A = 0,
    PET_DEVICE_B = 1
} PetDeviceId;

typedef enum PetScreenShape {
    PET_SCREEN_SHAPE_CIRCLE = 0,
    PET_SCREEN_SHAPE_RECTANGLE = 1
} PetScreenShape;

typedef enum PetSaveSlot {
    PET_SAVE_SLOT_A = 0,
    PET_SAVE_SLOT_B = 1,
    PET_SAVE_SLOT_NONE = 255
} PetSaveSlot;

typedef uint32_t PetAssetId;
typedef uint32_t PetAnimationId;
typedef uint32_t PetDeviceShortId;
typedef uint16_t PetResourceId;
typedef uint16_t PetSpeciesId;
typedef uint16_t PetUnlockId;
typedef pet_result_t PetResult;

#define PET_STATIC_ASSERT_JOIN_INNER(a, b) a##b
#define PET_STATIC_ASSERT_JOIN(a, b) PET_STATIC_ASSERT_JOIN_INNER(a, b)
#define PET_STATIC_ASSERT(name, expr) \
    typedef char PET_STATIC_ASSERT_JOIN(pet_static_assert_##name##_, __LINE__)[(expr) ? 1 : -1]

#if defined(_MSC_VER)
#define PET_PACKED_BEGIN __pragma(pack(push, 1))
#define PET_PACKED_END   __pragma(pack(pop))
#define PET_PACKED
#elif defined(__GNUC__) || defined(__clang__)
#define PET_PACKED_BEGIN
#define PET_PACKED_END
#define PET_PACKED __attribute__((packed))
#else
#define PET_PACKED_BEGIN
#define PET_PACKED_END
#define PET_PACKED
#endif

#ifdef __cplusplus
}
#endif

#endif
