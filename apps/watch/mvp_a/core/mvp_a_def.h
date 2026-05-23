#ifndef MVP_A_DEF_H
#define MVP_A_DEF_H

#include "typedef.h"

typedef enum {
    MVP_A_FALSE = 0,
    MVP_A_TRUE = 1,
} mvp_a_bool_t;

typedef enum {
    MVP_A_RESULT_OK = 0,
    MVP_A_RESULT_BUSY,
    MVP_A_RESULT_TIMEOUT,
    MVP_A_RESULT_NOT_READY,
    MVP_A_RESULT_NOT_FOUND,
    MVP_A_RESULT_DUPLICATE,
    MVP_A_RESULT_STORAGE_FULL,
    MVP_A_RESULT_STORAGE_ERROR,
    MVP_A_RESULT_INVALID_PARAM,
    MVP_A_RESULT_ERROR,
} mvp_a_result_t;

typedef enum {
    MVP_A_PET_STAGE_EGG = 0,
    MVP_A_PET_STAGE_CORE,
    MVP_A_PET_STAGE_BUN,
    MVP_A_PET_STAGE_WOOD,
    MVP_A_PET_STAGE_COCOON,
    MVP_A_PET_STAGE_QINGLONG,
    MVP_A_PET_STAGE_BABY_QINGLONG = MVP_A_PET_STAGE_QINGLONG,
    MVP_A_PET_STAGE_MAX,
} mvp_a_pet_stage_t;

typedef enum {
    MVP_A_SCENE_BOOT = 0,
    MVP_A_SCENE_HOME,
    MVP_A_SCENE_CARE,
    MVP_A_SCENE_TRAINING,
    MVP_A_SCENE_CARD_BAG,
    MVP_A_SCENE_NFC_READ,
    MVP_A_SCENE_COOP_WAIT,
    MVP_A_SCENE_BOSS,
    MVP_A_SCENE_DIARY,
    MVP_A_SCENE_DEBUG,
    MVP_A_SCENE_MAX,
} mvp_a_scene_t;

#define MVP_A_SCENE_GROWTH MVP_A_SCENE_CARE

typedef enum {
    MVP_A_KEY_UP = 0,
    MVP_A_KEY_DOWN,
    MVP_A_KEY_CONFIRM,
    MVP_A_KEY_BACK,
    MVP_A_KEY_MAX,
} mvp_a_key_t;

typedef enum {
    MVP_A_KEY_EVENT_CLICK = 0,
    MVP_A_KEY_EVENT_LONG,
    MVP_A_KEY_EVENT_HOLD,
    MVP_A_KEY_EVENT_RELEASE,
    MVP_A_KEY_EVENT_MAX,
} mvp_a_key_event_t;

typedef enum {
    MVP_A_CARE_COMPANION = 0,
    MVP_A_CARE_CLEAN,
    MVP_A_CARE_REST,
    MVP_A_CARE_MAX,
} mvp_a_care_action_t;

typedef enum {
    MVP_A_TRAINING_COURAGE = 0,
    MVP_A_TRAINING_GUARD,
    MVP_A_TRAINING_MAX,
} mvp_a_training_type_t;

typedef enum {
    MVP_A_CARD_TYPE_COMPANION = 0,
    MVP_A_CARD_TYPE_EQUIPMENT,
    MVP_A_CARD_TYPE_MAX,
} mvp_a_card_type_t;

typedef enum {
    MVP_A_BOSS_ACTION_COURAGE = 0,
    MVP_A_BOSS_ACTION_GUARD,
    MVP_A_BOSS_ACTION_MAX,
} mvp_a_boss_action_t;

typedef enum {
    MVP_A_DIARY_FIRST_WAKE = (1UL << 0),
    MVP_A_DIARY_FIRST_COMPANION = (1UL << 1),
    MVP_A_DIARY_FIRST_TRAINING = (1UL << 2),
    MVP_A_DIARY_FIRST_COOP_BOSS = (1UL << 3),
    MVP_A_DIARY_FIRST_BOSS_WIN = (1UL << 4),
    MVP_A_DIARY_COCOON_BREAK = (1UL << 5),
    MVP_A_DIARY_FINAL_FORM = (1UL << 6),
    MVP_A_DIARY_FIRST_CLEAN = (1UL << 7),
    MVP_A_DIARY_FIRST_REST = (1UL << 8),
    MVP_A_DIARY_STAGE_1_CORE = (1UL << 9),
    MVP_A_DIARY_STAGE_2_BUN = (1UL << 10),
    MVP_A_DIARY_STAGE_3_WOOD = (1UL << 11),
    MVP_A_DIARY_STAGE_4_COCOON = (1UL << 12),
    MVP_A_DIARY_STAGE_5_QINGLONG = (1UL << 13),
    MVP_A_DIARY_FAST_GROWTH = (1UL << 14),
    MVP_A_DIARY_FIRST_HOME = (1UL << 15),
} mvp_a_diary_flag_t;

#define MVP_A_SAVE_MAGIC          0x4D565041UL
#define MVP_A_SAVE_VERSION        1
#define MVP_A_CARD_ID_LEN         24
#define MVP_A_CARD_NAME_LEN       24
#define MVP_A_CARD_MAX            8
#define MVP_A_PET_NICKNAME_LEN    16

typedef struct {
    char card_id[MVP_A_CARD_ID_LEN];
    char card_name[MVP_A_CARD_NAME_LEN];
    u8 card_type;
    u8 version;
    u8 used;
    u8 reserved;
} mvp_a_card_data_t;

typedef struct {
    u32 magic;
    u16 version;
    u16 data_len;
    u8 initialized;
    u8 first_wake_done;
    u8 pet_stage;
    u8 pet_mood;
    u8 pet_energy;
    u8 growth_points;
    u8 pet_clean;
    u8 card_count;
    u8 boss_unlocked;
    u8 boss_win_count;
    u8 reserved[2];
    u32 diary_flags;
    char pet_nickname[MVP_A_PET_NICKNAME_LEN];
    mvp_a_card_data_t cards[MVP_A_CARD_MAX];
    u32 checksum;
} mvp_a_save_data_t;

#endif
