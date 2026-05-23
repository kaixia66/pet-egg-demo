#include "mvp_a_assets.h"

#define ASSET_PATH_BACKGROUND(name, file) \
    "assets/background_scenes/backgrounds/" name "/" file
#define ASSET_PATH_CHARACTER(name, file) \
    "assets/character_p0/characters/" name "/" file
#define ASSET_PATH_UI(dir, file) \
    "assets/ui_components/" dir "/" file

static const char * const mvp_a_anim_stage0_idle_frames[] = {
    ASSET_PATH_CHARACTER("stage0_core", "stage0_core_idle_0001.png"),
    ASSET_PATH_CHARACTER("stage0_core", "stage0_core_idle_0002.png"),
    ASSET_PATH_CHARACTER("stage0_core", "stage0_core_idle_0003.png"),
    ASSET_PATH_CHARACTER("stage0_core", "stage0_core_idle_0004.png"),
};

static const char * const mvp_a_anim_stage1_idle_frames[] = {
    ASSET_PATH_CHARACTER("stage1_bun", "stage1_bun_idle_0001.png"),
    ASSET_PATH_CHARACTER("stage1_bun", "stage1_bun_idle_0002.png"),
    ASSET_PATH_CHARACTER("stage1_bun", "stage1_bun_idle_0003.png"),
    ASSET_PATH_CHARACTER("stage1_bun", "stage1_bun_idle_0004.png"),
};

static const char * const mvp_a_anim_stage2_idle_frames[] = {
    ASSET_PATH_CHARACTER("stage2_wood", "stage2_wood_idle_0001.png"),
    ASSET_PATH_CHARACTER("stage2_wood", "stage2_wood_idle_0002.png"),
    ASSET_PATH_CHARACTER("stage2_wood", "stage2_wood_idle_0003.png"),
    ASSET_PATH_CHARACTER("stage2_wood", "stage2_wood_idle_0004.png"),
};

static const char * const mvp_a_anim_stage3_idle_frames[] = {
    ASSET_PATH_CHARACTER("stage3_cocoon", "stage3_cocoon_idle_0001.png"),
    ASSET_PATH_CHARACTER("stage3_cocoon", "stage3_cocoon_idle_0002.png"),
    ASSET_PATH_CHARACTER("stage3_cocoon", "stage3_cocoon_idle_0003.png"),
    ASSET_PATH_CHARACTER("stage3_cocoon", "stage3_cocoon_idle_0004.png"),
};

static const char * const mvp_a_anim_stage4_idle_frames[] = {
    ASSET_PATH_CHARACTER("stage4_qinglong_baby", "stage4_qinglong_baby_idle_0001.png"),
    ASSET_PATH_CHARACTER("stage4_qinglong_baby", "stage4_qinglong_baby_idle_0002.png"),
    ASSET_PATH_CHARACTER("stage4_qinglong_baby", "stage4_qinglong_baby_idle_0003.png"),
    ASSET_PATH_CHARACTER("stage4_qinglong_baby", "stage4_qinglong_baby_idle_0004.png"),
};

static const char * const mvp_a_anim_stage5_idle_frames[] = {
    ASSET_PATH_CHARACTER("stage5_qinglong_final", "stage5_qinglong_final_idle_0001.png"),
    ASSET_PATH_CHARACTER("stage5_qinglong_final", "stage5_qinglong_final_idle_0002.png"),
    ASSET_PATH_CHARACTER("stage5_qinglong_final", "stage5_qinglong_final_idle_0003.png"),
    ASSET_PATH_CHARACTER("stage5_qinglong_final", "stage5_qinglong_final_idle_0004.png"),
};

static const char * const mvp_a_anim_boss_rock_idle_frames[] = {
    ASSET_PATH_CHARACTER("boss_rock", "boss_rock_idle_0001.png"),
    ASSET_PATH_CHARACTER("boss_rock", "boss_rock_idle_0002.png"),
    ASSET_PATH_CHARACTER("boss_rock", "boss_rock_idle_0003.png"),
    ASSET_PATH_CHARACTER("boss_rock", "boss_rock_idle_0004.png"),
};

static const char * const mvp_a_anim_lock_cloud_frames[] = {
    ASSET_PATH_UI("08_lock_cloud", "ui_lock_cloud_pulse_0001.png"),
    ASSET_PATH_UI("08_lock_cloud", "ui_lock_cloud_pulse_0002.png"),
    ASSET_PATH_UI("08_lock_cloud", "ui_lock_cloud_pulse_0003.png"),
    ASSET_PATH_UI("08_lock_cloud", "ui_lock_cloud_pulse_0004.png"),
};

static const char * const mvp_a_anim_connection_frames[] = {
    ASSET_PATH_UI("09_connection_dots", "fx_connection_points_0001.png"),
    ASSET_PATH_UI("09_connection_dots", "fx_connection_points_0002.png"),
    ASSET_PATH_UI("09_connection_dots", "fx_connection_points_0003.png"),
    ASSET_PATH_UI("09_connection_dots", "fx_connection_points_0004.png"),
};

static const char * const mvp_a_anim_qte_flash_frames[] = {
    ASSET_PATH_UI("04_qte_bar", "fx_qte_perfect_flash_0001.png"),
    ASSET_PATH_UI("04_qte_bar", "fx_qte_perfect_flash_0002.png"),
    ASSET_PATH_UI("04_qte_bar", "fx_qte_perfect_flash_0003.png"),
    ASSET_PATH_UI("04_qte_bar", "fx_qte_perfect_flash_0004.png"),
};

static const mvp_a_asset_ref_t mvp_a_asset_table[] = {
    {MVP_A_ASSET_ID_BG_EGG_SPACE, MVP_A_ASSET_KIND_BACKGROUND, "bg_egg_space", ASSET_PATH_BACKGROUND("bg_egg_space", "bg_egg_space_454.png"), ASSET_PATH_BACKGROUND("bg_error", "bg_error_454.png"), 454, 454, 1, 0},
    {MVP_A_ASSET_ID_BG_HOME_DEFAULT_DAY, MVP_A_ASSET_KIND_BACKGROUND, "bg_home_default_day", ASSET_PATH_BACKGROUND("bg_home_default_day", "bg_home_default_day_454.png"), ASSET_PATH_BACKGROUND("bg_error", "bg_error_454.png"), 454, 454, 1, 0},
    {MVP_A_ASSET_ID_BG_HOME_NIGHT_NEST, MVP_A_ASSET_KIND_BACKGROUND, "bg_home_night_nest", ASSET_PATH_BACKGROUND("bg_home_night_nest", "bg_home_night_nest_454.png"), ASSET_PATH_BACKGROUND("bg_home_default_day", "bg_home_default_day_454.png"), 454, 454, 1, 0},
    {MVP_A_ASSET_ID_BG_TRAINING_GROUND, MVP_A_ASSET_KIND_BACKGROUND, "bg_training_ground", ASSET_PATH_BACKGROUND("bg_training_ground", "bg_training_ground_454.png"), ASSET_PATH_BACKGROUND("bg_home_default_day", "bg_home_default_day_454.png"), 454, 454, 1, 0},
    {MVP_A_ASSET_ID_BG_NFC_SPACE, MVP_A_ASSET_KIND_BACKGROUND, "bg_nfc_space", ASSET_PATH_BACKGROUND("bg_nfc_space", "bg_nfc_space_454.png"), ASSET_PATH_BACKGROUND("bg_home_default_day", "bg_home_default_day_454.png"), 454, 454, 1, 0},
    {MVP_A_ASSET_ID_BG_BOSS_ROCK_SPIRIT, MVP_A_ASSET_KIND_BACKGROUND, "bg_boss_rock_spirit", ASSET_PATH_BACKGROUND("bg_boss_rock_spirit", "bg_boss_rock_spirit_454.png"), ASSET_PATH_BACKGROUND("bg_error", "bg_error_454.png"), 454, 454, 1, 0},
    {MVP_A_ASSET_ID_BG_JOURNAL, MVP_A_ASSET_KIND_BACKGROUND, "bg_journal", ASSET_PATH_BACKGROUND("bg_journal", "bg_journal_454.png"), ASSET_PATH_BACKGROUND("bg_home_default_day", "bg_home_default_day_454.png"), 454, 454, 1, 0},
    {MVP_A_ASSET_ID_BG_ERROR, MVP_A_ASSET_KIND_BACKGROUND, "bg_error", ASSET_PATH_BACKGROUND("bg_error", "bg_error_454.png"), ASSET_PATH_BACKGROUND("bg_error", "bg_error_454.png"), 454, 454, 1, 1},

    {MVP_A_ASSET_ID_CHAR_STAGE0_CORE, MVP_A_ASSET_KIND_CHARACTER, "stage0_core", ASSET_PATH_CHARACTER("stage0_core", "stage0_core_static_fallback.png"), ASSET_PATH_CHARACTER("stage0_core", "stage0_core_idle.png"), 454, 454, 1, 0},
    {MVP_A_ASSET_ID_CHAR_STAGE1_BUN, MVP_A_ASSET_KIND_CHARACTER, "stage1_bun", ASSET_PATH_CHARACTER("stage1_bun", "stage1_bun_static_fallback.png"), ASSET_PATH_CHARACTER("stage0_core", "stage0_core_static_fallback.png"), 454, 454, 1, 0},
    {MVP_A_ASSET_ID_CHAR_STAGE2_WOOD, MVP_A_ASSET_KIND_CHARACTER, "stage2_wood", ASSET_PATH_CHARACTER("stage2_wood", "stage2_wood_static_fallback.png"), ASSET_PATH_CHARACTER("stage1_bun", "stage1_bun_static_fallback.png"), 454, 454, 1, 0},
    {MVP_A_ASSET_ID_CHAR_STAGE3_COCOON, MVP_A_ASSET_KIND_CHARACTER, "stage3_cocoon", ASSET_PATH_CHARACTER("stage3_cocoon", "stage3_cocoon_static_fallback.png"), ASSET_PATH_CHARACTER("stage2_wood", "stage2_wood_static_fallback.png"), 454, 454, 1, 0},
    {MVP_A_ASSET_ID_CHAR_STAGE4_QINGLONG_BABY, MVP_A_ASSET_KIND_CHARACTER, "stage4_qinglong_baby", ASSET_PATH_CHARACTER("stage4_qinglong_baby", "stage4_qinglong_baby_static_fallback.png"), ASSET_PATH_CHARACTER("stage3_cocoon", "stage3_cocoon_static_fallback.png"), 454, 454, 1, 0},
    {MVP_A_ASSET_ID_CHAR_STAGE5_QINGLONG_FINAL, MVP_A_ASSET_KIND_CHARACTER, "stage5_qinglong_final", ASSET_PATH_CHARACTER("stage5_qinglong_final", "stage5_qinglong_final_static_fallback.png"), ASSET_PATH_CHARACTER("stage4_qinglong_baby", "stage4_qinglong_baby_static_fallback.png"), 454, 454, 1, 0},
    {MVP_A_ASSET_ID_CHAR_BOSS_ROCK, MVP_A_ASSET_KIND_CHARACTER, "boss_rock", ASSET_PATH_CHARACTER("boss_rock", "boss_rock_static_fallback.png"), ASSET_PATH_CHARACTER("stage5_qinglong_final", "stage5_qinglong_final_static_fallback.png"), 454, 454, 1, 0},

    {MVP_A_ASSET_ID_UI_BUTTON_NORMAL, MVP_A_ASSET_KIND_UI_COMPONENT, "ui_button_normal", ASSET_PATH_UI("05_button_states", "ui_button_normal_180x56.png"), ASSET_PATH_UI("05_button_states", "ui_button_normal_96x46.png"), 180, 56, 1, 0},
    {MVP_A_ASSET_ID_UI_BUTTON_SUCCESS, MVP_A_ASSET_KIND_UI_COMPONENT, "ui_button_success", ASSET_PATH_UI("05_button_states", "ui_button_success_180x56.png"), ASSET_PATH_UI("05_button_states", "ui_button_normal_180x56.png"), 180, 56, 1, 0},
    {MVP_A_ASSET_ID_UI_BUTTON_WARNING, MVP_A_ASSET_KIND_UI_COMPONENT, "ui_button_warning", ASSET_PATH_UI("05_button_states", "ui_button_warning_180x56.png"), ASSET_PATH_UI("05_button_states", "ui_button_normal_180x56.png"), 180, 56, 1, 0},
    {MVP_A_ASSET_ID_UI_BUTTON_ERROR, MVP_A_ASSET_KIND_UI_COMPONENT, "ui_button_error", ASSET_PATH_UI("05_button_states", "ui_button_error_180x56.png"), ASSET_PATH_UI("05_button_states", "ui_button_normal_180x56.png"), 180, 56, 1, 0},
    {MVP_A_ASSET_ID_UI_PROMPT_NOTICE, MVP_A_ASSET_KIND_UI_COMPONENT, "ui_prompt_bubble_notice", ASSET_PATH_UI("07_prompt_bubbles", "ui_prompt_bubble_notice_360x80.png"), ASSET_PATH_UI("07_prompt_bubbles", "ui_prompt_bubble_notice_360x80.png"), 360, 80, 1, 0},
    {MVP_A_ASSET_ID_UI_PROMPT_SUCCESS, MVP_A_ASSET_KIND_UI_COMPONENT, "ui_prompt_bubble_success", ASSET_PATH_UI("07_prompt_bubbles", "ui_prompt_bubble_success_360x80.png"), ASSET_PATH_UI("07_prompt_bubbles", "ui_prompt_bubble_notice_360x80.png"), 360, 80, 1, 0},
    {MVP_A_ASSET_ID_UI_PROMPT_WARNING, MVP_A_ASSET_KIND_UI_COMPONENT, "ui_prompt_bubble_warning", ASSET_PATH_UI("07_prompt_bubbles", "ui_prompt_bubble_warning_360x80.png"), ASSET_PATH_UI("07_prompt_bubbles", "ui_prompt_bubble_notice_360x80.png"), 360, 80, 1, 0},
    {MVP_A_ASSET_ID_UI_PROMPT_ERROR, MVP_A_ASSET_KIND_UI_COMPONENT, "ui_prompt_bubble_error", ASSET_PATH_UI("07_prompt_bubbles", "ui_prompt_bubble_error_360x80.png"), ASSET_PATH_UI("07_prompt_bubbles", "ui_prompt_bubble_notice_360x80.png"), 360, 80, 1, 0},
    {MVP_A_ASSET_ID_UI_LOCK_CLOUD_LOCKED, MVP_A_ASSET_KIND_UI_COMPONENT, "ui_lock_cloud_locked", ASSET_PATH_UI("08_lock_cloud", "ui_lock_cloud_locked_170x90.png"), ASSET_PATH_UI("08_lock_cloud", "ui_lock_cloud_unlocked_170x90.png"), 170, 90, 1, 0},
    {MVP_A_ASSET_ID_UI_LOCK_CLOUD_UNLOCKED, MVP_A_ASSET_KIND_UI_COMPONENT, "ui_lock_cloud_unlocked", ASSET_PATH_UI("08_lock_cloud", "ui_lock_cloud_unlocked_170x90.png"), ASSET_PATH_UI("08_lock_cloud", "ui_lock_cloud_unlocked_170x90.png"), 170, 90, 1, 0},
    {MVP_A_ASSET_ID_UI_CONNECT_DOT_IDLE, MVP_A_ASSET_KIND_UI_COMPONENT, "ui_connect_dot_idle", ASSET_PATH_UI("09_connection_dots", "ui_connect_dot_idle_42x42.png"), ASSET_PATH_UI("09_connection_dots", "ui_connect_dot_idle_42x42.png"), 42, 42, 1, 0},
    {MVP_A_ASSET_ID_UI_CONNECT_DOT_ACTIVE, MVP_A_ASSET_KIND_UI_COMPONENT, "ui_connect_dot_active", ASSET_PATH_UI("09_connection_dots", "ui_connect_dot_active_42x42.png"), ASSET_PATH_UI("09_connection_dots", "ui_connect_dot_idle_42x42.png"), 42, 42, 1, 0},
    {MVP_A_ASSET_ID_UI_CONNECT_DOT_SUCCESS, MVP_A_ASSET_KIND_UI_COMPONENT, "ui_connect_dot_success", ASSET_PATH_UI("09_connection_dots", "ui_connect_dot_success_42x42.png"), ASSET_PATH_UI("09_connection_dots", "ui_connect_dot_idle_42x42.png"), 42, 42, 1, 0},
    {MVP_A_ASSET_ID_UI_CONNECT_DOT_ERROR, MVP_A_ASSET_KIND_UI_COMPONENT, "ui_connect_dot_error", ASSET_PATH_UI("09_connection_dots", "ui_connect_dot_error_42x42.png"), ASSET_PATH_UI("09_connection_dots", "ui_connect_dot_idle_42x42.png"), 42, 42, 1, 0},
    {MVP_A_ASSET_ID_UI_EXCEPTION_LOW_BATTERY, MVP_A_ASSET_KIND_UI_COMPONENT, "ui_exception_low_battery", ASSET_PATH_UI("10_exception_components", "ui_exception_low_battery_128x64.png"), ASSET_PATH_UI("10_exception_components", "ui_exception_panel_warning_360x92.png"), 128, 64, 1, 0},

    {MVP_A_ASSET_ID_ICON_HOME, MVP_A_ASSET_KIND_ICON, "icon_home", ASSET_PATH_UI("06_icons", "icon_home_64.png"), ASSET_PATH_UI("06_icons", "icon_star_64.png"), 64, 64, 1, 0},
    {MVP_A_ASSET_ID_ICON_TRAIN, MVP_A_ASSET_KIND_ICON, "icon_train", ASSET_PATH_UI("06_icons", "icon_train_64.png"), ASSET_PATH_UI("06_icons", "icon_star_64.png"), 64, 64, 1, 0},
    {MVP_A_ASSET_ID_ICON_CARDBAG, MVP_A_ASSET_KIND_ICON, "icon_cardbag", ASSET_PATH_UI("06_icons", "icon_cardbag_64.png"), ASSET_PATH_UI("06_icons", "icon_star_64.png"), 64, 64, 1, 0},
    {MVP_A_ASSET_ID_ICON_NFC, MVP_A_ASSET_KIND_ICON, "icon_nfc", ASSET_PATH_UI("06_icons", "icon_nfc_64.png"), ASSET_PATH_UI("06_icons", "icon_connect_64.png"), 64, 64, 1, 0},
    {MVP_A_ASSET_ID_ICON_BOSS, MVP_A_ASSET_KIND_ICON, "icon_boss", ASSET_PATH_UI("06_icons", "icon_boss_64.png"), ASSET_PATH_UI("06_icons", "icon_warning_64.png"), 64, 64, 1, 0},
    {MVP_A_ASSET_ID_ICON_STAR, MVP_A_ASSET_KIND_ICON, "icon_star", ASSET_PATH_UI("06_icons", "icon_star_64.png"), ASSET_PATH_UI("06_icons", "icon_notice_64.png"), 64, 64, 1, 0},
    {MVP_A_ASSET_ID_ICON_WATER, MVP_A_ASSET_KIND_ICON, "icon_water", ASSET_PATH_UI("06_icons", "icon_water_64.png"), ASSET_PATH_UI("06_icons", "icon_star_64.png"), 64, 64, 1, 0},
    {MVP_A_ASSET_ID_ICON_SLEEP, MVP_A_ASSET_KIND_ICON, "icon_sleep", ASSET_PATH_UI("06_icons", "icon_sleep_64.png"), ASSET_PATH_UI("06_icons", "icon_star_64.png"), 64, 64, 1, 0},
    {MVP_A_ASSET_ID_ICON_CONFIRM, MVP_A_ASSET_KIND_ICON, "icon_confirm_check", ASSET_PATH_UI("06_icons", "icon_confirm_check_64.png"), ASSET_PATH_UI("06_icons", "icon_star_64.png"), 64, 64, 1, 0},
    {MVP_A_ASSET_ID_ICON_BACK, MVP_A_ASSET_KIND_ICON, "icon_back_cross", ASSET_PATH_UI("06_icons", "icon_back_cross_64.png"), ASSET_PATH_UI("06_icons", "icon_star_64.png"), 64, 64, 1, 0},
};

static const mvp_a_animation_ref_t mvp_a_animation_table[] = {
    {MVP_A_ASSET_ID_ANIM_STAGE0_IDLE, "stage0_core.idle_loop", ASSET_PATH_CHARACTER("stage0_core", "stage0_core_sprite_sheet.png"), ASSET_PATH_CHARACTER("stage0_core", "stage0_core_static_fallback.png"), mvp_a_anim_stage0_idle_frames, 4, 8, 1, 0},
    {MVP_A_ASSET_ID_ANIM_STAGE1_IDLE, "stage1_bun.idle_loop", ASSET_PATH_CHARACTER("stage1_bun", "stage1_bun_sprite_sheet.png"), ASSET_PATH_CHARACTER("stage1_bun", "stage1_bun_static_fallback.png"), mvp_a_anim_stage1_idle_frames, 4, 8, 1, 0},
    {MVP_A_ASSET_ID_ANIM_STAGE2_IDLE, "stage2_wood.idle_loop", ASSET_PATH_CHARACTER("stage2_wood", "stage2_wood_sprite_sheet.png"), ASSET_PATH_CHARACTER("stage2_wood", "stage2_wood_static_fallback.png"), mvp_a_anim_stage2_idle_frames, 4, 8, 1, 0},
    {MVP_A_ASSET_ID_ANIM_STAGE3_IDLE, "stage3_cocoon.idle_loop", ASSET_PATH_CHARACTER("stage3_cocoon", "stage3_cocoon_sprite_sheet.png"), ASSET_PATH_CHARACTER("stage3_cocoon", "stage3_cocoon_static_fallback.png"), mvp_a_anim_stage3_idle_frames, 4, 8, 1, 0},
    {MVP_A_ASSET_ID_ANIM_STAGE4_IDLE, "stage4_qinglong_baby.idle_loop", ASSET_PATH_CHARACTER("stage4_qinglong_baby", "stage4_qinglong_baby_sprite_sheet.png"), ASSET_PATH_CHARACTER("stage4_qinglong_baby", "stage4_qinglong_baby_static_fallback.png"), mvp_a_anim_stage4_idle_frames, 4, 8, 1, 0},
    {MVP_A_ASSET_ID_ANIM_STAGE5_IDLE, "stage5_qinglong_final.idle_loop", ASSET_PATH_CHARACTER("stage5_qinglong_final", "stage5_qinglong_final_sprite_sheet.png"), ASSET_PATH_CHARACTER("stage5_qinglong_final", "stage5_qinglong_final_static_fallback.png"), mvp_a_anim_stage5_idle_frames, 4, 8, 1, 0},
    {MVP_A_ASSET_ID_ANIM_BOSS_ROCK_IDLE, "boss_rock.idle_loop", ASSET_PATH_CHARACTER("boss_rock", "boss_rock_sprite_sheet.png"), ASSET_PATH_CHARACTER("boss_rock", "boss_rock_static_fallback.png"), mvp_a_anim_boss_rock_idle_frames, 4, 8, 1, 0},
    {MVP_A_ASSET_ID_ANIM_UI_LOCK_CLOUD_PULSE, "ui_lock_cloud_pulse", NULL, ASSET_PATH_UI("08_lock_cloud", "ui_lock_cloud_locked_170x90.png"), mvp_a_anim_lock_cloud_frames, 4, 8, 1, 0},
    {MVP_A_ASSET_ID_ANIM_FX_CONNECTION_POINTS, "fx_connection_points", NULL, ASSET_PATH_UI("09_connection_dots", "ui_connect_dot_idle_42x42.png"), mvp_a_anim_connection_frames, 4, 8, 1, 0},
    {MVP_A_ASSET_ID_ANIM_FX_QTE_PERFECT_FLASH, "fx_qte_perfect_flash", NULL, ASSET_PATH_UI("04_qte_bar", "qte_bar_idle_320x24.png"), mvp_a_anim_qte_flash_frames, 4, 10, 0, 0},
};

static const mvp_a_asset_ref_t mvp_a_missing_asset = {
    MVP_A_ASSET_ID_BG_ERROR,
    MVP_A_ASSET_KIND_BACKGROUND,
    "fallback_missing_resource",
    ASSET_PATH_BACKGROUND("bg_error", "bg_error_454.png"),
    ASSET_PATH_BACKGROUND("bg_error", "bg_error_454.png"),
    454,
    454,
    1,
    1,
};

static const mvp_a_animation_ref_t mvp_a_missing_animation = {
    MVP_A_ASSET_ID_ANIM_STAGE0_IDLE,
    "fallback_stage0_idle",
    ASSET_PATH_CHARACTER("stage0_core", "stage0_core_sprite_sheet.png"),
    ASSET_PATH_CHARACTER("stage0_core", "stage0_core_static_fallback.png"),
    mvp_a_anim_stage0_idle_frames,
    4,
    8,
    1,
    1,
};

const char *mvp_a_assets_root(void)
{
    return MVP_A_ASSET_ROOT;
}

const mvp_a_asset_ref_t *mvp_a_assets_get(mvp_a_asset_id_t id)
{
    u32 i;

    for (i = 0; i < (u32)(sizeof(mvp_a_asset_table) / sizeof(mvp_a_asset_table[0])); i++) {
        if (mvp_a_asset_table[i].id == id) {
            return &mvp_a_asset_table[i];
        }
    }

    return &mvp_a_missing_asset;
}

const mvp_a_asset_ref_t *mvp_a_assets_background_for_scene(mvp_a_scene_t scene)
{
    switch (scene) {
    case MVP_A_SCENE_BOOT:
        return mvp_a_assets_get(MVP_A_ASSET_ID_BG_EGG_SPACE);
    case MVP_A_SCENE_HOME:
        return mvp_a_assets_get(MVP_A_ASSET_ID_BG_HOME_DEFAULT_DAY);
    case MVP_A_SCENE_CARE:
        return mvp_a_assets_get(MVP_A_ASSET_ID_BG_HOME_NIGHT_NEST);
    case MVP_A_SCENE_TRAINING:
        return mvp_a_assets_get(MVP_A_ASSET_ID_BG_TRAINING_GROUND);
    case MVP_A_SCENE_CARD_BAG:
        return mvp_a_assets_get(MVP_A_ASSET_ID_BG_HOME_NIGHT_NEST);
    case MVP_A_SCENE_NFC_READ:
        return mvp_a_assets_get(MVP_A_ASSET_ID_BG_NFC_SPACE);
    case MVP_A_SCENE_COOP_WAIT:
        return mvp_a_assets_get(MVP_A_ASSET_ID_BG_HOME_DEFAULT_DAY);
    case MVP_A_SCENE_BOSS:
        return mvp_a_assets_get(MVP_A_ASSET_ID_BG_BOSS_ROCK_SPIRIT);
    case MVP_A_SCENE_DIARY:
        return mvp_a_assets_get(MVP_A_ASSET_ID_BG_JOURNAL);
    case MVP_A_SCENE_DEBUG:
        return mvp_a_assets_get(MVP_A_ASSET_ID_BG_ERROR);
    default:
        break;
    }

    return &mvp_a_missing_asset;
}

const mvp_a_asset_ref_t *mvp_a_assets_character_for_stage(mvp_a_pet_stage_t stage)
{
    switch (stage) {
    case MVP_A_PET_STAGE_EGG:
    case MVP_A_PET_STAGE_CORE:
        return mvp_a_assets_get(MVP_A_ASSET_ID_CHAR_STAGE0_CORE);
    case MVP_A_PET_STAGE_BUN:
        return mvp_a_assets_get(MVP_A_ASSET_ID_CHAR_STAGE1_BUN);
    case MVP_A_PET_STAGE_WOOD:
        return mvp_a_assets_get(MVP_A_ASSET_ID_CHAR_STAGE2_WOOD);
    case MVP_A_PET_STAGE_COCOON:
        return mvp_a_assets_get(MVP_A_ASSET_ID_CHAR_STAGE3_COCOON);
    case MVP_A_PET_STAGE_QINGLONG:
        return mvp_a_assets_get(MVP_A_ASSET_ID_CHAR_STAGE5_QINGLONG_FINAL);
    default:
        break;
    }

    return mvp_a_assets_get(MVP_A_ASSET_ID_CHAR_STAGE0_CORE);
}

const mvp_a_asset_ref_t *mvp_a_assets_ui_component(mvp_a_ui_component_t component)
{
    static const mvp_a_asset_id_t component_map[MVP_A_UI_COMPONENT_MAX] = {
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
    };

    if (component >= MVP_A_UI_COMPONENT_MAX) {
        return mvp_a_assets_get(MVP_A_ASSET_ID_UI_PROMPT_ERROR);
    }

    return mvp_a_assets_get(component_map[component]);
}

const mvp_a_asset_ref_t *mvp_a_assets_icon(mvp_a_icon_t icon)
{
    static const mvp_a_asset_id_t icon_map[MVP_A_ICON_MAX] = {
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
    };

    if (icon >= MVP_A_ICON_MAX) {
        return mvp_a_assets_get(MVP_A_ASSET_ID_ICON_STAR);
    }

    return mvp_a_assets_get(icon_map[icon]);
}

const mvp_a_animation_ref_t *mvp_a_assets_animation_group(mvp_a_asset_id_t group_id)
{
    u32 i;

    for (i = 0; i < (u32)(sizeof(mvp_a_animation_table) / sizeof(mvp_a_animation_table[0])); i++) {
        if (mvp_a_animation_table[i].id == group_id) {
            return &mvp_a_animation_table[i];
        }
    }

    return &mvp_a_missing_animation;
}

const mvp_a_animation_ref_t *mvp_a_assets_animation_for_stage(mvp_a_pet_stage_t stage,
                                                              mvp_a_animation_type_t type)
{
    if (type != MVP_A_ANIMATION_IDLE_LOOP) {
        return &mvp_a_missing_animation;
    }

    switch (stage) {
    case MVP_A_PET_STAGE_EGG:
    case MVP_A_PET_STAGE_CORE:
        return mvp_a_assets_animation_group(MVP_A_ASSET_ID_ANIM_STAGE0_IDLE);
    case MVP_A_PET_STAGE_BUN:
        return mvp_a_assets_animation_group(MVP_A_ASSET_ID_ANIM_STAGE1_IDLE);
    case MVP_A_PET_STAGE_WOOD:
        return mvp_a_assets_animation_group(MVP_A_ASSET_ID_ANIM_STAGE2_IDLE);
    case MVP_A_PET_STAGE_COCOON:
        return mvp_a_assets_animation_group(MVP_A_ASSET_ID_ANIM_STAGE3_IDLE);
    case MVP_A_PET_STAGE_QINGLONG:
        return mvp_a_assets_animation_group(MVP_A_ASSET_ID_ANIM_STAGE5_IDLE);
    default:
        break;
    }

    return &mvp_a_missing_animation;
}

const char *mvp_a_assets_stage_name(mvp_a_pet_stage_t stage)
{
    switch (stage) {
    case MVP_A_PET_STAGE_EGG:
        return "Stage0 Egg";
    case MVP_A_PET_STAGE_CORE:
        return "Stage1 Core";
    case MVP_A_PET_STAGE_BUN:
        return "Stage2 Bun";
    case MVP_A_PET_STAGE_WOOD:
        return "Stage3 Wood";
    case MVP_A_PET_STAGE_COCOON:
        return "Stage4 Cocoon";
    case MVP_A_PET_STAGE_QINGLONG:
        return "Stage5 Qinglong";
    default:
        break;
    }

    return "Stage?";
}
