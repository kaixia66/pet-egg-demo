#include "mvp_a_debug.h"
#include "mvp_a_pet.h"
#include "mvp_a_save.h"
#include "pet2d_mvp_a_scene_skeleton.h"
#include "pet2d_perf_poc.h"
#include "pet2d_scene.h"
#include "pet_save_jieli_syscfg_backend.h"
#include "pet_selftest.h"

static mvp_a_debug_action_t mvp_a_debug_selected = MVP_A_DEBUG_VIEW_SAVE;

static const char *mvp_a_debug_p21_backend_name(pet_u8_t backend_type)
{
    switch (backend_type) {
    case PET_SAVE_JIELI_SYSCFG_BACKEND_FAKE:
        return "fake";
    case PET_SAVE_JIELI_SYSCFG_BACKEND_SYSCFG:
        return "syscfg";
    default:
        break;
    }
    return "none";
}

static const char *mvp_a_debug_p21_slot_name(pet_u8_t slot)
{
    switch (slot) {
    case PET_SAVE_SLOT_A:
        return "A";
    case PET_SAVE_SLOT_B:
        return "B";
    default:
        break;
    }
    return "NONE";
}

static const char *mvp_a_debug_p21_slot_status(pet_result_t result)
{
    switch (result) {
    case PET_RESULT_OK:
        return "VALID";
    case PET_RESULT_NOT_FOUND:
        return "EMPTY";
    case PET_RESULT_BAD_CRC:
        return "BAD_CRC";
    case PET_RESULT_BAD_VERSION:
        return "BAD_VERSION";
    default:
        break;
    }
    return "ERROR";
}

static mvp_a_result_t mvp_a_debug_run_p21_save_ab(void)
{
    pet_result_t ret;
    pet_save_jieli_syscfg_stats_t stats;

    printf("[MVP_A][P21] save_ab_internal begin\n");
    ret = pet_selftest_run_case(PET_SELFTEST_SAVE_AB_INTERNAL);
    if (pet_save_jieli_syscfg_get_last_stats(&stats) == PET_RESULT_OK) {
        printf("[MVP_A][P21] result=%s backend=%s backend_id=%u non_destructive=%u items=%u,%u\n",
               (ret == PET_RESULT_OK) ? "PASS" : "FAIL",
               mvp_a_debug_p21_backend_name(stats.backend_type),
               stats.backend_type,
               stats.non_destructive_namespace,
               stats.slot_a_item_id,
               stats.slot_b_item_id);
        printf("[MVP_A][P21] slot_a=%s slot_b=%s selected_slot=%s counter=%lu payload_len=%lu crc=0x%08lx\n",
               mvp_a_debug_p21_slot_status(stats.slot_a_read_result),
               mvp_a_debug_p21_slot_status(stats.slot_b_read_result),
               mvp_a_debug_p21_slot_name(stats.selected_slot),
               (unsigned long)stats.selected_counter,
               (unsigned long)stats.selected_payload_len,
               (unsigned long)stats.selected_crc32);
        printf("[MVP_A][P21] write_cases pass=%u fail=%u\n",
               stats.write_case_pass_count,
               stats.write_case_fail_count);
        printf("[MVP_A][P21] readback_cases pass=%u fail=%u\n",
               stats.readback_case_pass_count,
               stats.readback_case_fail_count);
        printf("[MVP_A][P21] fallback_cases pass=%u fail=%u\n",
               stats.fallback_case_pass_count,
               stats.fallback_case_fail_count);
        printf("[MVP_A][P21] real_syscfg_backend=%u manual_selftest_real_write_verified=%u snapshot_side_effect_free=1 low_battery_guard=planned\n",
               (stats.backend_type == PET_SAVE_JIELI_SYSCFG_BACKEND_SYSCFG) ? 1u : 0u,
               stats.real_write_verified);
    } else {
        printf("[MVP_A][P21] result=%s stats unavailable ret=%d\n",
               (ret == PET_RESULT_OK) ? "PASS" : "FAIL", ret);
    }

    return (ret == PET_RESULT_OK) ? MVP_A_RESULT_OK : MVP_A_RESULT_NOT_READY;
}

void mvp_a_debug_init(void)
{
    mvp_a_debug_selected = MVP_A_DEBUG_VIEW_SAVE;
}

void mvp_a_debug_select_next(void)
{
    mvp_a_debug_selected = (mvp_a_debug_action_t)(mvp_a_debug_selected + 1);
    if (mvp_a_debug_selected >= MVP_A_DEBUG_MAX) {
        mvp_a_debug_selected = MVP_A_DEBUG_VIEW_SAVE;
    }
}

void mvp_a_debug_select_prev(void)
{
    if (mvp_a_debug_selected == MVP_A_DEBUG_VIEW_SAVE) {
        mvp_a_debug_selected = (mvp_a_debug_action_t)(MVP_A_DEBUG_MAX - 1);
    } else {
        mvp_a_debug_selected = (mvp_a_debug_action_t)(mvp_a_debug_selected - 1);
    }
}

mvp_a_debug_action_t mvp_a_debug_get_selected(void)
{
    return mvp_a_debug_selected;
}

mvp_a_result_t mvp_a_debug_execute_selected(void)
{
    switch (mvp_a_debug_selected) {
    case MVP_A_DEBUG_VIEW_SAVE:
        return MVP_A_RESULT_OK;
    case MVP_A_DEBUG_RESET_SAVE:
        return mvp_a_save_reset();
    case MVP_A_DEBUG_FAST_GROWTH:
        return mvp_a_pet_set_fast_growth(mvp_a_pet_fast_growth_enabled() ? MVP_A_FALSE : MVP_A_TRUE);
    case MVP_A_DEBUG_PET2D_SCENE:
        return (pet2d_scene_enter_test() == PET_RESULT_OK) ? MVP_A_RESULT_OK : MVP_A_RESULT_NOT_READY;
    case MVP_A_DEBUG_P19_PERF32:
        return (pet2d_perf_poc_run_mode(PET2D_PERF_MODE_RECT_32, 60u, 0u) == PET_RESULT_OK) ?
               MVP_A_RESULT_OK : MVP_A_RESULT_NOT_READY;
    case MVP_A_DEBUG_P19_PERF64:
        return (pet2d_perf_poc_run_mode(PET2D_PERF_MODE_RECT_64, 60u, 0u) == PET_RESULT_OK) ?
               MVP_A_RESULT_OK : MVP_A_RESULT_NOT_READY;
    case MVP_A_DEBUG_P19_PERF96:
        return (pet2d_perf_poc_run_mode(PET2D_PERF_MODE_RECT_96, 60u, 0u) == PET_RESULT_OK) ?
               MVP_A_RESULT_OK : MVP_A_RESULT_NOT_READY;
    case MVP_A_DEBUG_P21_SAVE_AB:
        return mvp_a_debug_run_p21_save_ab();
    case MVP_A_DEBUG_P22_SCENE_SKELETON:
        return (pet2d_mvp_a_scene_skeleton_enter() == PET_RESULT_OK) ?
               MVP_A_RESULT_OK : MVP_A_RESULT_NOT_READY;
    default:
        break;
    }

    return MVP_A_RESULT_INVALID_PARAM;
}

const char *mvp_a_debug_get_action_name(mvp_a_debug_action_t action)
{
    switch (action) {
    case MVP_A_DEBUG_VIEW_SAVE:
        return "View";
    case MVP_A_DEBUG_RESET_SAVE:
        return "Clear";
    case MVP_A_DEBUG_FAST_GROWTH:
        return "Fast";
    case MVP_A_DEBUG_PET2D_SCENE:
        return "P18 Scene";
    case MVP_A_DEBUG_P19_PERF32:
        return "P19 Perf32";
    case MVP_A_DEBUG_P19_PERF64:
        return "P19 Perf64";
    case MVP_A_DEBUG_P19_PERF96:
        return "P19 Perf96";
    case MVP_A_DEBUG_P21_SAVE_AB:
        return "P21 Save";
    case MVP_A_DEBUG_P22_SCENE_SKELETON:
        return "P22 Scene";
    default:
        break;
    }

    return "Debug";
}

const char *mvp_a_debug_get_prompt(void)
{
    const mvp_a_save_data_t *data;

    switch (mvp_a_debug_selected) {
    case MVP_A_DEBUG_VIEW_SAVE:
        data = mvp_a_save_get_const_data();
        if (!data) {
            return "No Save";
        }
        return mvp_a_pet_get_stage_name((mvp_a_pet_stage_t)data->pet_stage);
    case MVP_A_DEBUG_RESET_SAVE:
        return "Confirm Clear";
    case MVP_A_DEBUG_FAST_GROWTH:
        return mvp_a_pet_fast_growth_enabled() ? "Fast On" : "Fast Off";
    case MVP_A_DEBUG_PET2D_SCENE:
        return "Pet2D Scene";
    case MVP_A_DEBUG_P19_PERF32:
        return "Perf 32x32";
    case MVP_A_DEBUG_P19_PERF64:
        return "Perf 64x64";
    case MVP_A_DEBUG_P19_PERF96:
        return "Perf 96x96";
    case MVP_A_DEBUG_P21_SAVE_AB:
        return "Save A/B";
    case MVP_A_DEBUG_P22_SCENE_SKELETON:
        return "Scene Skeleton";
    default:
        break;
    }

    return "Debug";
}
