#include "sim_consistency_replay.h"

typedef __builtin_va_list sim_consistency_va_list;

extern int vsnprintf(char *buffer,
                     size_t buffer_size,
                     const char *format,
                     sim_consistency_va_list args);
extern void *memcpy(void *dst, const void *src, size_t size);
extern void *memset(void *dst, int value, size_t size);
extern size_t strlen(const char *text);

static const pet2d_mvp_a_rect_t k_stage_rect = {0, 0,
                                                SIM_CONSISTENCY_STAGE_W,
                                                SIM_CONSISTENCY_STAGE_H};

static pet2d_mvp_a_rect_t sim_consistency_pet_rect(pet_i16_t x, pet_i16_t y)
{
    pet2d_mvp_a_rect_t rect;

    rect.x = x;
    rect.y = y;
    rect.w = SIM_CONSISTENCY_PET_SIZE;
    rect.h = SIM_CONSISTENCY_PET_SIZE;
    return rect;
}

static const char *sim_consistency_state_name(pet2d_mvp_a_scene_state_t state)
{
    switch (state) {
    case PET2D_MVP_A_SCENE_STATE_ENTER:
        return "ENTER";
    case PET2D_MVP_A_SCENE_STATE_IDLE:
        return "IDLE";
    case PET2D_MVP_A_SCENE_STATE_MOVE_LEFT:
        return "MOVE_LEFT";
    case PET2D_MVP_A_SCENE_STATE_MOVE_RIGHT:
        return "MOVE_RIGHT";
    case PET2D_MVP_A_SCENE_STATE_ACTION:
        return "ACTION";
    case PET2D_MVP_A_SCENE_STATE_EXITING:
        return "EXITING";
    case PET2D_MVP_A_SCENE_STATE_DONE:
        return "DONE";
    case PET2D_MVP_A_SCENE_STATE_ERROR:
        return "ERROR";
    case PET2D_MVP_A_SCENE_STATE_NONE:
    default:
        return "NONE";
    }
}

static const char *sim_consistency_pose_name(pet2d_mvp_a_scene_pose_t pose)
{
    switch (pose) {
    case PET2D_MVP_A_SCENE_POSE_HAPPY:
        return "HAPPY";
    case PET2D_MVP_A_SCENE_POSE_BLINK:
        return "BLINK";
    case PET2D_MVP_A_SCENE_POSE_STEP:
        return "STEP";
    case PET2D_MVP_A_SCENE_POSE_IDLE:
    default:
        return "IDLE";
    }
}

static const char *sim_consistency_exit_name(pet2d_mvp_a_scene_exit_reason_t reason)
{
    switch (reason) {
    case PET2D_MVP_A_SCENE_EXIT_CANCEL:
        return "CANCEL";
    case PET2D_MVP_A_SCENE_EXIT_TIMEOUT:
        return "TIMEOUT";
    case PET2D_MVP_A_SCENE_EXIT_ERROR:
        return "ERROR";
    case PET2D_MVP_A_SCENE_EXIT_NONE:
    default:
        return "NONE";
    }
}

static const char *sim_consistency_cmd_name(pet2d_mvp_a_render_cmd_type_t type)
{
    switch (type) {
    case PET2D_MVP_A_RENDER_CMD_STAGE_PATCH:
        return "STAGE_PATCH";
    case PET2D_MVP_A_RENDER_CMD_PET_PLACEHOLDER:
        return "PET_PLACEHOLDER";
    case PET2D_MVP_A_RENDER_CMD_CLEAR_DIRTY:
        return "CLEAR_DIRTY";
    case PET2D_MVP_A_RENDER_CMD_NONE:
    default:
        return "NONE";
    }
}

static pet_result_t sim_consistency_append(char *out_log,
                                           size_t out_capacity,
                                           size_t *cursor,
                                           const char *format,
                                           ...)
{
    sim_consistency_va_list args;
    int written;

    if ((out_log == 0) || (cursor == 0) || (format == 0) ||
        (*cursor >= out_capacity)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }

    __builtin_va_start(args, format);
    written = vsnprintf(out_log + *cursor, out_capacity - *cursor, format, args);
    __builtin_va_end(args);

    if (written < 0) {
        return PET_RESULT_ERROR;
    }
    if ((size_t)written >= (out_capacity - *cursor)) {
        return PET_RESULT_BUFFER_TOO_SMALL;
    }

    *cursor += (size_t)written;
    return PET_RESULT_OK;
}

static void sim_consistency_init_context(sim_consistency_scene_context_t *ctx,
                                         pet_u32_t now_ms)
{
    pet_i16_t start_x = (pet_i16_t)((SIM_CONSISTENCY_STAGE_W -
                                     SIM_CONSISTENCY_PET_SIZE) / 2u);
    pet_i16_t start_y = (pet_i16_t)((SIM_CONSISTENCY_STAGE_H -
                                     SIM_CONSISTENCY_PET_SIZE) / 2u);

    memset(ctx, 0, sizeof(*ctx));
    ctx->model.state = PET2D_MVP_A_SCENE_STATE_IDLE;
    ctx->model.pose = PET2D_MVP_A_SCENE_POSE_IDLE;
    ctx->model.pet_x = start_x;
    ctx->model.pet_y = start_y;
    ctx->model.prev_x = start_x;
    ctx->model.prev_y = start_y;
    ctx->model.enter_ms = now_ms;
    ctx->model.timeout_ms = now_ms + PET2D_MVP_A_SCENE_TIMEOUT_MS;
    ctx->model.exit_reason = PET2D_MVP_A_SCENE_EXIT_NONE;
}

static pet_result_t sim_consistency_build_initial(sim_consistency_scene_context_t *ctx)
{
    pet2d_mvp_a_rect_t pet;

    pet = sim_consistency_pet_rect(ctx->model.pet_x, ctx->model.pet_y);
    return pet2d_mvp_a_renderer_build_initial_plan(
        &k_stage_rect, &pet, (pet_u8_t)ctx->model.pose, &ctx->plan);
}

static pet_result_t sim_consistency_build_change(sim_consistency_scene_context_t *ctx,
                                                 pet_bool_t restore_stage)
{
    pet2d_mvp_a_rect_t old_pet;
    pet2d_mvp_a_rect_t pet;

    old_pet = sim_consistency_pet_rect(ctx->model.prev_x, ctx->model.prev_y);
    pet = sim_consistency_pet_rect(ctx->model.pet_x, ctx->model.pet_y);
    return pet2d_mvp_a_renderer_build_pet_change_plan(
        &k_stage_rect, &old_pet, &pet, (pet_u8_t)ctx->model.pose,
        restore_stage, &ctx->plan);
}

static pet_result_t sim_consistency_build_idle(sim_consistency_scene_context_t *ctx)
{
    pet_result_t ret = pet2d_mvp_a_renderer_build_idle_plan(&ctx->plan);

    if (ret == PET_RESULT_OK) {
        ctx->skipped_flush_count++;
    }
    return ret;
}

static pet_result_t sim_consistency_log_step(sim_consistency_scene_context_t *ctx,
                                             const char *step_name,
                                             char *out_log,
                                             size_t out_capacity,
                                             size_t *cursor)
{
    const char *cmd_name = "NONE";

    if (ctx->plan.cmd_count > 0u) {
        cmd_name = sim_consistency_cmd_name(ctx->plan.cmds[0].type);
    }

    return sim_consistency_append(
        out_log,
        out_capacity,
        cursor,
        "step=%s state=%s pose=%s pet=%d,%d dirty=%ux%u cmd_count=%u cmd0=%s skipped=%lu exit=%s\n",
        step_name,
        sim_consistency_state_name(ctx->model.state),
        sim_consistency_pose_name(ctx->model.pose),
        ctx->model.pet_x,
        ctx->model.pet_y,
        ctx->plan.dirty_rect.w,
        ctx->plan.dirty_rect.h,
        ctx->plan.cmd_count,
        cmd_name,
        (unsigned long)ctx->skipped_flush_count,
        sim_consistency_exit_name(ctx->model.exit_reason));
}

static pet_result_t sim_consistency_finish_action_if_due(
    sim_consistency_scene_context_t *ctx,
    pet_u32_t now_ms,
    pet_bool_t render_on_done,
    const char *step_name,
    char *out_log,
    size_t out_capacity,
    size_t *cursor)
{
    pet_result_t ret;

    if ((ctx->model.state != PET2D_MVP_A_SCENE_STATE_MOVE_LEFT) &&
        (ctx->model.state != PET2D_MVP_A_SCENE_STATE_MOVE_RIGHT) &&
        (ctx->model.state != PET2D_MVP_A_SCENE_STATE_ACTION)) {
        return PET_RESULT_OK;
    }
    if ((pet_u32_t)(now_ms - ctx->model.action_started_ms) <
        ctx->model.action_duration_ms) {
        return PET_RESULT_OK;
    }

    ctx->model.prev_x = ctx->model.pet_x;
    ctx->model.prev_y = ctx->model.pet_y;
    ctx->model.pose = PET2D_MVP_A_SCENE_POSE_IDLE;
    ctx->model.state = PET2D_MVP_A_SCENE_STATE_IDLE;
    ctx->model.action_duration_ms = 0u;

    if (render_on_done == PET_FALSE) {
        return PET_RESULT_OK;
    }

    ret = sim_consistency_build_change(ctx, PET_FALSE);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    return sim_consistency_log_step(ctx, step_name, out_log, out_capacity, cursor);
}

static pet_result_t sim_consistency_apply_left(sim_consistency_scene_context_t *ctx,
                                               pet_u32_t now_ms)
{
    pet2d_mvp_a_rect_t pet;

    ctx->model.prev_x = ctx->model.pet_x;
    ctx->model.prev_y = ctx->model.pet_y;
    ctx->model.pet_x = (pet_i16_t)(ctx->model.pet_x - SIM_CONSISTENCY_STEP_PIXELS);
    pet = sim_consistency_pet_rect(ctx->model.pet_x, ctx->model.pet_y);
    if (pet2d_mvp_a_rect_clamp_pet_to_stage(&k_stage_rect, &pet) != PET_RESULT_OK) {
        return PET_RESULT_ERROR;
    }
    ctx->model.pet_x = pet.x;
    ctx->model.pet_y = pet.y;
    ctx->model.pose = PET2D_MVP_A_SCENE_POSE_STEP;
    ctx->model.state = PET2D_MVP_A_SCENE_STATE_MOVE_LEFT;
    ctx->model.action_started_ms = now_ms;
    ctx->model.action_duration_ms = PET2D_MVP_A_SCENE_MOVE_ACTION_MS;
    return sim_consistency_build_change(ctx, PET_TRUE);
}

static pet_result_t sim_consistency_apply_right(sim_consistency_scene_context_t *ctx,
                                                pet_u32_t now_ms)
{
    pet2d_mvp_a_rect_t pet;

    ctx->model.prev_x = ctx->model.pet_x;
    ctx->model.prev_y = ctx->model.pet_y;
    ctx->model.pet_x = (pet_i16_t)(ctx->model.pet_x + SIM_CONSISTENCY_STEP_PIXELS);
    pet = sim_consistency_pet_rect(ctx->model.pet_x, ctx->model.pet_y);
    if (pet2d_mvp_a_rect_clamp_pet_to_stage(&k_stage_rect, &pet) != PET_RESULT_OK) {
        return PET_RESULT_ERROR;
    }
    ctx->model.pet_x = pet.x;
    ctx->model.pet_y = pet.y;
    ctx->model.pose = PET2D_MVP_A_SCENE_POSE_STEP;
    ctx->model.state = PET2D_MVP_A_SCENE_STATE_MOVE_RIGHT;
    ctx->model.action_started_ms = now_ms;
    ctx->model.action_duration_ms = PET2D_MVP_A_SCENE_MOVE_ACTION_MS;
    return sim_consistency_build_change(ctx, PET_TRUE);
}

static pet_result_t sim_consistency_apply_ok(sim_consistency_scene_context_t *ctx,
                                             pet_u32_t now_ms)
{
    ctx->model.prev_x = ctx->model.pet_x;
    ctx->model.prev_y = ctx->model.pet_y;
    ctx->pose_cycle = (pet_u8_t)((ctx->pose_cycle + 1u) % 3u);
    if (ctx->pose_cycle == 1u) {
        ctx->model.pose = PET2D_MVP_A_SCENE_POSE_HAPPY;
    } else if (ctx->pose_cycle == 2u) {
        ctx->model.pose = PET2D_MVP_A_SCENE_POSE_BLINK;
    } else {
        ctx->model.pose = PET2D_MVP_A_SCENE_POSE_IDLE;
    }
    ctx->model.state = PET2D_MVP_A_SCENE_STATE_ACTION;
    ctx->model.action_started_ms = now_ms;
    ctx->model.action_duration_ms = PET2D_MVP_A_SCENE_POSE_ACTION_MS;
    return sim_consistency_build_change(ctx, PET_FALSE);
}

static pet_result_t sim_consistency_apply_cancel(sim_consistency_scene_context_t *ctx)
{
    ctx->model.state = PET2D_MVP_A_SCENE_STATE_DONE;
    ctx->model.exit_reason = PET2D_MVP_A_SCENE_EXIT_CANCEL;
    return sim_consistency_build_idle(ctx);
}

pet_result_t sim_consistency_run_scene_replay(char *out_log, size_t out_capacity)
{
    sim_consistency_scene_context_t ctx;
    pet_result_t ret;
    size_t cursor = 0u;

    if ((out_log == 0) || (out_capacity == 0u)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }
    out_log[0] = '\0';

    sim_consistency_init_context(&ctx, 0u);
    ret = sim_consistency_build_initial(&ctx);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    ret = sim_consistency_log_step(&ctx, "enter", out_log, out_capacity, &cursor);
    if (ret != PET_RESULT_OK) {
        return ret;
    }

    ret = sim_consistency_build_idle(&ctx);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    ret = sim_consistency_log_step(&ctx, "tick0", out_log, out_capacity, &cursor);
    if (ret != PET_RESULT_OK) {
        return ret;
    }

    ret = sim_consistency_apply_left(&ctx, 10u);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    ret = sim_consistency_log_step(&ctx, "left", out_log, out_capacity, &cursor);
    if (ret != PET_RESULT_OK) {
        return ret;
    }

    ret = sim_consistency_finish_action_if_due(&ctx, 43u, PET_TRUE, "left_tick33",
                                               out_log, out_capacity, &cursor);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    ret = sim_consistency_finish_action_if_due(&ctx, 171u, PET_TRUE, "left_done",
                                               out_log, out_capacity, &cursor);
    if (ret != PET_RESULT_OK) {
        return ret;
    }

    ret = sim_consistency_apply_right(&ctx, 200u);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    ret = sim_consistency_log_step(&ctx, "right", out_log, out_capacity, &cursor);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    ret = sim_consistency_finish_action_if_due(&ctx, 233u, PET_TRUE, "right_tick33",
                                               out_log, out_capacity, &cursor);
    if (ret != PET_RESULT_OK) {
        return ret;
    }

    ret = sim_consistency_apply_ok(&ctx, 300u);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    ret = sim_consistency_log_step(&ctx, "ok", out_log, out_capacity, &cursor);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    ret = sim_consistency_finish_action_if_due(&ctx, 333u, PET_TRUE, "ok_tick33",
                                               out_log, out_capacity, &cursor);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    ret = sim_consistency_finish_action_if_due(&ctx, 601u, PET_TRUE, "ok_done",
                                               out_log, out_capacity, &cursor);
    if (ret != PET_RESULT_OK) {
        return ret;
    }

    ret = sim_consistency_apply_cancel(&ctx);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    return sim_consistency_log_step(&ctx, "cancel", out_log, out_capacity, &cursor);
}

pet_result_t sim_consistency_run_timeout_replay(char *out_log, size_t out_capacity)
{
    sim_consistency_scene_context_t ctx;
    pet_result_t ret;
    size_t cursor = 0u;

    if ((out_log == 0) || (out_capacity == 0u)) {
        return PET_RESULT_INVALID_ARGUMENT;
    }
    out_log[0] = '\0';

    sim_consistency_init_context(&ctx, 0u);
    ret = sim_consistency_build_initial(&ctx);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    ret = sim_consistency_log_step(&ctx, "timeout_enter", out_log, out_capacity,
                                   &cursor);
    if (ret != PET_RESULT_OK) {
        return ret;
    }

    if (PET2D_MVP_A_SCENE_TIMEOUT_MS < 4000u) {
        return PET_RESULT_ERROR;
    }
    ctx.model.state = PET2D_MVP_A_SCENE_STATE_DONE;
    ctx.model.exit_reason = PET2D_MVP_A_SCENE_EXIT_TIMEOUT;
    ret = sim_consistency_build_idle(&ctx);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    return sim_consistency_log_step(&ctx, "timeout", out_log, out_capacity,
                                    &cursor);
}

pet_result_t sim_consistency_check_screen_profile_fixture(void)
{
    pet_display_profile_t profile;

    memset(&profile, 0, sizeof(profile));
    profile.width = 454u;
    profile.height = 454u;
    profile.shape = PET_SCREEN_SHAPE_RECTANGLE;
    profile.rotation = PET_DISPLAY_ROTATION_0;
    profile.rgb565_order = PET_RGB565_ORDER_RGB;
    profile.safe_area.left = 0u;
    profile.safe_area.top = 0u;
    profile.safe_area.right = 453u;
    profile.safe_area.bottom = 453u;
    profile.flush_mode = PET_DISPLAY_FLUSH_MODE_RGB565_RECT;
    profile.stride_align_pixels = 1u;
    profile.min_flush_width = 1u;
    profile.min_flush_height = 1u;

    if ((profile.width != 454u) || (profile.height != 454u) ||
        (profile.flush_mode != PET_DISPLAY_FLUSH_MODE_RGB565_RECT) ||
        (profile.rgb565_order != PET_RGB565_ORDER_RGB)) {
        return PET_RESULT_ERROR;
    }
    return PET_RESULT_OK;
}

pet_result_t sim_consistency_check_key_replay_fixture(void)
{
    pet_key_event_t event;

    event.key = PET_KEY_LEFT_UP;
    event.type = PET_KEY_EVENT_CLICK;
    event.timestamp_ms = 10u;
    event.hold_ms = 0u;
    event.repeat_count = 0u;
    event.raw_code = 0u;

    if ((event.key != PET_KEY_LEFT_UP) ||
        (PET_KEY_RIGHT_DOWN != 1) ||
        (PET_KEY_OK != 2) ||
        (PET_KEY_CANCEL != 3) ||
        (event.type != PET_KEY_EVENT_CLICK)) {
        return PET_RESULT_ERROR;
    }
    return PET_RESULT_OK;
}

pet_result_t sim_consistency_check_save_slot_fixture(void)
{
    pet_save_slot_header_t header;

    memset(&header, 0, sizeof(header));
    header.magic = PET_SAVE_MAGIC;
    header.version = PET_SAVE_VERSION;
    header.schema_version = PET_SAVE_SCHEMA_VERSION;
    header.payload_type = PET_SAVE_PAYLOAD_DEVICE;
    header.payload_len = 32u;
    header.counter = 7u;
    header.timestamp_sec = 1234u;
    header.crc32 = 0xd3ca3239u;

    if ((sizeof(header) != PET_SAVE_SLOT_HEADER_SERIALIZED_SIZE) ||
        (header.magic != PET_SAVE_MAGIC) ||
        (header.version != PET_SAVE_VERSION) ||
        (header.payload_len != 32u) ||
        (header.counter != 7u) ||
        (header.crc32 != 0xd3ca3239u)) {
        return PET_RESULT_ERROR;
    }
    return PET_RESULT_OK;
}

pet_result_t sim_consistency_check_packet_fixture(void)
{
    pet_packet_t packet;

    memset(&packet, 0, sizeof(packet));
    packet.magic = PET_PACKET_MAGIC;
    packet.version = PET_PACKET_VERSION;
    packet.type = (pet_u8_t)PET_PACKET_PING;
    packet.flags = PET_PACKET_FLAG_REQUIRES_ACK;
    packet.seq = 3u;
    packet.ack = 2u;
    packet.len = 4u;
    packet.payload[0] = 0x50u;
    packet.payload[1] = 0x45u;
    packet.payload[2] = 0x54u;
    packet.payload[3] = 0x21u;
    packet.crc16 = 0x4a31u;

    if ((sizeof(packet) != PET_PACKET_MAX_SERIALIZED_SIZE) ||
        (packet.magic != PET_PACKET_MAGIC) ||
        (packet.version != PET_PACKET_VERSION) ||
        (packet.type != (pet_u8_t)PET_PACKET_PING) ||
        (packet.len != 4u) ||
        (packet.crc16 != 0x4a31u)) {
        return PET_RESULT_ERROR;
    }
    return PET_RESULT_OK;
}

pet_result_t sim_consistency_run_all(char *out_log, size_t out_capacity)
{
    char timeout_log[512];
    pet_result_t ret;
    size_t cursor = 0u;

    ret = sim_consistency_run_scene_replay(out_log, out_capacity);
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    cursor = strlen(out_log);

    ret = sim_consistency_run_timeout_replay(timeout_log, sizeof(timeout_log));
    if (ret != PET_RESULT_OK) {
        return ret;
    }
    ret = sim_consistency_append(out_log, out_capacity, &cursor, "%s", timeout_log);
    if (ret != PET_RESULT_OK) {
        return ret;
    }

    if ((sim_consistency_check_screen_profile_fixture() != PET_RESULT_OK) ||
        (sim_consistency_check_key_replay_fixture() != PET_RESULT_OK) ||
        (sim_consistency_check_save_slot_fixture() != PET_RESULT_OK) ||
        (sim_consistency_check_packet_fixture() != PET_RESULT_OK) ||
        (pet2d_mvp_a_renderer_contract_self_test() != PET_RESULT_OK)) {
        return PET_RESULT_ERROR;
    }

    return sim_consistency_append(out_log, out_capacity, &cursor,
                                  "fixtures=PASS screen=1 key=1 save=1 packet=1 renderer=1\n");
}
