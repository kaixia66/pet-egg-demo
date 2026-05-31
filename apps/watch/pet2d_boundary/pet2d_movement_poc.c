#include "pet2d_movement_poc.h"
#include "pet2d_dirty_rect_poc.h"
#include "pet2d_resource_sprite_poc.h"
#include "pet_platform_jieli_internal.h"

#define PET2D_MOVEMENT_POC_MIN_X ((pet_i16_t)PET_JIELI_DISPLAY_SAFE_X)
#define PET2D_MOVEMENT_POC_MIN_Y ((pet_i16_t)PET_JIELI_DISPLAY_SAFE_Y)
#define PET2D_MOVEMENT_POC_MAX_X \
    ((pet_i16_t)(PET_JIELI_DISPLAY_SAFE_X + PET_JIELI_DISPLAY_SAFE_W - PET2D_MOVEMENT_POC_SURFACE_SIZE))
#define PET2D_MOVEMENT_POC_MAX_Y \
    ((pet_i16_t)(PET_JIELI_DISPLAY_SAFE_Y + PET_JIELI_DISPLAY_SAFE_H - PET2D_MOVEMENT_POC_SURFACE_SIZE))
#define PET2D_MOVEMENT_POC_PATCH_SIZE PET2D_DIRTY_RECT_SIZE_64

static pet2d_movement_poc_state_t g_pet2d_movement_state;
static pet2d_movement_poc_stats_t g_pet2d_movement_stats;
static pet_i16_t g_pet2d_movement_min_x = PET2D_MOVEMENT_POC_MIN_X;
static pet_i16_t g_pet2d_movement_max_x = PET2D_MOVEMENT_POC_MAX_X;
static pet_i16_t g_pet2d_movement_patch_x;
static pet_i16_t g_pet2d_movement_patch_y;

static pet_u32_t pet2d_movement_now_ms(void)
{
    const pet_platform_t *platform = pet_platform_jieli_get();

    if ((platform == 0) || (platform->millis == 0)) {
        return 0u;
    }
    return platform->millis(platform->ctx);
}

static pet_u32_t pet2d_movement_elapsed_ms(pet_u32_t end_ms, pet_u32_t start_ms)
{
    return (pet_u32_t)(end_ms - start_ms);
}

static void pet2d_movement_wait_ms(pet_u16_t delay_ms)
{
    pet_u32_t start_ms;

    if (delay_ms == 0u) {
        return;
    }

    start_ms = pet2d_movement_now_ms();
    while (pet2d_movement_elapsed_ms(pet2d_movement_now_ms(), start_ms) < delay_ms) {
        /* Bounded manual board-test delay only. */
    }
}

static void pet2d_movement_update_latency_stats(pet_result_t ret)
{
    pet_u32_t elapsed;

    if (g_pet2d_movement_stats.last_key_timestamp_ms == 0u) {
        return;
    }

    g_pet2d_movement_stats.last_key_to_logic_ms =
        pet2d_movement_elapsed_ms(g_pet2d_movement_stats.last_logic_start_ms,
                                  g_pet2d_movement_stats.last_key_timestamp_ms);
    g_pet2d_movement_stats.last_key_to_render_ms =
        pet2d_movement_elapsed_ms(g_pet2d_movement_stats.last_render_start_ms,
                                  g_pet2d_movement_stats.last_key_timestamp_ms);
    g_pet2d_movement_stats.last_key_to_flush_done_ms =
        pet2d_movement_elapsed_ms(g_pet2d_movement_stats.last_flush_end_ms,
                                  g_pet2d_movement_stats.last_key_timestamp_ms);

    if (ret == PET_RESULT_OK) {
        elapsed = g_pet2d_movement_stats.last_key_to_flush_done_ms;
        if ((g_pet2d_movement_stats.render_success_count == 1u) ||
            (elapsed < g_pet2d_movement_stats.min_key_to_flush_done_ms)) {
            g_pet2d_movement_stats.min_key_to_flush_done_ms = elapsed;
        }
        if (elapsed > g_pet2d_movement_stats.max_key_to_flush_done_ms) {
            g_pet2d_movement_stats.max_key_to_flush_done_ms = elapsed;
        }
        g_pet2d_movement_stats.total_key_to_flush_done_ms += elapsed;
        g_pet2d_movement_stats.avg_key_to_flush_done_ms =
            g_pet2d_movement_stats.total_key_to_flush_done_ms /
            g_pet2d_movement_stats.render_success_count;
    }
}

static pet_key_t pet2d_movement_bounded_key(pet_key_t preferred_key)
{
    if ((preferred_key == PET_KEY_RIGHT_DOWN) &&
        (g_pet2d_movement_state.x >= g_pet2d_movement_max_x)) {
        return PET_KEY_LEFT_UP;
    }
    if ((preferred_key == PET_KEY_LEFT_UP) &&
        (g_pet2d_movement_state.x <= g_pet2d_movement_min_x)) {
        return PET_KEY_RIGHT_DOWN;
    }
    return preferred_key;
}

static pet_i16_t pet2d_movement_clamp_i16(pet_i16_t value, pet_i16_t min_value, pet_i16_t max_value)
{
    if (value < min_value) {
        return min_value;
    }
    if (value > max_value) {
        return max_value;
    }
    return value;
}

static pet_u16_t pet2d_movement_scene_background_pixel(pet_i16_t x, pet_i16_t y)
{
    pet_u16_t shade;

    shade = (pet_u16_t)((((pet_u16_t)x >> 4) ^ ((pet_u16_t)y >> 4)) & 0x01u);
    return shade ? 0x18c3u : 0x2104u;
}

static void pet2d_movement_record_dirty_rect(void)
{
    pet_i16_t x0 = g_pet2d_movement_state.last_x;
    pet_i16_t y0 = g_pet2d_movement_state.last_y;
    pet_i16_t x1 = g_pet2d_movement_state.x;
    pet_i16_t y1 = g_pet2d_movement_state.y;
    pet_i16_t min_x = (x0 < x1) ? x0 : x1;
    pet_i16_t min_y = (y0 < y1) ? y0 : y1;
    pet_i16_t max_x = (x0 > x1) ? x0 : x1;
    pet_i16_t max_y = (y0 > y1) ? y0 : y1;

    g_pet2d_movement_stats.last_dirty_x = min_x;
    g_pet2d_movement_stats.last_dirty_y = min_y;
    g_pet2d_movement_stats.last_dirty_w =
        (pet_u16_t)((max_x - min_x) + (pet_i16_t)g_pet2d_movement_state.surface_w);
    g_pet2d_movement_stats.last_dirty_h =
        (pet_u16_t)((max_y - min_y) + (pet_i16_t)g_pet2d_movement_state.surface_h);
}

static pet_result_t pet2d_movement_fill_background(pet2d_minimal_surface_t *surface,
                                                   pet_i16_t origin_x,
                                                   pet_i16_t origin_y)
{
    pet_u16_t x;
    pet_u16_t y;

    if ((surface == 0) || (surface->pixels == 0) ||
        (surface->width == 0u) || (surface->height == 0u) ||
        (surface->pitch_pixels < surface->width)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    for (y = 0u; y < surface->height; y++) {
        for (x = 0u; x < surface->width; x++) {
            surface->pixels[((pet_u32_t)y * surface->pitch_pixels) + x] =
                pet2d_movement_scene_background_pixel((pet_i16_t)(origin_x + (pet_i16_t)x),
                                                      (pet_i16_t)(origin_y + (pet_i16_t)y));
        }
    }
    return PET_RESULT_OK;
}

static pet_result_t pet2d_movement_fill_surface(pet2d_minimal_surface_t *surface)
{
    pet2d_resource_sprite_view_t sprite;
    pet_result_t ret;

    ret = pet2d_dirty_rect_poc_fill_pattern(PET2D_DIRTY_RECT_PATTERN_32, surface);
    if (ret != PET_RESULT_OK) {
        return ret;
    }

    if (g_pet2d_movement_state.pattern_toggle == 0u) {
        ret = pet2d_resource_sprite_poc_open(PET2D_RESOURCE_SPRITE_POC_ID_4X4, &sprite);
        if (ret != PET_RESULT_OK) {
            return ret;
        }
        return pet2d_minimal_visual_blit_sprite(surface,
                                                ((int)surface->width - (int)sprite.width) / 2,
                                                ((int)surface->height - (int)sprite.height) / 2,
                                                &sprite);
    }

    ret = pet2d_resource_sprite_poc_open(PET2D_RESOURCE_SPRITE_POC_ID_8X8, &sprite);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    return pet2d_minimal_visual_blit_sprite(surface, 12, 12, &sprite);
}

static pet_result_t pet2d_movement_fill_dirty_surface(pet2d_minimal_surface_t *surface,
                                                      pet_i16_t dirty_x,
                                                      pet_i16_t dirty_y)
{
    pet2d_minimal_surface_t sprite_surface;
    pet_i16_t rel_x;
    pet_i16_t rel_y;
    pet_result_t ret;

    if ((surface == 0) || (surface->pixels == 0) ||
        (surface->width == 0u) || (surface->height == 0u) ||
        (surface->pitch_pixels < surface->width)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    ret = pet2d_movement_fill_background(surface, dirty_x, dirty_y);
    if (ret != PET_RESULT_OK) {
        return ret;
    }

    rel_x = (pet_i16_t)(g_pet2d_movement_state.x - dirty_x);
    rel_y = (pet_i16_t)(g_pet2d_movement_state.y - dirty_y);
    if ((rel_x < 0) || (rel_y < 0) ||
        (((pet_u16_t)rel_x + g_pet2d_movement_state.surface_w) > surface->width) ||
        (((pet_u16_t)rel_y + g_pet2d_movement_state.surface_h) > surface->height)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    sprite_surface.width = g_pet2d_movement_state.surface_w;
    sprite_surface.height = g_pet2d_movement_state.surface_h;
    sprite_surface.pitch_pixels = surface->pitch_pixels;
    sprite_surface.pixels =
        &surface->pixels[((pet_u32_t)(pet_u16_t)rel_y * surface->pitch_pixels) +
                         (pet_u16_t)rel_x];
    return pet2d_movement_fill_surface(&sprite_surface);
}

pet_result_t pet2d_movement_poc_init(void)
{
    g_pet2d_movement_state.surface_w = PET2D_MOVEMENT_POC_SURFACE_SIZE;
    g_pet2d_movement_state.surface_h = PET2D_MOVEMENT_POC_SURFACE_SIZE;
    g_pet2d_movement_state.step = PET2D_MOVEMENT_POC_STEP;
    g_pet2d_movement_state.x =
        (pet_i16_t)(((int)PET_JIELI_DISPLAY_WIDTH - (int)PET2D_MOVEMENT_POC_SURFACE_SIZE) / 2);
    g_pet2d_movement_state.y =
        (pet_i16_t)(((int)PET_JIELI_DISPLAY_HEIGHT - (int)PET2D_MOVEMENT_POC_SURFACE_SIZE) / 2);
    g_pet2d_movement_state.last_x = g_pet2d_movement_state.x;
    g_pet2d_movement_state.last_y = g_pet2d_movement_state.y;
    g_pet2d_movement_state.frame_count = 0u;
    g_pet2d_movement_state.pattern_toggle = 0u;
    g_pet2d_movement_state.exit_requested = 0u;
    g_pet2d_movement_patch_x =
        (pet_i16_t)(g_pet2d_movement_state.x - (pet_i16_t)((PET2D_MOVEMENT_POC_PATCH_SIZE -
                                                            PET2D_MOVEMENT_POC_SURFACE_SIZE) / 2u));
    g_pet2d_movement_patch_y =
        (pet_i16_t)(g_pet2d_movement_state.y - (pet_i16_t)((PET2D_MOVEMENT_POC_PATCH_SIZE -
                                                            PET2D_MOVEMENT_POC_SURFACE_SIZE) / 2u));
    g_pet2d_movement_min_x = g_pet2d_movement_patch_x;
    g_pet2d_movement_max_x =
        (pet_i16_t)(g_pet2d_movement_patch_x + (pet_i16_t)PET2D_MOVEMENT_POC_PATCH_SIZE -
                    (pet_i16_t)PET2D_MOVEMENT_POC_SURFACE_SIZE);
    pet2d_movement_record_dirty_rect();
    return PET_RESULT_OK;
}

pet_result_t pet2d_movement_poc_reset_stats(void)
{
    pet_u8_t *bytes = (pet_u8_t *)&g_pet2d_movement_stats;
    pet_u32_t i;

    for (i = 0u; i < (pet_u32_t)sizeof(g_pet2d_movement_stats); i++) {
        bytes[i] = 0u;
    }
    g_pet2d_movement_stats.last_key = PET_KEY_MAX;
    g_pet2d_movement_stats.last_event = PET_KEY_EVENT_UP;
    g_pet2d_movement_stats.last_result = PET_RESULT_NOT_READY;
    g_pet2d_movement_stats.min_key_to_flush_done_ms = 0xffffffffu;
    return PET_RESULT_OK;
}

pet_result_t pet2d_movement_poc_clear_scene_background(void)
{
#if PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC
    const pet_platform_t *platform = pet_platform_jieli_get();
    pet_display_owner_t original_owner;
    pet_bool_t acquired_here = PET_FALSE;
    pet_result_t ret;
    static pet_u16_t pixels[PET2D_DIRTY_RECT_SIZE_64 * PET2D_DIRTY_RECT_SIZE_64];
    pet2d_minimal_surface_t surface;
    pet_u16_t patch_w = PET2D_MOVEMENT_POC_PATCH_SIZE;
    pet_u16_t patch_h = PET2D_MOVEMENT_POC_PATCH_SIZE;

    if ((platform == 0) || (platform->display_acquire == 0) ||
        (platform->display_release == 0)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    original_owner = pet_display_jieli_get_owner();
    if (original_owner == PET_DISPLAY_OWNER_NONE) {
        ret = platform->display_acquire(platform->ctx, PET_DISPLAY_OWNER_PET2D, 0u);
        if (ret != PET_RESULT_OK) {
            return ret;
        }
        acquired_here = PET_TRUE;
    } else if (original_owner != PET_DISPLAY_OWNER_PET2D) {
        return PET_RESULT_BUSY;
    }

    surface.pixels = pixels;
    surface.width = patch_w;
    surface.height = patch_h;
    surface.pitch_pixels = patch_w;
    ret = pet2d_movement_fill_background(&surface, g_pet2d_movement_patch_x,
                                         g_pet2d_movement_patch_y);
    if (ret == PET_RESULT_OK) {
        ret = pet_display_jieli_real_flush_poc_rect(g_pet2d_movement_patch_x,
                                                    g_pet2d_movement_patch_y,
                                                    patch_w,
                                                    patch_h,
                                                    surface.pixels,
                                                    surface.pitch_pixels);
    }
    if (ret != PET_RESULT_OK) {
        if (acquired_here == PET_TRUE) {
            (void)platform->display_release(platform->ctx, PET_DISPLAY_OWNER_PET2D);
        }
        return ret;
    }

    if (acquired_here == PET_TRUE) {
        (void)platform->display_release(platform->ctx, PET_DISPLAY_OWNER_PET2D);
    }
    return PET_RESULT_OK;
#else
    return PET_RESULT_UNSUPPORTED;
#endif
}

pet_result_t pet2d_movement_poc_get_state(pet2d_movement_poc_state_t *out_state)
{
    if (out_state == 0) {
        return PET_RESULT_INVALID_ARGUMENT;
    }
    *out_state = g_pet2d_movement_state;
    return PET_RESULT_OK;
}

pet_result_t pet2d_movement_poc_get_stats(pet2d_movement_poc_stats_t *out_stats)
{
    if (out_stats == 0) {
        return PET_RESULT_INVALID_ARGUMENT;
    }
    *out_stats = g_pet2d_movement_stats;
    return PET_RESULT_OK;
}

pet_result_t pet2d_movement_poc_handle_key(const pet_key_event_t *event)
{
    pet_i16_t next_x;
    pet_u32_t logic_start_ms;
    pet_u32_t key_timestamp_ms;

    if (event == 0) {
        return PET_RESULT_INVALID_ARGUMENT;
    }
    logic_start_ms = pet2d_movement_now_ms();
    key_timestamp_ms = (event->timestamp_ms == 0u) ? logic_start_ms : event->timestamp_ms;
    g_pet2d_movement_stats.key_event_count++;
    g_pet2d_movement_stats.last_key_timestamp_ms = key_timestamp_ms;
    g_pet2d_movement_stats.last_logic_start_ms = logic_start_ms;
    g_pet2d_movement_stats.last_key = event->key;
    g_pet2d_movement_stats.last_event = event->type;
    if (event->type != PET_KEY_EVENT_CLICK) {
        g_pet2d_movement_stats.last_logic_end_ms = pet2d_movement_now_ms();
        g_pet2d_movement_stats.last_result = PET_RESULT_OK;
        return PET_RESULT_OK;
    }

    g_pet2d_movement_state.last_x = g_pet2d_movement_state.x;
    g_pet2d_movement_state.last_y = g_pet2d_movement_state.y;
    g_pet2d_movement_stats.last_old_x = g_pet2d_movement_state.last_x;
    g_pet2d_movement_stats.last_old_y = g_pet2d_movement_state.last_y;

    switch (event->key) {
    case PET_KEY_LEFT_UP:
        next_x = (pet_i16_t)(g_pet2d_movement_state.x -
                             (pet_i16_t)g_pet2d_movement_state.step);
        g_pet2d_movement_state.x =
            pet2d_movement_clamp_i16(next_x, g_pet2d_movement_min_x,
                                     g_pet2d_movement_max_x);
        break;
    case PET_KEY_RIGHT_DOWN:
        next_x = (pet_i16_t)(g_pet2d_movement_state.x +
                             (pet_i16_t)g_pet2d_movement_state.step);
        g_pet2d_movement_state.x =
            pet2d_movement_clamp_i16(next_x, g_pet2d_movement_min_x,
                                     g_pet2d_movement_max_x);
        break;
    case PET_KEY_OK:
        g_pet2d_movement_state.pattern_toggle =
            (pet_u8_t)(g_pet2d_movement_state.pattern_toggle ? 0u : 1u);
        break;
    case PET_KEY_CANCEL:
        g_pet2d_movement_state.exit_requested = 1u;
        break;
    default:
        g_pet2d_movement_stats.last_logic_end_ms = pet2d_movement_now_ms();
        g_pet2d_movement_stats.last_result = PET_RESULT_UNSUPPORTED;
        return PET_RESULT_UNSUPPORTED;
    }

    g_pet2d_movement_state.frame_count++;
    g_pet2d_movement_stats.movement_step_count++;
    pet2d_movement_record_dirty_rect();
    g_pet2d_movement_stats.last_new_x = g_pet2d_movement_state.x;
    g_pet2d_movement_stats.last_new_y = g_pet2d_movement_state.y;
    g_pet2d_movement_stats.last_x = g_pet2d_movement_state.x;
    g_pet2d_movement_stats.last_y = g_pet2d_movement_state.y;
    g_pet2d_movement_stats.last_logic_end_ms = pet2d_movement_now_ms();
    g_pet2d_movement_stats.last_key_to_logic_ms =
        pet2d_movement_elapsed_ms(g_pet2d_movement_stats.last_logic_start_ms,
                                  g_pet2d_movement_stats.last_key_timestamp_ms);
    g_pet2d_movement_stats.last_result = PET_RESULT_OK;
    return PET_RESULT_OK;
}

pet_result_t pet2d_movement_poc_render_once(void)
{
#if PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC
    const pet_platform_t *platform = pet_platform_jieli_get();
    pet_display_owner_t original_owner;
    pet_bool_t acquired_here = PET_FALSE;
    pet_result_t ret;
    static pet_u16_t pixels[PET2D_DIRTY_RECT_SIZE_64 * PET2D_DIRTY_RECT_SIZE_64];
    pet2d_minimal_surface_t surface;
    pet_i16_t dirty_x;
    pet_i16_t dirty_y;
    pet_u16_t dirty_w;
    pet_u16_t dirty_h;

    if ((platform == 0) || (platform->display_acquire == 0) ||
        (platform->display_release == 0)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }
    if (g_pet2d_movement_state.exit_requested != 0u) {
        g_pet2d_movement_stats.last_result = PET_RESULT_UNSUPPORTED;
        return PET_RESULT_UNSUPPORTED;
    }

    g_pet2d_movement_stats.render_attempt_count++;
    g_pet2d_movement_stats.last_render_start_ms = pet2d_movement_now_ms();
    dirty_x = g_pet2d_movement_stats.last_dirty_x;
    dirty_y = g_pet2d_movement_stats.last_dirty_y;
    dirty_w = g_pet2d_movement_stats.last_dirty_w;
    dirty_h = g_pet2d_movement_stats.last_dirty_h;
    if ((dirty_w == 0u) || (dirty_h == 0u) ||
        (dirty_w > PET2D_DIRTY_RECT_SIZE_64) ||
        (dirty_h > PET2D_DIRTY_RECT_SIZE_64)) {
        dirty_x = g_pet2d_movement_state.x;
        dirty_y = g_pet2d_movement_state.y;
        dirty_w = g_pet2d_movement_state.surface_w;
        dirty_h = g_pet2d_movement_state.surface_h;
    }
    surface.width = dirty_w;
    surface.height = dirty_h;
    surface.pitch_pixels = dirty_w;
    surface.pixels = pixels;
    ret = pet2d_movement_fill_dirty_surface(&surface, dirty_x, dirty_y);
    if (ret != PET_RESULT_OK) {
        g_pet2d_movement_stats.render_fail_count++;
        g_pet2d_movement_stats.last_render_end_ms = pet2d_movement_now_ms();
        g_pet2d_movement_stats.last_result = ret;
        return ret;
    }

    original_owner = pet_display_jieli_get_owner();
    if (original_owner == PET_DISPLAY_OWNER_NONE) {
        ret = platform->display_acquire(platform->ctx, PET_DISPLAY_OWNER_PET2D, 0u);
        if (ret != PET_RESULT_OK) {
            g_pet2d_movement_stats.render_fail_count++;
            g_pet2d_movement_stats.last_render_end_ms = pet2d_movement_now_ms();
            g_pet2d_movement_stats.last_result = ret;
            return ret;
        }
        acquired_here = PET_TRUE;
    } else if (original_owner != PET_DISPLAY_OWNER_PET2D) {
        g_pet2d_movement_stats.render_fail_count++;
        g_pet2d_movement_stats.movement_probe_fail_count++;
        g_pet2d_movement_stats.last_render_end_ms = pet2d_movement_now_ms();
        g_pet2d_movement_stats.last_result = PET_RESULT_BUSY;
        return PET_RESULT_BUSY;
    }

    g_pet2d_movement_stats.movement_probe_attempt_count++;
    g_pet2d_movement_stats.last_flush_start_ms = pet2d_movement_now_ms();
    ret = pet_display_jieli_real_flush_poc_rect(dirty_x,
                                                dirty_y,
                                                surface.width,
                                                surface.height,
                                                surface.pixels,
                                                surface.pitch_pixels);
    g_pet2d_movement_stats.last_flush_end_ms = pet2d_movement_now_ms();
    g_pet2d_movement_stats.last_render_end_ms = g_pet2d_movement_stats.last_flush_end_ms;
    if (ret == PET_RESULT_OK) {
        g_pet2d_movement_stats.render_success_count++;
        g_pet2d_movement_stats.movement_probe_success_count++;
    } else {
        g_pet2d_movement_stats.render_fail_count++;
        g_pet2d_movement_stats.movement_probe_fail_count++;
    }
    g_pet2d_movement_stats.last_result = ret;
    pet2d_movement_update_latency_stats(ret);

    if (acquired_here == PET_TRUE) {
        (void)platform->display_release(platform->ctx, PET_DISPLAY_OWNER_PET2D);
    }
    return ret;
#else
    g_pet2d_movement_stats.last_result = PET_RESULT_UNSUPPORTED;
    return PET_RESULT_UNSUPPORTED;
#endif
}

pet_result_t pet2d_boundary_movement_probe_step(pet_key_t key)
{
    pet_key_event_t event;
    pet_result_t ret;

    if (g_pet2d_movement_state.surface_w == 0u) {
        ret = pet2d_movement_poc_init();
        if (ret != PET_RESULT_OK) {
            return ret;
        }
    }

    event.key = key;
    event.type = PET_KEY_EVENT_CLICK;
    event.timestamp_ms = pet2d_movement_now_ms();
    event.hold_ms = 0u;
    event.repeat_count = 0u;
    event.raw_code = 0xffffu;

    ret = pet2d_movement_poc_handle_key(&event);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    if (key == PET_KEY_CANCEL) {
        return PET_RESULT_OK;
    }
    return pet2d_movement_poc_render_once();
}

pet_result_t pet2d_movement_poc_run_repeated_steps(pet_key_t direction_key,
                                                   pet_u16_t repeat_count,
                                                   pet_u16_t delay_ms)
{
    pet_key_event_t event;
    pet_u16_t i;
    pet_key_t step_key;
    pet_result_t ret;

    if ((direction_key != PET_KEY_LEFT_UP) && (direction_key != PET_KEY_RIGHT_DOWN)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }
    if ((repeat_count == 0u) || (repeat_count > PET2D_MOVEMENT_POC_REPEAT_MAX)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }
    if (g_pet2d_movement_state.surface_w == 0u) {
        ret = pet2d_movement_poc_init();
        if (ret != PET_RESULT_OK) {
            return ret;
        }
    }

    for (i = 0u; i < repeat_count; i++) {
        step_key = pet2d_movement_bounded_key(direction_key);
        event.key = step_key;
        event.type = PET_KEY_EVENT_CLICK;
        event.timestamp_ms = pet2d_movement_now_ms();
        event.hold_ms = 0u;
        event.repeat_count = i;
        event.raw_code = (step_key == PET_KEY_LEFT_UP) ?
                         PET_JIELI_RAW_KEY_LEFT_UP : PET_JIELI_RAW_KEY_RIGHT_DOWN;

        ret = pet2d_movement_poc_handle_key(&event);
        if (ret != PET_RESULT_OK) {
            return ret;
        }
        ret = pet2d_movement_poc_render_once();
        if (ret != PET_RESULT_OK) {
            return ret;
        }
        pet2d_movement_wait_ms(delay_ms);
    }

    return PET_RESULT_OK;
}

pet_result_t pet2d_boundary_movement_repeated_probe(pet_u16_t repeat_count,
                                                    pet_u16_t delay_ms)
{
#if PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC
    const pet_platform_t *platform = pet_platform_jieli_get();
    pet_display_owner_t original_owner;
    pet_bool_t acquired_here = PET_FALSE;
    pet_result_t ret;

    if ((repeat_count == 0u) || (repeat_count > PET2D_MOVEMENT_POC_REPEAT_MAX)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }
    if ((platform == 0) || (platform->display_acquire == 0) ||
        (platform->display_release == 0)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    original_owner = pet_display_jieli_get_owner();
    if (original_owner == PET_DISPLAY_OWNER_NONE) {
        ret = platform->display_acquire(platform->ctx, PET_DISPLAY_OWNER_PET2D, 0u);
        if (ret != PET_RESULT_OK) {
            return ret;
        }
        acquired_here = PET_TRUE;
    } else if (original_owner != PET_DISPLAY_OWNER_PET2D) {
        return PET_RESULT_BUSY;
    }

    (void)pet2d_movement_poc_init();
    (void)pet2d_movement_poc_reset_stats();
    ret = pet2d_movement_poc_run_repeated_steps(PET_KEY_RIGHT_DOWN,
                                                repeat_count,
                                                delay_ms);

    if (acquired_here == PET_TRUE) {
        (void)platform->display_release(platform->ctx, PET_DISPLAY_OWNER_PET2D);
    }
    return ret;
#else
    if ((repeat_count == 0u) || (repeat_count > PET2D_MOVEMENT_POC_REPEAT_MAX)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }
    (void)delay_ms;
    return PET_RESULT_UNSUPPORTED;
#endif
}

pet_result_t pet2d_movement_poc_self_test(void)
{
    pet_key_event_t event;
    pet2d_movement_poc_state_t state;
    pet2d_movement_poc_stats_t stats;
    pet_result_t ret;

    ret = pet2d_movement_poc_init();
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    if (pet2d_movement_poc_reset_stats() != PET_RESULT_OK) {
        return PET_RESULT_ERROR;
    }
    if (pet2d_movement_poc_get_state(&state) != PET_RESULT_OK) {
        return PET_RESULT_ERROR;
    }

    event.key = PET_KEY_LEFT_UP;
    event.type = PET_KEY_EVENT_CLICK;
    event.timestamp_ms = 10u;
    event.hold_ms = 0u;
    event.repeat_count = 0u;
    event.raw_code = PET_JIELI_RAW_KEY_LEFT_UP;
    ret = pet2d_movement_poc_handle_key(&event);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    if (pet2d_movement_poc_get_state(&state) != PET_RESULT_OK) {
        return PET_RESULT_ERROR;
    }
    if (state.x >= state.last_x) {
        return PET_RESULT_ERROR;
    }
    if (pet2d_movement_poc_get_stats(&stats) != PET_RESULT_OK) {
        return PET_RESULT_ERROR;
    }
    if ((stats.last_old_x != state.last_x) ||
        (stats.last_new_x != state.x) ||
        (stats.last_dirty_w != (PET2D_MOVEMENT_POC_SURFACE_SIZE + PET2D_MOVEMENT_POC_STEP)) ||
        (stats.last_dirty_h != PET2D_MOVEMENT_POC_SURFACE_SIZE) ||
        (stats.key_event_count != 1u) ||
        (stats.movement_step_count != 1u)) {
        return PET_RESULT_ERROR;
    }

    event.key = PET_KEY_RIGHT_DOWN;
    ret = pet2d_movement_poc_handle_key(&event);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    event.key = PET_KEY_OK;
    ret = pet2d_movement_poc_handle_key(&event);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    if (pet2d_movement_poc_get_state(&state) != PET_RESULT_OK) {
        return PET_RESULT_ERROR;
    }
    if (state.pattern_toggle == 0u) {
        return PET_RESULT_ERROR;
    }

    if (pet2d_boundary_movement_repeated_probe(0u, 0u) != PET_RESULT_INVALID_ARGUMENT) {
        return PET_RESULT_ERROR;
    }
    if (pet2d_boundary_movement_repeated_probe(10u, 0u) != PET_RESULT_UNSUPPORTED) {
        return PET_RESULT_ERROR;
    }

    event.key = PET_KEY_CANCEL;
    ret = pet2d_movement_poc_handle_key(&event);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    if (pet2d_movement_poc_get_state(&state) != PET_RESULT_OK) {
        return PET_RESULT_ERROR;
    }
    if (state.exit_requested == 0u) {
        return PET_RESULT_ERROR;
    }
    if (pet2d_movement_poc_get_stats(&stats) != PET_RESULT_OK) {
        return PET_RESULT_ERROR;
    }
    if ((stats.last_dirty_w < PET2D_MOVEMENT_POC_SURFACE_SIZE) ||
        (stats.last_dirty_h < PET2D_MOVEMENT_POC_SURFACE_SIZE)) {
        return PET_RESULT_ERROR;
    }

    ret = pet2d_movement_poc_render_once();
    if (ret != PET_RESULT_UNSUPPORTED) {
        return PET_RESULT_ERROR;
    }

    return PET_RESULT_OK;
}
