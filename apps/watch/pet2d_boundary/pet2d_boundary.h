#ifndef PET2D_BOUNDARY_H
#define PET2D_BOUNDARY_H

#include "pet2d_dirty_rect_poc.h"
#include "pet2d_movement_poc.h"
#include "pet2d_resource_sprite_poc.h"
#include "pet_platform.h"

#ifdef __cplusplus
extern "C" {
#endif

pet_result_t pet2d_boundary_enter_placeholder(void);
pet_result_t pet2d_boundary_exit_placeholder(void);
pet_result_t pet2d_boundary_self_test(void);
pet_result_t pet2d_boundary_resource_probe_self_test(void);
pet_result_t pet2d_boundary_tiny_visual_probe(void);
pet_result_t pet2d_boundary_minimal_real_flush_probe(void);

typedef struct {
    pet2d_dirty_rect_pattern_t pattern;
    pet_u16_t repeat_count;
    pet_i16_t x;
    pet_i16_t y;
    pet_u16_t delay_ms;
    pet_u8_t move_each_flush;
} pet2d_repeated_flush_config_t;

typedef struct {
    pet_u32_t repeated_probe_attempt_count;
    pet_u32_t repeated_probe_success_count;
    pet_u32_t repeated_probe_fail_count;
    pet_u16_t last_pattern_size;
    pet_u16_t last_repeat_count;
    pet_u16_t last_success_count;
    pet_u16_t last_fail_index;
    pet_i16_t last_rect_x;
    pet_i16_t last_rect_y;
    pet_u16_t last_rect_w;
    pet_u16_t last_rect_h;
    pet_result_t last_result;
    pet_u32_t max_single_flush_ms;
    pet_u32_t total_flush_ms;
} pet2d_boundary_repeated_flush_stats_t;

pet_result_t pet2d_boundary_repeated_flush_probe(const pet2d_repeated_flush_config_t *config);
pet_result_t pet2d_boundary_repeated_flush_default_probe(void);
pet_result_t pet2d_boundary_get_repeated_flush_stats(
    pet2d_boundary_repeated_flush_stats_t *out_stats);
pet_result_t pet2d_boundary_reset_repeated_flush_stats(void);
pet_result_t pet2d_boundary_repeated_flush_gate_self_test(void);

typedef struct {
    pet_u32_t resource_sprite_probe_attempt_count;
    pet_u32_t resource_sprite_probe_success_count;
    pet_u32_t resource_sprite_probe_fail_count;
    pet_u32_t last_resource_id;
    pet_u16_t last_surface_w;
    pet_u16_t last_surface_h;
    pet_u16_t last_sprite_w;
    pet_u16_t last_sprite_h;
    pet_result_t last_result;
} pet2d_boundary_resource_sprite_stats_t;

pet_result_t pet2d_boundary_resource_sprite_flush_probe(void);
pet_result_t pet2d_boundary_resource_sprite_gate_self_test(void);
pet_result_t pet2d_boundary_get_resource_sprite_stats(
    pet2d_boundary_resource_sprite_stats_t *out_stats);
pet_result_t pet2d_boundary_reset_resource_sprite_stats(void);

#ifdef __cplusplus
}
#endif

#endif
