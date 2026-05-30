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

static pet2d_movement_poc_state_t g_pet2d_movement_state;
static pet2d_movement_poc_stats_t g_pet2d_movement_stats;

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
    return PET_RESULT_OK;
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

    if (event == 0) {
        return PET_RESULT_INVALID_ARGUMENT;
    }
    if (event->type != PET_KEY_EVENT_CLICK) {
        return PET_RESULT_OK;
    }

    g_pet2d_movement_state.last_x = g_pet2d_movement_state.x;
    g_pet2d_movement_state.last_y = g_pet2d_movement_state.y;
    g_pet2d_movement_stats.last_key = event->key;
    g_pet2d_movement_stats.last_event = event->type;

    switch (event->key) {
    case PET_KEY_LEFT_UP:
        next_x = (pet_i16_t)(g_pet2d_movement_state.x -
                             (pet_i16_t)g_pet2d_movement_state.step);
        g_pet2d_movement_state.x =
            pet2d_movement_clamp_i16(next_x, PET2D_MOVEMENT_POC_MIN_X,
                                     PET2D_MOVEMENT_POC_MAX_X);
        break;
    case PET_KEY_RIGHT_DOWN:
        next_x = (pet_i16_t)(g_pet2d_movement_state.x +
                             (pet_i16_t)g_pet2d_movement_state.step);
        g_pet2d_movement_state.x =
            pet2d_movement_clamp_i16(next_x, PET2D_MOVEMENT_POC_MIN_X,
                                     PET2D_MOVEMENT_POC_MAX_X);
        break;
    case PET_KEY_OK:
        g_pet2d_movement_state.pattern_toggle =
            (pet_u8_t)(g_pet2d_movement_state.pattern_toggle ? 0u : 1u);
        break;
    case PET_KEY_CANCEL:
        g_pet2d_movement_state.exit_requested = 1u;
        break;
    default:
        return PET_RESULT_UNSUPPORTED;
    }

    g_pet2d_movement_state.frame_count++;
    pet2d_movement_record_dirty_rect();
    g_pet2d_movement_stats.last_x = g_pet2d_movement_state.x;
    g_pet2d_movement_stats.last_y = g_pet2d_movement_state.y;
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
    static pet_u16_t pixels[PET2D_MOVEMENT_POC_SURFACE_SIZE * PET2D_MOVEMENT_POC_SURFACE_SIZE];
    pet2d_minimal_surface_t surface;

    if ((platform == 0) || (platform->display_acquire == 0) ||
        (platform->display_release == 0)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }
    if (g_pet2d_movement_state.exit_requested != 0u) {
        return PET_RESULT_UNSUPPORTED;
    }

    surface.width = PET2D_MOVEMENT_POC_SURFACE_SIZE;
    surface.height = PET2D_MOVEMENT_POC_SURFACE_SIZE;
    surface.pitch_pixels = PET2D_MOVEMENT_POC_SURFACE_SIZE;
    surface.pixels = pixels;
    ret = pet2d_movement_fill_surface(&surface);
    if (ret != PET_RESULT_OK) {
        return ret;
    }

    original_owner = pet_display_jieli_get_owner();
    if (original_owner == PET_DISPLAY_OWNER_NONE) {
        ret = platform->display_acquire(platform->ctx, PET_DISPLAY_OWNER_PET2D, 0u);
        if (ret != PET_RESULT_OK) {
            return ret;
        }
        acquired_here = PET_TRUE;
    } else if (original_owner != PET_DISPLAY_OWNER_PET2D) {
        g_pet2d_movement_stats.movement_probe_fail_count++;
        g_pet2d_movement_stats.last_result = PET_RESULT_BUSY;
        return PET_RESULT_BUSY;
    }

    g_pet2d_movement_stats.movement_probe_attempt_count++;
    ret = pet_display_jieli_real_flush_poc_rect(g_pet2d_movement_state.x,
                                                g_pet2d_movement_state.y,
                                                surface.width,
                                                surface.height,
                                                surface.pixels,
                                                surface.pitch_pixels);
    if (ret == PET_RESULT_OK) {
        g_pet2d_movement_stats.movement_probe_success_count++;
    } else {
        g_pet2d_movement_stats.movement_probe_fail_count++;
    }
    g_pet2d_movement_stats.last_result = ret;

    if (acquired_here == PET_TRUE) {
        (void)platform->display_release(platform->ctx, PET_DISPLAY_OWNER_PET2D);
    }
    return ret;
#else
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
    event.timestamp_ms = 0u;
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
