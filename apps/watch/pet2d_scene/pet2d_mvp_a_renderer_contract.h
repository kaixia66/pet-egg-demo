#ifndef PET2D_MVP_A_RENDERER_CONTRACT_H
#define PET2D_MVP_A_RENDERER_CONTRACT_H

#include "pet_platform.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PET2D_MVP_A_RENDER_PLAN_MAX_CMDS 4u
#define PET2D_MVP_A_RENDER_FLAG_STAGE_RESTORE 0x01u
#define PET2D_MVP_A_RENDER_FLAG_PET_DRAW      0x02u

typedef enum {
    PET2D_MVP_A_RENDER_CMD_NONE = 0,
    PET2D_MVP_A_RENDER_CMD_STAGE_PATCH,
    PET2D_MVP_A_RENDER_CMD_PET_PLACEHOLDER,
    PET2D_MVP_A_RENDER_CMD_CLEAR_DIRTY
} pet2d_mvp_a_render_cmd_type_t;

typedef enum {
    PET2D_MVP_A_RENDER_PATTERN_STAGE = 0,
    PET2D_MVP_A_RENDER_PATTERN_IDLE,
    PET2D_MVP_A_RENDER_PATTERN_HAPPY,
    PET2D_MVP_A_RENDER_PATTERN_BLINK,
    PET2D_MVP_A_RENDER_PATTERN_STEP
} pet2d_mvp_a_render_pattern_t;

typedef struct {
    pet_i16_t x;
    pet_i16_t y;
    pet_u16_t w;
    pet_u16_t h;
} pet2d_mvp_a_rect_t;

typedef struct {
    pet2d_mvp_a_render_cmd_type_t type;
    pet2d_mvp_a_rect_t dst;
    pet2d_mvp_a_render_pattern_t pattern;
    pet_u8_t pose;
    pet_u8_t alpha_mode;
    pet_u8_t flags;
    pet_u8_t reserved;
} pet2d_mvp_a_render_cmd_t;

typedef struct {
    pet2d_mvp_a_rect_t stage_rect;
    pet2d_mvp_a_rect_t prev_pet_rect;
    pet2d_mvp_a_rect_t pet_rect;
    pet2d_mvp_a_rect_t dirty_rect;
    pet2d_mvp_a_render_cmd_t cmds[PET2D_MVP_A_RENDER_PLAN_MAX_CMDS];
    pet_u8_t cmd_count;
    pet_u8_t needs_stage_restore;
    pet_u8_t needs_pet_draw;
    pet_u8_t skipped_flush;
} pet2d_mvp_a_render_plan_t;

typedef struct {
    pet_u32_t frame_index;
    pet_u16_t cmd_count;
    pet_u16_t dirty_w;
    pet_u16_t dirty_h;
    pet_u16_t render_time_ms;
    pet_u16_t flush_time_ms;
    pet_u16_t frame_time_ms;
    pet_u16_t skipped_flush_count;
    pet_u16_t render_fail_count;
    pet_u16_t flush_fail_count;
} pet2d_mvp_a_render_stats_t;

pet_bool_t pet2d_mvp_a_rect_is_valid(const pet2d_mvp_a_rect_t *rect);
pet_bool_t pet2d_mvp_a_rect_equal(const pet2d_mvp_a_rect_t *a,
                                  const pet2d_mvp_a_rect_t *b);
pet_u32_t pet2d_mvp_a_rect_area(const pet2d_mvp_a_rect_t *rect);
pet_result_t pet2d_mvp_a_rect_union(const pet2d_mvp_a_rect_t *a,
                                    const pet2d_mvp_a_rect_t *b,
                                    pet2d_mvp_a_rect_t *out_rect);
pet_result_t pet2d_mvp_a_rect_clip_to_stage(const pet2d_mvp_a_rect_t *stage,
                                            const pet2d_mvp_a_rect_t *rect,
                                            pet2d_mvp_a_rect_t *out_rect);
pet_result_t pet2d_mvp_a_rect_clamp_pet_to_stage(
    const pet2d_mvp_a_rect_t *stage,
    pet2d_mvp_a_rect_t *pet_rect);

pet2d_mvp_a_render_pattern_t pet2d_mvp_a_renderer_pattern_from_pose(
    pet_u8_t pose);
pet_result_t pet2d_mvp_a_renderer_build_initial_plan(
    const pet2d_mvp_a_rect_t *stage,
    const pet2d_mvp_a_rect_t *pet_rect,
    pet_u8_t pose,
    pet2d_mvp_a_render_plan_t *out_plan);
pet_result_t pet2d_mvp_a_renderer_build_pet_change_plan(
    const pet2d_mvp_a_rect_t *stage,
    const pet2d_mvp_a_rect_t *prev_pet_rect,
    const pet2d_mvp_a_rect_t *pet_rect,
    pet_u8_t pose,
    pet_bool_t restore_stage,
    pet2d_mvp_a_render_plan_t *out_plan);
pet_result_t pet2d_mvp_a_renderer_build_idle_plan(
    pet2d_mvp_a_render_plan_t *out_plan);
pet_result_t pet2d_mvp_a_renderer_contract_self_test(void);

#ifdef __cplusplus
}
#endif

#endif
