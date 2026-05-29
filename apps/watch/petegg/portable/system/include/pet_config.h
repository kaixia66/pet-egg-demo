#ifndef PETEGG_PORTABLE_PET_CONFIG_H_
#define PETEGG_PORTABLE_PET_CONFIG_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

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

typedef uint32_t PetAssetId;
typedef uint32_t PetAnimationId;
typedef uint32_t PetDeviceShortId;
typedef uint16_t PetResourceId;
typedef uint16_t PetSpeciesId;
typedef uint16_t PetUnlockId;

#ifdef __cplusplus
}
#endif

#endif  /* PETEGG_PORTABLE_PET_CONFIG_H_ */
