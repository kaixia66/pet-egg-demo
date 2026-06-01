#include "pet2d_mvp_a_home_observe_imported.h"

#include "pet_platform_jieli_internal.h"

extern int printf(const char *format, ...);

void mvp_a_lvgl_shell_release_display_owner(void);
void mvp_a_lvgl_shell_request_refresh(void);

#define PET2D_MVP_A_HOME_OBSERVE_IMPORTED_PIXELS \
    (PET2D_MVP_A_HOME_OBSERVE_IMPORTED_STAGE_W * \
     PET2D_MVP_A_HOME_OBSERVE_IMPORTED_STAGE_H)

typedef struct {
    pet2d_mvp_a_home_observe_imported_model_t model;
    pet2d_mvp_a_render_plan_t plan;
    pet_u8_t pose_cycle;
    pet_u32_t skipped_flush_count;
} pet2d_mvp_a_home_observe_imported_core_t;

typedef struct {
    pet_i16_t screen_x;
    pet_i16_t screen_y;
    pet_u32_t last_render_ms;
    pet2d_mvp_a_home_observe_imported_core_t core;
} pet2d_mvp_a_home_observe_imported_runtime_t;

static pet2d_mvp_a_home_observe_imported_runtime_t g_imported_runtime;
static pet2d_mvp_a_home_observe_imported_stats_t g_imported_stats;

#if PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC
static pet_u16_t g_imported_pixels[PET2D_MVP_A_HOME_OBSERVE_IMPORTED_PIXELS];
#endif

static pet2d_mvp_a_rect_t imported_stage_rect(void)
{
    pet2d_mvp_a_rect_t rect;

    rect.x = 0;
    rect.y = 0;
    rect.w = PET2D_MVP_A_HOME_OBSERVE_IMPORTED_STAGE_W;
    rect.h = PET2D_MVP_A_HOME_OBSERVE_IMPORTED_STAGE_H;
    return rect;
}

static pet2d_mvp_a_rect_t imported_pet_rect(pet_i16_t x, pet_i16_t y)
{
    pet2d_mvp_a_rect_t rect;

    rect.x = x;
    rect.y = y;
    rect.w = PET2D_MVP_A_HOME_OBSERVE_IMPORTED_SPRITE_SIZE;
    rect.h = PET2D_MVP_A_HOME_OBSERVE_IMPORTED_SPRITE_SIZE;
    return rect;
}

static pet_u32_t imported_now_ms(void)
{
    const pet_platform_t *platform = pet_platform_jieli_get();

    if ((platform == 0) || (platform->millis == 0)) {
        return 0u;
    }
    return platform->millis(platform->ctx);
}

static pet_u32_t imported_resolve_now(pet_u32_t now_ms)
{
    return (now_ms != 0u) ? now_ms : imported_now_ms();
}

static pet_bool_t imported_state_is_active(
    pet2d_mvp_a_home_observe_imported_state_t state)
{
    return (state == PET2D_MVP_A_HOME_OBSERVE_IMPORTED_STATE_ENTER) ||
           (state == PET2D_MVP_A_HOME_OBSERVE_IMPORTED_STATE_IDLE) ||
           (state == PET2D_MVP_A_HOME_OBSERVE_IMPORTED_STATE_MOVE_LEFT) ||
           (state == PET2D_MVP_A_HOME_OBSERVE_IMPORTED_STATE_MOVE_RIGHT) ||
           (state == PET2D_MVP_A_HOME_OBSERVE_IMPORTED_STATE_ACTION) ||
           (state == PET2D_MVP_A_HOME_OBSERVE_IMPORTED_STATE_EXITING);
}

static pet_bool_t imported_state_is_action(
    pet2d_mvp_a_home_observe_imported_state_t state)
{
    return (state == PET2D_MVP_A_HOME_OBSERVE_IMPORTED_STATE_MOVE_LEFT) ||
           (state == PET2D_MVP_A_HOME_OBSERVE_IMPORTED_STATE_MOVE_RIGHT) ||
           (state == PET2D_MVP_A_HOME_OBSERVE_IMPORTED_STATE_ACTION);
}

static pet_result_t imported_build_idle_plan(
    pet2d_mvp_a_home_observe_imported_core_t *core)
{
    pet_result_t ret;

    if (core == 0) {
        return PET_RESULT_INVALID_ARGUMENT;
    }
    ret = pet2d_mvp_a_renderer_build_idle_plan(&core->plan);
    if (ret == PET_RESULT_OK) {
        core->skipped_flush_count++;
    }
    return ret;
}

static pet_result_t imported_build_pet_plan(
    pet2d_mvp_a_home_observe_imported_core_t *core,
    pet_bool_t restore_stage)
{
    pet2d_mvp_a_rect_t stage;
    pet2d_mvp_a_rect_t pet;
    pet2d_mvp_a_rect_t prev;

    if (core == 0) {
        return PET_RESULT_INVALID_ARGUMENT;
    }
    stage = imported_stage_rect();
    pet = imported_pet_rect(core->model.pet_x, core->model.pet_y);
    prev = imported_pet_rect(core->model.prev_x, core->model.prev_y);
    return pet2d_mvp_a_renderer_build_pet_change_plan(
        &stage, &prev, &pet, core->model.pose, restore_stage, &core->plan);
}

static pet_result_t imported_core_enter(
    pet2d_mvp_a_home_observe_imported_core_t *core,
    pet_u32_t now_ms)
{
    pet2d_mvp_a_rect_t stage;
    pet2d_mvp_a_rect_t pet;
    pet_u8_t *bytes;
    pet_u32_t i;

    if (core == 0) {
        return PET_RESULT_INVALID_ARGUMENT;
    }
    bytes = (pet_u8_t *)core;
    for (i = 0u; i < (pet_u32_t)sizeof(*core); i++) {
        bytes[i] = 0u;
    }

    core->model.state = PET2D_MVP_A_HOME_OBSERVE_IMPORTED_STATE_IDLE;
    core->model.pose = 0u;
    core->model.pet_x = PET2D_MVP_A_HOME_OBSERVE_IMPORTED_INITIAL_X;
    core->model.pet_y = PET2D_MVP_A_HOME_OBSERVE_IMPORTED_INITIAL_Y;
    core->model.prev_x = core->model.pet_x;
    core->model.prev_y = core->model.pet_y;
    core->model.enter_ms = now_ms;
    core->model.timeout_ms =
        now_ms + PET2D_MVP_A_HOME_OBSERVE_IMPORTED_TIMEOUT_MS;
    core->model.exit_reason = PET2D_MVP_A_HOME_OBSERVE_IMPORTED_EXIT_NONE;

    stage = imported_stage_rect();
    pet = imported_pet_rect(core->model.pet_x, core->model.pet_y);
    return pet2d_mvp_a_renderer_build_initial_plan(
        &stage, &pet, core->model.pose, &core->plan);
}

static pet_result_t imported_core_finish_action_if_due(
    pet2d_mvp_a_home_observe_imported_core_t *core,
    pet_u32_t now_ms,
    pet_bool_t render_on_done)
{
    if (core == 0) {
        return PET_RESULT_INVALID_ARGUMENT;
    }
    if (imported_state_is_action(core->model.state) == PET_FALSE) {
        return PET_RESULT_OK;
    }
    if ((pet_u32_t)(now_ms - core->model.action_started_ms) <
        core->model.action_duration_ms) {
        return PET_RESULT_OK;
    }

    core->model.prev_x = core->model.pet_x;
    core->model.prev_y = core->model.pet_y;
    core->model.pose = 0u;
    core->model.state = PET2D_MVP_A_HOME_OBSERVE_IMPORTED_STATE_IDLE;
    core->model.action_duration_ms = 0u;
    if (render_on_done != PET_FALSE) {
        return imported_build_pet_plan(core, PET_FALSE);
    }
    return PET_RESULT_OK;
}

static pet_result_t imported_core_tick(
    pet2d_mvp_a_home_observe_imported_core_t *core,
    pet_u32_t now_ms,
    pet_bool_t render_on_done)
{
    pet_result_t ret;
    pet2d_mvp_a_home_observe_imported_state_t previous_state;

    if (core == 0) {
        return PET_RESULT_INVALID_ARGUMENT;
    }
    if ((core->model.state == PET2D_MVP_A_HOME_OBSERVE_IMPORTED_STATE_DONE) ||
        (core->model.state == PET2D_MVP_A_HOME_OBSERVE_IMPORTED_STATE_ERROR)) {
        return imported_build_idle_plan(core);
    }
    if (now_ms >= core->model.timeout_ms) {
        core->model.state = PET2D_MVP_A_HOME_OBSERVE_IMPORTED_STATE_DONE;
        core->model.exit_reason =
            PET2D_MVP_A_HOME_OBSERVE_IMPORTED_EXIT_TIMEOUT;
        return imported_build_idle_plan(core);
    }

    previous_state = core->model.state;
    ret = imported_core_finish_action_if_due(core, now_ms, render_on_done);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    if (previous_state == core->model.state) {
        if ((core->model.state ==
             PET2D_MVP_A_HOME_OBSERVE_IMPORTED_STATE_IDLE) ||
            (imported_state_is_action(core->model.state) != PET_FALSE)) {
            return imported_build_idle_plan(core);
        }
    }
    if ((core->plan.cmd_count == 0u) && (core->plan.skipped_flush == 0u)) {
        ret = imported_build_idle_plan(core);
    }
    return ret;
}

static pet_result_t imported_core_move(
    pet2d_mvp_a_home_observe_imported_core_t *core,
    pet_i16_t delta_x,
    pet2d_mvp_a_home_observe_imported_state_t state,
    pet_u32_t now_ms)
{
    pet2d_mvp_a_rect_t stage;
    pet2d_mvp_a_rect_t pet;
    pet_result_t ret;

    core->model.prev_x = core->model.pet_x;
    core->model.prev_y = core->model.pet_y;
    core->model.pet_x = (pet_i16_t)(core->model.pet_x + delta_x);
    pet = imported_pet_rect(core->model.pet_x, core->model.pet_y);
    stage = imported_stage_rect();
    ret = pet2d_mvp_a_rect_clamp_pet_to_stage(&stage, &pet);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    core->model.pet_x = pet.x;
    core->model.pet_y = pet.y;
    core->model.state = state;
    core->model.pose = 3u;
    core->model.action_started_ms = now_ms;
    core->model.action_duration_ms = PET2D_MVP_A_HOME_OBSERVE_IMPORTED_MOVE_MS;
    return imported_build_pet_plan(core, PET_TRUE);
}

static pet_result_t imported_core_handle_key(
    pet2d_mvp_a_home_observe_imported_core_t *core,
    const pet_key_event_t *event,
    pet_u32_t now_ms)
{
    pet_result_t ret;

    if ((core == 0) || (event == 0)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }
    if (event->type != PET_KEY_EVENT_CLICK) {
        return PET_RESULT_OK;
    }
    if ((core->model.state == PET2D_MVP_A_HOME_OBSERVE_IMPORTED_STATE_DONE) ||
        (core->model.state == PET2D_MVP_A_HOME_OBSERVE_IMPORTED_STATE_ERROR)) {
        return imported_build_idle_plan(core);
    }

    if (event->key == PET_KEY_CANCEL) {
        core->model.state = PET2D_MVP_A_HOME_OBSERVE_IMPORTED_STATE_DONE;
        core->model.exit_reason =
            PET2D_MVP_A_HOME_OBSERVE_IMPORTED_EXIT_CANCEL;
        return imported_build_idle_plan(core);
    }

    ret = imported_core_finish_action_if_due(core, now_ms, PET_FALSE);
    if (ret != PET_RESULT_OK) {
        return ret;
    }

    switch (event->key) {
    case PET_KEY_LEFT_UP:
        return imported_core_move(
            core, -(pet_i16_t)PET2D_MVP_A_HOME_OBSERVE_IMPORTED_STEP_PIXELS,
            PET2D_MVP_A_HOME_OBSERVE_IMPORTED_STATE_MOVE_LEFT, now_ms);
    case PET_KEY_RIGHT_DOWN:
        return imported_core_move(
            core, (pet_i16_t)PET2D_MVP_A_HOME_OBSERVE_IMPORTED_STEP_PIXELS,
            PET2D_MVP_A_HOME_OBSERVE_IMPORTED_STATE_MOVE_RIGHT, now_ms);
    case PET_KEY_OK:
        core->model.prev_x = core->model.pet_x;
        core->model.prev_y = core->model.pet_y;
        core->pose_cycle = (pet_u8_t)((core->pose_cycle + 1u) % 3u);
        core->model.pose = (core->pose_cycle == 1u) ? 1u :
                           ((core->pose_cycle == 2u) ? 2u : 0u);
        core->model.state = PET2D_MVP_A_HOME_OBSERVE_IMPORTED_STATE_ACTION;
        core->model.action_started_ms = now_ms;
        core->model.action_duration_ms =
            PET2D_MVP_A_HOME_OBSERVE_IMPORTED_ACTION_MS;
        return imported_build_pet_plan(core, PET_FALSE);
    default:
        break;
    }
    return PET_RESULT_OK;
}

const char *pet2d_mvp_a_home_observe_imported_state_name(
    pet2d_mvp_a_home_observe_imported_state_t state)
{
    switch (state) {
    case PET2D_MVP_A_HOME_OBSERVE_IMPORTED_STATE_ENTER:
        return "ENTER";
    case PET2D_MVP_A_HOME_OBSERVE_IMPORTED_STATE_IDLE:
        return "OBSERVE_IDLE";
    case PET2D_MVP_A_HOME_OBSERVE_IMPORTED_STATE_MOVE_LEFT:
        return "OBSERVE_MOVE_LEFT";
    case PET2D_MVP_A_HOME_OBSERVE_IMPORTED_STATE_MOVE_RIGHT:
        return "OBSERVE_MOVE_RIGHT";
    case PET2D_MVP_A_HOME_OBSERVE_IMPORTED_STATE_ACTION:
        return "OBSERVE_ACTION";
    case PET2D_MVP_A_HOME_OBSERVE_IMPORTED_STATE_EXITING:
        return "EXITING";
    case PET2D_MVP_A_HOME_OBSERVE_IMPORTED_STATE_DONE:
        return "DONE";
    case PET2D_MVP_A_HOME_OBSERVE_IMPORTED_STATE_ERROR:
        return "ERROR";
    case PET2D_MVP_A_HOME_OBSERVE_IMPORTED_STATE_NONE:
    default:
        return "NONE";
    }
}

const char *pet2d_mvp_a_home_observe_imported_exit_name(
    pet2d_mvp_a_home_observe_imported_exit_reason_t reason)
{
    switch (reason) {
    case PET2D_MVP_A_HOME_OBSERVE_IMPORTED_EXIT_CANCEL:
        return "CANCEL";
    case PET2D_MVP_A_HOME_OBSERVE_IMPORTED_EXIT_TIMEOUT:
        return "TIMEOUT";
    case PET2D_MVP_A_HOME_OBSERVE_IMPORTED_EXIT_END:
        return "END";
    case PET2D_MVP_A_HOME_OBSERVE_IMPORTED_EXIT_ERROR:
        return "ERROR";
    case PET2D_MVP_A_HOME_OBSERVE_IMPORTED_EXIT_NONE:
    default:
        return "NONE";
    }
}

static const char *imported_pose_name(pet_u8_t pose)
{
    switch (pose) {
    case 1u:
        return "HAPPY";
    case 2u:
        return "BLINK";
    case 3u:
        return "STEP";
    case 0u:
    default:
        return "IDLE";
    }
}

#if PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC
static pet_u16_t imported_stage_pixel(pet_i16_t x, pet_i16_t y)
{
    return (((((pet_u16_t)x >> 3) ^ ((pet_u16_t)y >> 3)) & 1u) != 0u) ?
           0x39e7u : 0x2104u;
}

static pet_u16_t imported_pet_pixel(pet_u16_t x, pet_u16_t y, pet_u8_t pose)
{
    if ((x < 2u) || (y < 2u) ||
        (x >= (PET2D_MVP_A_HOME_OBSERVE_IMPORTED_SPRITE_SIZE - 2u)) ||
        (y >= (PET2D_MVP_A_HOME_OBSERVE_IMPORTED_SPRITE_SIZE - 2u))) {
        return 0xffffu;
    }
    if (pose == 1u) {
        return 0xffe0u;
    }
    if (pose == 2u) {
        return ((y >= 12u) && (y < 16u)) ? 0x0000u : 0xf81fu;
    }
    if (pose == 3u) {
        return ((((x >> 1) + y) & 1u) != 0u) ? 0x001fu : 0x07ffu;
    }
    return ((((x >> 3) + (y >> 3)) & 1u) != 0u) ? 0xf81fu : 0x07e0u;
}

static void imported_fill_dirty_surface(const pet2d_mvp_a_rect_t *dirty)
{
    pet_u16_t x;
    pet_u16_t y;
    const pet2d_mvp_a_home_observe_imported_model_t *model =
        &g_imported_runtime.core.model;

    for (y = 0u; y < dirty->h; y++) {
        for (x = 0u; x < dirty->w; x++) {
            pet_i16_t local_x = (pet_i16_t)(dirty->x + (pet_i16_t)x);
            pet_i16_t local_y = (pet_i16_t)(dirty->y + (pet_i16_t)y);
            pet_u16_t pixel = imported_stage_pixel(local_x, local_y);

            if ((local_x >= model->pet_x) && (local_y >= model->pet_y) &&
                (local_x < (pet_i16_t)(model->pet_x +
                    PET2D_MVP_A_HOME_OBSERVE_IMPORTED_SPRITE_SIZE)) &&
                (local_y < (pet_i16_t)(model->pet_y +
                    PET2D_MVP_A_HOME_OBSERVE_IMPORTED_SPRITE_SIZE))) {
                pixel = imported_pet_pixel(
                    (pet_u16_t)(local_x - model->pet_x),
                    (pet_u16_t)(local_y - model->pet_y),
                    model->pose);
            }
            g_imported_pixels[((pet_u32_t)y * (pet_u32_t)dirty->w) +
                              (pet_u32_t)x] = pixel;
        }
    }
}
#endif

static pet_result_t imported_render_once(void)
{
    pet2d_mvp_a_render_plan_t *plan = &g_imported_runtime.core.plan;
    pet2d_mvp_a_rect_t dirty = plan->dirty_rect;
    pet_result_t ret = PET_RESULT_OK;
    pet_result_t raw_ret = PET_RESULT_OK;

    g_imported_stats.render_count++;
    g_imported_stats.last_dirty_w = dirty.w;
    g_imported_stats.last_dirty_h = dirty.h;
    g_imported_stats.last_pet_x =
        (pet_u16_t)g_imported_runtime.core.model.pet_x;
    g_imported_stats.last_pet_y =
        (pet_u16_t)g_imported_runtime.core.model.pet_y;
    g_imported_stats.last_pose = g_imported_runtime.core.model.pose;
    g_imported_stats.last_state =
        (pet_u8_t)g_imported_runtime.core.model.state;

    if ((plan->skipped_flush != 0u) || (plan->cmd_count == 0u) ||
        (pet2d_mvp_a_rect_is_valid(&dirty) == PET_FALSE)) {
        g_imported_stats.flush_skipped_count++;
        g_imported_stats.last_dirty_w = 0u;
        g_imported_stats.last_dirty_h = 0u;
        printf("[PET2D_MVP_A_IMPORT] render skipped state=%s exit=%s skipped=%lu\n",
               pet2d_mvp_a_home_observe_imported_state_name(
                   g_imported_runtime.core.model.state),
               pet2d_mvp_a_home_observe_imported_exit_name(
                   g_imported_runtime.core.model.exit_reason),
               (unsigned long)g_imported_stats.flush_skipped_count);
        return PET_RESULT_OK;
    }

#if PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC
    imported_fill_dirty_surface(&dirty);
    ret = pet_display_jieli_real_flush_poc_rect(
        (pet_i16_t)(g_imported_runtime.screen_x + dirty.x),
        (pet_i16_t)(g_imported_runtime.screen_y + dirty.y),
        dirty.w, dirty.h, g_imported_pixels, dirty.w);
#else
    ret = PET_RESULT_UNSUPPORTED;
#endif

    raw_ret = ret;
    if (ret == PET_RESULT_OK) {
        g_imported_stats.flush_success_count++;
    } else if (ret == PET_RESULT_UNSUPPORTED) {
        g_imported_stats.flush_skipped_count++;
        ret = PET_RESULT_OK;
    } else if ((ret == PET_RESULT_BUSY) || (ret == PET_RESULT_NOT_READY)) {
        g_imported_stats.flush_skipped_count++;
        ret = PET_RESULT_OK;
    } else {
        g_imported_stats.flush_fail_count++;
    }

    g_imported_runtime.core.model.frame_index++;
    g_imported_stats.last_result = (pet_u8_t)ret;
    printf("[PET2D_MVP_A_IMPORT] frame=%lu state=%s pose=%s pet=%d,%d dirty=%ux%u cmd_count=%u ret=%d raw_ret=%d owner=%d\n",
           (unsigned long)g_imported_runtime.core.model.frame_index,
           pet2d_mvp_a_home_observe_imported_state_name(
               g_imported_runtime.core.model.state),
           imported_pose_name(g_imported_runtime.core.model.pose),
           g_imported_runtime.core.model.pet_x,
           g_imported_runtime.core.model.pet_y,
           dirty.w,
           dirty.h,
           plan->cmd_count,
           ret,
           raw_ret,
           pet_display_jieli_get_owner());
    return ret;
}

static pet_result_t imported_exit_with_reason(
    pet2d_mvp_a_home_observe_imported_exit_reason_t reason,
    pet_u32_t now_ms)
{
    const pet_platform_t *platform = pet_platform_jieli_get();
    pet_result_t release_ret = PET_RESULT_OK;

    if (imported_state_is_active(g_imported_runtime.core.model.state) ==
        PET_FALSE &&
        pet_display_jieli_get_owner() != PET_DISPLAY_OWNER_PET2D) {
        return PET_RESULT_OK;
    }
    now_ms = imported_resolve_now(now_ms);
    g_imported_runtime.core.model.state =
        PET2D_MVP_A_HOME_OBSERVE_IMPORTED_STATE_EXITING;
    g_imported_runtime.core.model.exit_reason = reason;
    if ((pet_display_jieli_get_owner() == PET_DISPLAY_OWNER_PET2D) &&
        (platform != 0) && (platform->display_release != 0)) {
        release_ret = platform->display_release(platform->ctx,
                                                PET_DISPLAY_OWNER_PET2D);
    }
    g_imported_stats.exit_count++;
    g_imported_stats.last_exit_ms = now_ms;
    g_imported_stats.last_duration_ms =
        (pet_u32_t)(now_ms - g_imported_runtime.core.model.enter_ms);
    g_imported_stats.last_exit_reason = (pet_u8_t)reason;
    if ((release_ret != PET_RESULT_OK) ||
        (reason == PET2D_MVP_A_HOME_OBSERVE_IMPORTED_EXIT_ERROR)) {
        g_imported_stats.error_count++;
        g_imported_runtime.core.model.state =
            PET2D_MVP_A_HOME_OBSERVE_IMPORTED_STATE_ERROR;
    } else {
        g_imported_runtime.core.model.state =
            PET2D_MVP_A_HOME_OBSERVE_IMPORTED_STATE_DONE;
    }
    mvp_a_lvgl_shell_request_refresh();
    printf("[PET2D_MVP_A_IMPORT] exit reason=%s release_ret=%d duration=%lu frames=%lu keys=%lu owner=%d\n",
           pet2d_mvp_a_home_observe_imported_exit_name(reason),
           release_ret,
           (unsigned long)g_imported_stats.last_duration_ms,
           (unsigned long)g_imported_runtime.core.model.frame_index,
           (unsigned long)g_imported_stats.key_event_count,
           pet_display_jieli_get_owner());
    return release_ret;
}

pet_result_t pet2d_mvp_a_home_observe_imported_enter(void)
{
    const pet_platform_t *platform = pet_platform_jieli_get();
    pet_result_t ret;
    pet_u32_t now_ms;

    if (imported_state_is_active(g_imported_runtime.core.model.state) !=
        PET_FALSE) {
        return PET_RESULT_BUSY;
    }
    if ((platform == 0) || (platform->display_acquire == 0)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    now_ms = imported_now_ms();
    ret = imported_core_enter(&g_imported_runtime.core, now_ms);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    g_imported_runtime.screen_x =
        (pet_i16_t)(PET_JIELI_DISPLAY_SAFE_X +
                    ((PET_JIELI_DISPLAY_SAFE_W -
                      PET2D_MVP_A_HOME_OBSERVE_IMPORTED_STAGE_W) / 2u));
    g_imported_runtime.screen_y =
        (pet_i16_t)(PET_JIELI_DISPLAY_SAFE_Y +
                    ((PET_JIELI_DISPLAY_SAFE_H -
                      PET2D_MVP_A_HOME_OBSERVE_IMPORTED_STAGE_H) / 2u));
    g_imported_runtime.last_render_ms = now_ms;
    g_imported_stats.enter_count++;
    g_imported_stats.last_enter_ms = now_ms;
    g_imported_stats.last_exit_reason =
        PET2D_MVP_A_HOME_OBSERVE_IMPORTED_EXIT_NONE;

    printf("[PET2D_MVP_A_IMPORT] enter start owner=%d manifest=stage160x96 pet32 initial=64,32\n",
           pet_display_jieli_get_owner());
    mvp_a_lvgl_shell_release_display_owner();
    ret = platform->display_acquire(platform->ctx, PET_DISPLAY_OWNER_PET2D, 0u);
    if (ret != PET_RESULT_OK) {
        (void)imported_exit_with_reason(
            PET2D_MVP_A_HOME_OBSERVE_IMPORTED_EXIT_ERROR, now_ms);
        return ret;
    }
    ret = imported_render_once();
    if (ret != PET_RESULT_OK) {
        (void)imported_exit_with_reason(
            PET2D_MVP_A_HOME_OBSERVE_IMPORTED_EXIT_ERROR, now_ms);
        return ret;
    }
    printf("[PET2D_MVP_A_IMPORT] enter ok owner=%d screen=%d,%d timeout=%lu dirty=%ux%u\n",
           pet_display_jieli_get_owner(),
           g_imported_runtime.screen_x,
           g_imported_runtime.screen_y,
           (unsigned long)PET2D_MVP_A_HOME_OBSERVE_IMPORTED_TIMEOUT_MS,
           g_imported_runtime.core.plan.dirty_rect.w,
           g_imported_runtime.core.plan.dirty_rect.h);
    return PET_RESULT_OK;
}

pet_result_t pet2d_mvp_a_home_observe_imported_tick(pet_u32_t now_ms)
{
    pet_result_t ret;

    if (imported_state_is_active(g_imported_runtime.core.model.state) ==
        PET_FALSE) {
        return PET_RESULT_OK;
    }
    now_ms = imported_resolve_now(now_ms);
    g_imported_stats.tick_count++;
    ret = imported_core_tick(&g_imported_runtime.core, now_ms, PET_TRUE);
    if (ret != PET_RESULT_OK) {
        return imported_exit_with_reason(
            PET2D_MVP_A_HOME_OBSERVE_IMPORTED_EXIT_ERROR, now_ms);
    }
    ret = imported_render_once();
    if (ret != PET_RESULT_OK) {
        return imported_exit_with_reason(
            PET2D_MVP_A_HOME_OBSERVE_IMPORTED_EXIT_ERROR, now_ms);
    }
    if (g_imported_runtime.core.model.exit_reason ==
        PET2D_MVP_A_HOME_OBSERVE_IMPORTED_EXIT_TIMEOUT) {
        return imported_exit_with_reason(
            PET2D_MVP_A_HOME_OBSERVE_IMPORTED_EXIT_TIMEOUT, now_ms);
    }
    return PET_RESULT_OK;
}

pet_result_t pet2d_mvp_a_home_observe_imported_handle_key(
    const pet_key_event_t *event)
{
    pet_result_t ret;
    pet_u32_t now_ms;

    if (event == 0) {
        return PET_RESULT_INVALID_ARGUMENT;
    }
    if (imported_state_is_active(g_imported_runtime.core.model.state) ==
        PET_FALSE) {
        return PET_RESULT_NOT_READY;
    }
    now_ms = imported_resolve_now(event->timestamp_ms);
    if (event->type != PET_KEY_EVENT_CLICK) {
        return PET_RESULT_OK;
    }
    g_imported_stats.key_event_count++;
    ret = imported_core_handle_key(&g_imported_runtime.core, event, now_ms);
    if (ret != PET_RESULT_OK) {
        return imported_exit_with_reason(
            PET2D_MVP_A_HOME_OBSERVE_IMPORTED_EXIT_ERROR, now_ms);
    }
    printf("[PET2D_MVP_A_IMPORT] input key=%d state=%s pose=%s pet=%d,%d dirty=%ux%u exit=%s\n",
           event->key,
           pet2d_mvp_a_home_observe_imported_state_name(
               g_imported_runtime.core.model.state),
           imported_pose_name(g_imported_runtime.core.model.pose),
           g_imported_runtime.core.model.pet_x,
           g_imported_runtime.core.model.pet_y,
           g_imported_runtime.core.plan.dirty_rect.w,
           g_imported_runtime.core.plan.dirty_rect.h,
           pet2d_mvp_a_home_observe_imported_exit_name(
               g_imported_runtime.core.model.exit_reason));
    ret = imported_render_once();
    if (ret != PET_RESULT_OK) {
        return imported_exit_with_reason(
            PET2D_MVP_A_HOME_OBSERVE_IMPORTED_EXIT_ERROR, now_ms);
    }
    if (g_imported_runtime.core.model.exit_reason ==
        PET2D_MVP_A_HOME_OBSERVE_IMPORTED_EXIT_CANCEL) {
        return imported_exit_with_reason(
            PET2D_MVP_A_HOME_OBSERVE_IMPORTED_EXIT_CANCEL, now_ms);
    }
    return PET_RESULT_OK;
}

pet_result_t pet2d_mvp_a_home_observe_imported_exit(void)
{
    return imported_exit_with_reason(
        PET2D_MVP_A_HOME_OBSERVE_IMPORTED_EXIT_CANCEL, imported_now_ms());
}

pet_bool_t pet2d_mvp_a_home_observe_imported_is_active(void)
{
    return imported_state_is_active(g_imported_runtime.core.model.state);
}

pet_result_t pet2d_mvp_a_home_observe_imported_get_model(
    pet2d_mvp_a_home_observe_imported_model_t *out_model)
{
    if (out_model == 0) {
        return PET_RESULT_INVALID_ARGUMENT;
    }
    *out_model = g_imported_runtime.core.model;
    return PET_RESULT_OK;
}

pet_result_t pet2d_mvp_a_home_observe_imported_get_render_plan(
    pet2d_mvp_a_render_plan_t *out_plan)
{
    if (out_plan == 0) {
        return PET_RESULT_INVALID_ARGUMENT;
    }
    *out_plan = g_imported_runtime.core.plan;
    return PET_RESULT_OK;
}

pet_result_t pet2d_mvp_a_home_observe_imported_get_stats(
    pet2d_mvp_a_home_observe_imported_stats_t *out_stats)
{
    if (out_stats == 0) {
        return PET_RESULT_INVALID_ARGUMENT;
    }
    *out_stats = g_imported_stats;
    return PET_RESULT_OK;
}

pet_result_t pet2d_mvp_a_home_observe_imported_self_test(void)
{
    pet2d_mvp_a_home_observe_imported_core_t core;
    pet_key_event_t event;
    pet_result_t ret;

    ret = imported_core_enter(&core, 0u);
    if ((ret != PET_RESULT_OK) ||
        (core.model.pet_x != PET2D_MVP_A_HOME_OBSERVE_IMPORTED_INITIAL_X) ||
        (core.model.pet_y != PET2D_MVP_A_HOME_OBSERVE_IMPORTED_INITIAL_Y) ||
        (core.plan.dirty_rect.w != PET2D_MVP_A_HOME_OBSERVE_IMPORTED_STAGE_W) ||
        (core.plan.dirty_rect.h != PET2D_MVP_A_HOME_OBSERVE_IMPORTED_STAGE_H)) {
        return PET_RESULT_ERROR;
    }

    event.type = PET_KEY_EVENT_CLICK;
    event.timestamp_ms = 10u;
    event.hold_ms = 0u;
    event.repeat_count = 0u;
    event.raw_code = 0u;
    event.key = PET_KEY_LEFT_UP;
    ret = imported_core_handle_key(&core, &event, 10u);
    if ((ret != PET_RESULT_OK) ||
        (core.model.pet_x !=
         (PET2D_MVP_A_HOME_OBSERVE_IMPORTED_INITIAL_X -
          (pet_i16_t)PET2D_MVP_A_HOME_OBSERVE_IMPORTED_STEP_PIXELS)) ||
        (core.plan.dirty_rect.w !=
         (PET2D_MVP_A_HOME_OBSERVE_IMPORTED_SPRITE_SIZE +
          PET2D_MVP_A_HOME_OBSERVE_IMPORTED_STEP_PIXELS)) ||
        (core.plan.dirty_rect.h !=
         PET2D_MVP_A_HOME_OBSERVE_IMPORTED_SPRITE_SIZE)) {
        return PET_RESULT_ERROR;
    }
    ret = imported_core_tick(&core,
                             10u + PET2D_MVP_A_HOME_OBSERVE_IMPORTED_MOVE_MS,
                             PET_TRUE);
    if ((ret != PET_RESULT_OK) ||
        (core.model.state != PET2D_MVP_A_HOME_OBSERVE_IMPORTED_STATE_IDLE)) {
        return PET_RESULT_ERROR;
    }

    event.key = PET_KEY_RIGHT_DOWN;
    ret = imported_core_handle_key(&core, &event, 200u);
    if ((ret != PET_RESULT_OK) ||
        (core.model.pet_x != PET2D_MVP_A_HOME_OBSERVE_IMPORTED_INITIAL_X) ||
        (core.plan.dirty_rect.w !=
         (PET2D_MVP_A_HOME_OBSERVE_IMPORTED_SPRITE_SIZE +
          PET2D_MVP_A_HOME_OBSERVE_IMPORTED_STEP_PIXELS))) {
        return PET_RESULT_ERROR;
    }

    event.key = PET_KEY_OK;
    ret = imported_core_handle_key(&core, &event, 400u);
    if ((ret != PET_RESULT_OK) ||
        (core.model.state != PET2D_MVP_A_HOME_OBSERVE_IMPORTED_STATE_ACTION) ||
        (core.model.pose != 1u) ||
        (core.plan.dirty_rect.w !=
         PET2D_MVP_A_HOME_OBSERVE_IMPORTED_SPRITE_SIZE) ||
        (core.plan.dirty_rect.h !=
         PET2D_MVP_A_HOME_OBSERVE_IMPORTED_SPRITE_SIZE)) {
        return PET_RESULT_ERROR;
    }

    event.key = PET_KEY_CANCEL;
    ret = imported_core_handle_key(&core, &event, 720u);
    if ((ret != PET_RESULT_OK) ||
        (core.model.exit_reason !=
         PET2D_MVP_A_HOME_OBSERVE_IMPORTED_EXIT_CANCEL) ||
        (core.plan.skipped_flush == 0u)) {
        return PET_RESULT_ERROR;
    }

    ret = imported_core_enter(&core, 0u);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    ret = imported_core_tick(&core,
                             PET2D_MVP_A_HOME_OBSERVE_IMPORTED_TIMEOUT_MS,
                             PET_TRUE);
    if ((ret != PET_RESULT_OK) ||
        (core.model.exit_reason !=
         PET2D_MVP_A_HOME_OBSERVE_IMPORTED_EXIT_TIMEOUT) ||
        (core.plan.skipped_flush == 0u)) {
        return PET_RESULT_ERROR;
    }
    return PET_RESULT_OK;
}
