#include "sim_consistency_golden.h"
#include "sim_consistency_replay.h"

pet_result_t sim_consistency_build_check_run(void)
{
    sim_consistency_scene_context_t context;
    sim_consistency_replay_step_t step;
    const char *golden;

    context.skipped_flush_count = 0u;
    context.pose_cycle = 0u;
    step.type = SIM_CONSISTENCY_STEP_ENTER;
    step.at_ms = 0u;
    step.key = PET_KEY_OK;

    golden = sim_consistency_golden_scene_replay_expected();
    if (golden == 0) {
        return PET_RESULT_ERROR;
    }

    return PET_RESULT_OK;
}
