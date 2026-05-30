#include "pet2d_scene.h"

#include "pet2d_movement_poc.h"
#include "pet_platform_jieli.h"

extern int printf(const char *format, ...);

void mvp_a_lvgl_shell_release_display_owner(void);
void mvp_a_lvgl_shell_request_refresh(void);

static pet2d_scene_state_t g_pet2d_scene_state = PET2D_SCENE_STATE_IDLE;
static pet2d_scene_stats_t g_pet2d_scene_stats;
static pet_u32_t g_pet2d_scene_enter_ms;
static pet_u32_t g_pet2d_scene_last_render_ms;

static pet_u32_t pet2d_scene_now_ms(void)
{
    const pet_platform_t *platform = pet_platform_jieli_get();

    if ((platform == 0) || (platform->millis == 0)) {
        return 0u;
    }
    return platform->millis(platform->ctx);
}

static pet_u32_t pet2d_scene_resolve_now(pet_u32_t now_ms)
{
    if (now_ms != 0u) {
        return now_ms;
    }
    return pet2d_scene_now_ms();
}

static pet_u32_t pet2d_scene_elapsed_ms(pet_u32_t end_ms, pet_u32_t start_ms)
{
    return (pet_u32_t)(end_ms - start_ms);
}

static void pet2d_scene_set_state(pet2d_scene_state_t state)
{
    g_pet2d_scene_state = state;
    g_pet2d_scene_stats.last_state = (pet_u8_t)state;
}

static void pet2d_scene_record_render_result(pet_result_t ret)
{
    pet2d_movement_poc_stats_t movement_stats;

    g_pet2d_scene_stats.render_count++;
    if (ret == PET_RESULT_OK) {
        g_pet2d_scene_stats.flush_success_count++;
    } else if (ret != PET_RESULT_UNSUPPORTED) {
        g_pet2d_scene_stats.flush_fail_count++;
    }

    if (pet2d_movement_poc_get_stats(&movement_stats) == PET_RESULT_OK) {
        g_pet2d_scene_stats.last_key_to_flush_ms =
            movement_stats.last_key_to_flush_done_ms;
    }
}

static pet_result_t pet2d_scene_render_once(void)
{
    pet_result_t ret = pet2d_movement_poc_render_once();

    pet2d_scene_record_render_result(ret);
    if (ret == PET_RESULT_UNSUPPORTED) {
        printf("[PET2D_SCENE] render skipped real_flush_gate=0\n");
        return PET_RESULT_OK;
    }
    printf("[PET2D_SCENE] render ret=%d owner=%d\n",
           ret, pet_display_jieli_get_owner());
    return ret;
}

static pet_result_t pet2d_scene_exit_with_reason(pet2d_scene_exit_reason_t reason,
                                                 pet_u32_t now_ms)
{
    const pet_platform_t *platform = pet_platform_jieli_get();
    pet_display_owner_t owner;
    pet_result_t release_ret = PET_RESULT_OK;
    pet_u32_t duration_ms;

    if ((g_pet2d_scene_state != PET2D_SCENE_STATE_RUNNING) &&
        (g_pet2d_scene_state != PET2D_SCENE_STATE_ENTERING) &&
        (g_pet2d_scene_state != PET2D_SCENE_STATE_ERROR)) {
        return PET_RESULT_OK;
    }

    pet2d_scene_set_state(PET2D_SCENE_STATE_EXITING);
    now_ms = pet2d_scene_resolve_now(now_ms);
    owner = pet_display_jieli_get_owner();
    if ((owner == PET_DISPLAY_OWNER_PET2D) &&
        (platform != 0) && (platform->display_release != 0)) {
        release_ret = platform->display_release(platform->ctx, PET_DISPLAY_OWNER_PET2D);
    }

    duration_ms = pet2d_scene_elapsed_ms(now_ms, g_pet2d_scene_enter_ms);
    g_pet2d_scene_stats.exit_count++;
    g_pet2d_scene_stats.last_exit_ms = now_ms;
    g_pet2d_scene_stats.last_duration_ms = duration_ms;
    if (duration_ms > g_pet2d_scene_stats.max_duration_ms) {
        g_pet2d_scene_stats.max_duration_ms = duration_ms;
    }
    g_pet2d_scene_stats.last_exit_reason = (pet_u8_t)reason;
    if ((reason == PET2D_SCENE_EXIT_ERROR) || (release_ret != PET_RESULT_OK)) {
        g_pet2d_scene_stats.error_count++;
        pet2d_scene_set_state(PET2D_SCENE_STATE_ERROR);
    } else {
        pet2d_scene_set_state(PET2D_SCENE_STATE_DONE);
    }

    mvp_a_lvgl_shell_request_refresh();
    printf("[PET2D_SCENE] exit reason=%d release_ret=%d duration=%lu owner=%d\n",
           reason, release_ret, (unsigned long)duration_ms, pet_display_jieli_get_owner());
    return release_ret;
}

pet_result_t pet2d_scene_enter_test(void)
{
    const pet_platform_t *platform = pet_platform_jieli_get();
    pet_result_t ret;
    pet_u32_t now_ms;

    if ((g_pet2d_scene_state == PET2D_SCENE_STATE_ENTERING) ||
        (g_pet2d_scene_state == PET2D_SCENE_STATE_RUNNING)) {
        return PET_RESULT_BUSY;
    }
    if ((platform == 0) || (platform->display_acquire == 0)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    pet2d_scene_set_state(PET2D_SCENE_STATE_ENTERING);
    g_pet2d_scene_stats.enter_count++;
    now_ms = pet2d_scene_now_ms();
    g_pet2d_scene_enter_ms = now_ms;
    g_pet2d_scene_last_render_ms = now_ms;
    g_pet2d_scene_stats.last_enter_ms = now_ms;
    g_pet2d_scene_stats.last_exit_reason = PET2D_SCENE_EXIT_NONE;

    printf("[PET2D_SCENE] enter start owner=%d\n", pet_display_jieli_get_owner());
    mvp_a_lvgl_shell_release_display_owner();
    ret = platform->display_acquire(platform->ctx, PET_DISPLAY_OWNER_PET2D, 0u);
    if (ret != PET_RESULT_OK) {
        g_pet2d_scene_stats.error_count++;
        pet2d_scene_set_state(PET2D_SCENE_STATE_ERROR);
        (void)pet2d_scene_exit_with_reason(PET2D_SCENE_EXIT_ERROR, now_ms);
        printf("[PET2D_SCENE] acquire failed ret=%d owner=%d\n",
               ret, pet_display_jieli_get_owner());
        return ret;
    }

    ret = pet2d_movement_poc_init();
    if (ret != PET_RESULT_OK) {
        g_pet2d_scene_stats.error_count++;
        pet2d_scene_set_state(PET2D_SCENE_STATE_ERROR);
        (void)pet2d_scene_exit_with_reason(PET2D_SCENE_EXIT_ERROR, now_ms);
        return ret;
    }
    (void)pet2d_movement_poc_reset_stats();

    pet2d_scene_set_state(PET2D_SCENE_STATE_RUNNING);
    ret = pet2d_movement_poc_clear_scene_background();
    if ((ret != PET_RESULT_OK) && (ret != PET_RESULT_UNSUPPORTED)) {
        printf("[PET2D_SCENE] background failed ret=%d owner=%d\n",
               ret, pet_display_jieli_get_owner());
        (void)pet2d_scene_exit_with_reason(PET2D_SCENE_EXIT_ERROR, now_ms);
        return ret;
    }
    ret = pet2d_scene_render_once();
    if (ret != PET_RESULT_OK) {
        (void)pet2d_scene_exit_with_reason(PET2D_SCENE_EXIT_ERROR, now_ms);
        return ret;
    }

    printf("[PET2D_SCENE] enter ok owner=%d timeout=%lu\n",
           pet_display_jieli_get_owner(), (unsigned long)PET2D_SCENE_TEST_TIMEOUT_MS);
    return PET_RESULT_OK;
}

pet_result_t pet2d_scene_tick(pet_u32_t now_ms)
{
    pet_result_t ret;
    pet_u32_t elapsed_ms;

    if (g_pet2d_scene_state != PET2D_SCENE_STATE_RUNNING) {
        return PET_RESULT_OK;
    }

    now_ms = pet2d_scene_resolve_now(now_ms);
    g_pet2d_scene_stats.tick_count++;
    elapsed_ms = pet2d_scene_elapsed_ms(now_ms, g_pet2d_scene_enter_ms);
    if (elapsed_ms >= PET2D_SCENE_TEST_TIMEOUT_MS) {
        return pet2d_scene_exit_with_reason(PET2D_SCENE_EXIT_TIMEOUT, now_ms);
    }

    if (pet2d_scene_elapsed_ms(now_ms, g_pet2d_scene_last_render_ms) >=
        PET2D_SCENE_RENDER_INTERVAL_MS) {
        g_pet2d_scene_last_render_ms = now_ms;
        ret = pet2d_scene_render_once();
        if (ret != PET_RESULT_OK) {
            return pet2d_scene_exit_with_reason(PET2D_SCENE_EXIT_ERROR, now_ms);
        }
    }

    return PET_RESULT_OK;
}

pet_result_t pet2d_scene_handle_key(const pet_key_event_t *event)
{
    pet_result_t ret;

    if (event == 0) {
        return PET_RESULT_INVALID_ARGUMENT;
    }
    if (g_pet2d_scene_state != PET2D_SCENE_STATE_RUNNING) {
        return PET_RESULT_NOT_READY;
    }

    g_pet2d_scene_stats.key_count++;
    printf("[PET2D_SCENE] key=%d type=%d owner=%d\n",
           event->key, event->type, pet_display_jieli_get_owner());
    if ((event->type == PET_KEY_EVENT_CLICK) && (event->key == PET_KEY_CANCEL)) {
        (void)pet2d_movement_poc_handle_key(event);
        return pet2d_scene_exit_with_reason(PET2D_SCENE_EXIT_CANCEL,
                                            pet2d_scene_resolve_now(event->timestamp_ms));
    }

    ret = pet2d_movement_poc_handle_key(event);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    ret = pet2d_scene_render_once();
    if (ret != PET_RESULT_OK) {
        return pet2d_scene_exit_with_reason(PET2D_SCENE_EXIT_ERROR,
                                            pet2d_scene_resolve_now(event->timestamp_ms));
    }
    g_pet2d_scene_last_render_ms = pet2d_scene_now_ms();
    return PET_RESULT_OK;
}

pet_result_t pet2d_scene_exit(void)
{
    return pet2d_scene_exit_with_reason(PET2D_SCENE_EXIT_CANCEL, pet2d_scene_now_ms());
}

pet_bool_t pet2d_scene_is_active(void)
{
    return (g_pet2d_scene_state == PET2D_SCENE_STATE_ENTERING) ||
           (g_pet2d_scene_state == PET2D_SCENE_STATE_RUNNING) ||
           (g_pet2d_scene_state == PET2D_SCENE_STATE_EXITING);
}

pet_result_t pet2d_scene_get_state(pet2d_scene_state_t *out_state)
{
    if (out_state == 0) {
        return PET_RESULT_INVALID_ARGUMENT;
    }
    *out_state = g_pet2d_scene_state;
    return PET_RESULT_OK;
}

pet_result_t pet2d_scene_get_stats(pet2d_scene_stats_t *out_stats)
{
    if (out_stats == 0) {
        return PET_RESULT_INVALID_ARGUMENT;
    }
    *out_stats = g_pet2d_scene_stats;
    return PET_RESULT_OK;
}

pet_result_t pet2d_scene_reset_stats(void)
{
    pet_u8_t *bytes = (pet_u8_t *)&g_pet2d_scene_stats;
    pet_u32_t i;

    for (i = 0u; i < (pet_u32_t)sizeof(g_pet2d_scene_stats); i++) {
        bytes[i] = 0u;
    }
    g_pet2d_scene_state = PET2D_SCENE_STATE_IDLE;
    g_pet2d_scene_stats.last_state = (pet_u8_t)PET2D_SCENE_STATE_IDLE;
    return PET_RESULT_OK;
}

pet_result_t pet2d_scene_self_test(void)
{
    const pet_platform_t *platform = pet_platform_jieli_get();
    pet_display_owner_t original_owner;
    pet_key_event_t event;
    pet2d_scene_state_t state;
    pet2d_scene_stats_t stats;
    pet_result_t ret;

    if ((platform == 0) || (platform->display_acquire == 0) ||
        (platform->display_release == 0)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    original_owner = pet_display_jieli_get_owner();
    if (original_owner != PET_DISPLAY_OWNER_NONE) {
        (void)platform->display_release(platform->ctx, original_owner);
    }

    ret = pet2d_scene_reset_stats();
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    ret = pet2d_scene_enter_test();
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    if (pet2d_scene_get_state(&state) != PET_RESULT_OK) {
        return PET_RESULT_ERROR;
    }
    if (state != PET2D_SCENE_STATE_RUNNING) {
        return PET_RESULT_ERROR;
    }

    event.key = PET_KEY_RIGHT_DOWN;
    event.type = PET_KEY_EVENT_CLICK;
    event.timestamp_ms = 100u;
    ret = pet2d_scene_handle_key(&event);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    event.key = PET_KEY_CANCEL;
    event.type = PET_KEY_EVENT_CLICK;
    event.timestamp_ms = 200u;
    ret = pet2d_scene_handle_key(&event);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    if (pet2d_scene_get_state(&state) != PET_RESULT_OK) {
        return PET_RESULT_ERROR;
    }
    if (state != PET2D_SCENE_STATE_DONE) {
        return PET_RESULT_ERROR;
    }

    ret = pet2d_scene_enter_test();
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    ret = pet2d_scene_tick(g_pet2d_scene_enter_ms + PET2D_SCENE_TEST_TIMEOUT_MS + 1u);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    if (pet2d_scene_get_stats(&stats) != PET_RESULT_OK) {
        return PET_RESULT_ERROR;
    }
    if ((stats.enter_count != 2u) ||
        (stats.exit_count != 2u) ||
        (stats.key_count == 0u) ||
        (stats.render_count == 0u) ||
        (stats.last_exit_reason != (pet_u8_t)PET2D_SCENE_EXIT_TIMEOUT)) {
        return PET_RESULT_ERROR;
    }

    if (pet_display_jieli_get_owner() == PET_DISPLAY_OWNER_PET2D) {
        (void)platform->display_release(platform->ctx, PET_DISPLAY_OWNER_PET2D);
    }
    if (original_owner != PET_DISPLAY_OWNER_NONE) {
        (void)platform->display_acquire(platform->ctx, original_owner, 0u);
    }
    pet2d_scene_set_state(PET2D_SCENE_STATE_DONE);
    return PET_RESULT_OK;
}
