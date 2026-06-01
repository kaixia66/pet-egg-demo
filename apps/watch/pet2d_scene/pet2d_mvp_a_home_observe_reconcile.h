#ifndef PET2D_MVP_A_HOME_OBSERVE_RECONCILE_H
#define PET2D_MVP_A_HOME_OBSERVE_RECONCILE_H

#include "pet2d_mvp_a_home_observe_imported.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PET2D_MVP_A_HOME_OBSERVE_RECONCILE_SCENE_EXPORT_VERSION 1u
#define PET2D_MVP_A_HOME_OBSERVE_RECONCILE_SCENE_VERSION 1u
#define PET2D_MVP_A_HOME_OBSERVE_RECONCILE_EXACT_MATCH_COUNT 20u
#define PET2D_MVP_A_HOME_OBSERVE_RECONCILE_SEMANTIC_MATCH_COUNT 5u
#define PET2D_MVP_A_HOME_OBSERVE_RECONCILE_NON_MATCH_COUNT 7u

#define PET2D_MVP_A_HOME_OBSERVE_RECONCILE_SIM_STAGE_W 160u
#define PET2D_MVP_A_HOME_OBSERVE_RECONCILE_SIM_STAGE_H 96u
#define PET2D_MVP_A_HOME_OBSERVE_RECONCILE_SIM_VIEWPORT_W 160u
#define PET2D_MVP_A_HOME_OBSERVE_RECONCILE_SIM_VIEWPORT_H 96u
#define PET2D_MVP_A_HOME_OBSERVE_RECONCILE_SIM_PET_W 32u
#define PET2D_MVP_A_HOME_OBSERVE_RECONCILE_SIM_PET_H 32u
#define PET2D_MVP_A_HOME_OBSERVE_RECONCILE_SIM_INITIAL_X 64
#define PET2D_MVP_A_HOME_OBSERVE_RECONCILE_SIM_INITIAL_Y 32
#define PET2D_MVP_A_HOME_OBSERVE_RECONCILE_SIM_MOVE_STEP 8u
#define PET2D_MVP_A_HOME_OBSERVE_RECONCILE_SIM_MOVE_MS 160u
#define PET2D_MVP_A_HOME_OBSERVE_RECONCILE_SIM_ACTION_MS 300u
#define PET2D_MVP_A_HOME_OBSERVE_RECONCILE_SIM_TIMEOUT_MS 4000u

#define PET2D_MVP_A_HOME_OBSERVE_RECONCILE_DIRTY_ENTER_W 160u
#define PET2D_MVP_A_HOME_OBSERVE_RECONCILE_DIRTY_ENTER_H 96u
#define PET2D_MVP_A_HOME_OBSERVE_RECONCILE_DIRTY_MOVE_W 40u
#define PET2D_MVP_A_HOME_OBSERVE_RECONCILE_DIRTY_MOVE_H 32u
#define PET2D_MVP_A_HOME_OBSERVE_RECONCILE_DIRTY_ACTION_W 32u
#define PET2D_MVP_A_HOME_OBSERVE_RECONCILE_DIRTY_ACTION_H 32u

typedef enum {
    PET2D_MVP_A_RECONCILE_STATUS_EXACT_MATCH = 0,
    PET2D_MVP_A_RECONCILE_STATUS_SEMANTIC_MATCH,
    PET2D_MVP_A_RECONCILE_STATUS_EXPLICIT_NON_MATCH
} pet2d_mvp_a_reconcile_status_t;

typedef struct {
    pet_u8_t exact_match_count;
    pet_u8_t semantic_match_count;
    pet_u8_t explicit_non_match_count;
    pet_u8_t imported_constants_match;
    pet_u8_t imported_dirty_rules_match;
    pet_u8_t imported_exit_semantics_match;
    pet_u8_t p37_manual_real_board_smoke_verified;
    pet_u8_t host_crc_is_jieli_lcd_crc;
    pet_u8_t sdl_visible_parity_claimed;
    pet_u8_t production_resource_parity_claimed;
    pet_u8_t home_observe_enabled;
    pet_u8_t full_pet2d_runtime_enabled;
    pet_u8_t pet2d_runtime_enabled;
} pet2d_mvp_a_home_observe_reconcile_summary_t;

const char *pet2d_mvp_a_home_observe_reconcile_scene_id(void);
const char *pet2d_mvp_a_home_observe_reconcile_scene_status(void);
pet_result_t pet2d_mvp_a_home_observe_reconcile_get_summary(
    pet2d_mvp_a_home_observe_reconcile_summary_t *out_summary);
pet_result_t pet2d_mvp_a_home_observe_reconcile_self_test(void);

#ifdef __cplusplus
}
#endif

#endif
