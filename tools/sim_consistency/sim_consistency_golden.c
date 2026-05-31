#include "sim_consistency_golden.h"

const char *sim_consistency_golden_scene_replay_expected(void)
{
    return
        "step=enter state=IDLE pose=IDLE pet=32,16 dirty=96x64 cmd_count=2 cmd0=STAGE_PATCH skipped=0 exit=NONE\n"
        "step=tick0 state=IDLE pose=IDLE pet=32,16 dirty=0x0 cmd_count=0 cmd0=NONE skipped=1 exit=NONE\n"
        "step=left state=MOVE_LEFT pose=STEP pet=24,16 dirty=40x32 cmd_count=2 cmd0=CLEAR_DIRTY skipped=1 exit=NONE\n"
        "step=left_done state=IDLE pose=IDLE pet=24,16 dirty=32x32 cmd_count=1 cmd0=PET_PLACEHOLDER skipped=1 exit=NONE\n"
        "step=right state=MOVE_RIGHT pose=STEP pet=32,16 dirty=40x32 cmd_count=2 cmd0=CLEAR_DIRTY skipped=1 exit=NONE\n"
        "step=ok state=ACTION pose=HAPPY pet=32,16 dirty=32x32 cmd_count=1 cmd0=PET_PLACEHOLDER skipped=1 exit=NONE\n"
        "step=ok_done state=IDLE pose=IDLE pet=32,16 dirty=32x32 cmd_count=1 cmd0=PET_PLACEHOLDER skipped=1 exit=NONE\n"
        "step=cancel state=DONE pose=IDLE pet=32,16 dirty=0x0 cmd_count=0 cmd0=NONE skipped=2 exit=CANCEL\n"
        "step=timeout_enter state=IDLE pose=IDLE pet=32,16 dirty=96x64 cmd_count=2 cmd0=STAGE_PATCH skipped=0 exit=NONE\n"
        "step=timeout state=DONE pose=IDLE pet=32,16 dirty=0x0 cmd_count=0 cmd0=NONE skipped=1 exit=TIMEOUT\n"
        "fixtures=PASS screen=1 key=1 save=1 packet=1 renderer=1\n";
}
