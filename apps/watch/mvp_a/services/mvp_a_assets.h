#ifndef MVP_A_ASSETS_H
#define MVP_A_ASSETS_H

#include "mvp_a_def.h"

#ifndef MVP_A_ASSET_ROOT
#define MVP_A_ASSET_ROOT "apps/watch/mvp_a/resources/MVP-A_Final_Engineering_Resources_V0.1"
#endif

typedef enum {
    MVP_A_ASSET_KIND_BACKGROUND = 0,
    MVP_A_ASSET_KIND_CHARACTER,
    MVP_A_ASSET_KIND_UI_COMPONENT,
    MVP_A_ASSET_KIND_ICON,
    MVP_A_ASSET_KIND_ANIMATION_GROUP,
} mvp_a_asset_kind_t;

typedef enum {
    MVP_A_ASSET_ID_NONE = 0,

    MVP_A_ASSET_ID_BG_EGG_SPACE,
    MVP_A_ASSET_ID_BG_HOME_DEFAULT_DAY,
    MVP_A_ASSET_ID_BG_HOME_NIGHT_NEST,
    MVP_A_ASSET_ID_BG_TRAINING_GROUND,
    MVP_A_ASSET_ID_BG_NFC_SPACE,
    MVP_A_ASSET_ID_BG_BOSS_ROCK_SPIRIT,
    MVP_A_ASSET_ID_BG_JOURNAL,
    MVP_A_ASSET_ID_BG_ERROR,

    MVP_A_ASSET_ID_CHAR_STAGE0_CORE,
    MVP_A_ASSET_ID_CHAR_STAGE1_BUN,
    MVP_A_ASSET_ID_CHAR_STAGE2_WOOD,
    MVP_A_ASSET_ID_CHAR_STAGE3_COCOON,
    MVP_A_ASSET_ID_CHAR_STAGE4_QINGLONG_BABY,
    MVP_A_ASSET_ID_CHAR_STAGE5_QINGLONG_FINAL,
    MVP_A_ASSET_ID_CHAR_BOSS_ROCK,

    MVP_A_ASSET_ID_UI_BUTTON_NORMAL,
    MVP_A_ASSET_ID_UI_BUTTON_SUCCESS,
    MVP_A_ASSET_ID_UI_BUTTON_WARNING,
    MVP_A_ASSET_ID_UI_BUTTON_ERROR,
    MVP_A_ASSET_ID_UI_PROMPT_NOTICE,
    MVP_A_ASSET_ID_UI_PROMPT_SUCCESS,
    MVP_A_ASSET_ID_UI_PROMPT_WARNING,
    MVP_A_ASSET_ID_UI_PROMPT_ERROR,
    MVP_A_ASSET_ID_UI_LOCK_CLOUD_LOCKED,
    MVP_A_ASSET_ID_UI_LOCK_CLOUD_UNLOCKED,
    MVP_A_ASSET_ID_UI_CONNECT_DOT_IDLE,
    MVP_A_ASSET_ID_UI_CONNECT_DOT_ACTIVE,
    MVP_A_ASSET_ID_UI_CONNECT_DOT_SUCCESS,
    MVP_A_ASSET_ID_UI_CONNECT_DOT_ERROR,
    MVP_A_ASSET_ID_UI_EXCEPTION_LOW_BATTERY,

    MVP_A_ASSET_ID_ICON_HOME,
    MVP_A_ASSET_ID_ICON_TRAIN,
    MVP_A_ASSET_ID_ICON_CARDBAG,
    MVP_A_ASSET_ID_ICON_NFC,
    MVP_A_ASSET_ID_ICON_BOSS,
    MVP_A_ASSET_ID_ICON_STAR,
    MVP_A_ASSET_ID_ICON_WATER,
    MVP_A_ASSET_ID_ICON_SLEEP,
    MVP_A_ASSET_ID_ICON_CONFIRM,
    MVP_A_ASSET_ID_ICON_BACK,

    MVP_A_ASSET_ID_ANIM_STAGE0_IDLE,
    MVP_A_ASSET_ID_ANIM_STAGE1_IDLE,
    MVP_A_ASSET_ID_ANIM_STAGE2_IDLE,
    MVP_A_ASSET_ID_ANIM_STAGE3_IDLE,
    MVP_A_ASSET_ID_ANIM_STAGE4_IDLE,
    MVP_A_ASSET_ID_ANIM_STAGE5_IDLE,
    MVP_A_ASSET_ID_ANIM_BOSS_ROCK_IDLE,
    MVP_A_ASSET_ID_ANIM_UI_LOCK_CLOUD_PULSE,
    MVP_A_ASSET_ID_ANIM_FX_CONNECTION_POINTS,
    MVP_A_ASSET_ID_ANIM_FX_QTE_PERFECT_FLASH,

    MVP_A_ASSET_ID_MAX,
} mvp_a_asset_id_t;

typedef enum {
    MVP_A_UI_COMPONENT_BUTTON_NORMAL = 0,
    MVP_A_UI_COMPONENT_BUTTON_SUCCESS,
    MVP_A_UI_COMPONENT_BUTTON_WARNING,
    MVP_A_UI_COMPONENT_BUTTON_ERROR,
    MVP_A_UI_COMPONENT_PROMPT_NOTICE,
    MVP_A_UI_COMPONENT_PROMPT_SUCCESS,
    MVP_A_UI_COMPONENT_PROMPT_WARNING,
    MVP_A_UI_COMPONENT_PROMPT_ERROR,
    MVP_A_UI_COMPONENT_LOCK_CLOUD_LOCKED,
    MVP_A_UI_COMPONENT_LOCK_CLOUD_UNLOCKED,
    MVP_A_UI_COMPONENT_CONNECT_DOT_IDLE,
    MVP_A_UI_COMPONENT_CONNECT_DOT_ACTIVE,
    MVP_A_UI_COMPONENT_CONNECT_DOT_SUCCESS,
    MVP_A_UI_COMPONENT_CONNECT_DOT_ERROR,
    MVP_A_UI_COMPONENT_EXCEPTION_LOW_BATTERY,
    MVP_A_UI_COMPONENT_MAX,
} mvp_a_ui_component_t;

typedef enum {
    MVP_A_ICON_HOME = 0,
    MVP_A_ICON_TRAIN,
    MVP_A_ICON_CARDBAG,
    MVP_A_ICON_NFC,
    MVP_A_ICON_BOSS,
    MVP_A_ICON_STAR,
    MVP_A_ICON_WATER,
    MVP_A_ICON_SLEEP,
    MVP_A_ICON_CONFIRM,
    MVP_A_ICON_BACK,
    MVP_A_ICON_MAX,
} mvp_a_icon_t;

typedef enum {
    MVP_A_ANIMATION_IDLE_LOOP = 0,
    MVP_A_ANIMATION_HAPPY_BOUNCE,
    MVP_A_ANIMATION_SLEEP_BREATH,
    MVP_A_ANIMATION_TRAIN_ACTION,
    MVP_A_ANIMATION_BOSS_ACTION,
    MVP_A_ANIMATION_LOCK_CLOUD_PULSE,
    MVP_A_ANIMATION_CONNECTION_POINTS,
    MVP_A_ANIMATION_QTE_PERFECT_FLASH,
    MVP_A_ANIMATION_MAX,
} mvp_a_animation_type_t;

typedef struct {
    mvp_a_asset_id_t id;
    mvp_a_asset_kind_t kind;
    const char *name;
    const char *path;
    const char *fallback_path;
    u16 width;
    u16 height;
    u8 has_alpha;
    u8 is_fallback;
} mvp_a_asset_ref_t;

typedef struct {
    mvp_a_asset_id_t id;
    const char *name;
    const char *sprite_sheet_path;
    const char *fallback_path;
    const char * const *frame_paths;
    u8 frame_count;
    u8 fps;
    u8 loop;
    u8 is_fallback;
} mvp_a_animation_ref_t;

const char *mvp_a_assets_root(void);
const mvp_a_asset_ref_t *mvp_a_assets_get(mvp_a_asset_id_t id);
const mvp_a_asset_ref_t *mvp_a_assets_background_for_scene(mvp_a_scene_t scene);
const mvp_a_asset_ref_t *mvp_a_assets_character_for_stage(mvp_a_pet_stage_t stage);
const mvp_a_asset_ref_t *mvp_a_assets_ui_component(mvp_a_ui_component_t component);
const mvp_a_asset_ref_t *mvp_a_assets_icon(mvp_a_icon_t icon);
const mvp_a_animation_ref_t *mvp_a_assets_animation_group(mvp_a_asset_id_t group_id);
const mvp_a_animation_ref_t *mvp_a_assets_animation_for_stage(mvp_a_pet_stage_t stage,
                                                              mvp_a_animation_type_t type);
const char *mvp_a_assets_stage_name(mvp_a_pet_stage_t stage);

#endif
