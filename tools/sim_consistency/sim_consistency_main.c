#include <stdio.h>
#include <string.h>

#include "sim_consistency_golden.h"
#include "sim_consistency_replay.h"

int main(void)
{
    char log[SIM_CONSISTENCY_REPLAY_LOG_CAP];
    pet_result_t ret;

    ret = sim_consistency_run_all(log, sizeof(log));
    if (ret != PET_RESULT_OK) {
        printf("P25 sim consistency FAIL ret=%d\n", ret);
        return 1;
    }

    printf("%s", log);
    if (strcmp(log, sim_consistency_golden_scene_replay_expected()) != 0) {
        printf("P25 sim consistency FAIL golden_mismatch=1\n");
        return 2;
    }

    printf("P25 sim consistency PASS\n");
    return 0;
}
