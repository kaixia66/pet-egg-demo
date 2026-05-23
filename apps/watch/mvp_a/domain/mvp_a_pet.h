#ifndef MVP_A_PET_H
#define MVP_A_PET_H

#include "mvp_a_def.h"

#define MVP_A_PET_FAST_GROWTH_DEFAULT  1

typedef struct {
    u8 first_wake_done;
    u8 wake_step;
    mvp_a_pet_stage_t stage;
    u8 mood;
    u8 clean;
    u8 energy;
    u8 growth_points;
    u8 fast_growth;
    u32 diary_flags;
} mvp_a_pet_snapshot_t;

void mvp_a_pet_init(void);
void mvp_a_pet_reload(void);
void mvp_a_pet_reset_demo(void);
mvp_a_result_t mvp_a_pet_first_wake_next(void);
mvp_a_result_t mvp_a_pet_add_growth(u8 points);
mvp_a_result_t mvp_a_pet_apply_care(mvp_a_care_action_t action);
mvp_a_bool_t mvp_a_pet_first_wake_done(void);
mvp_a_pet_stage_t mvp_a_pet_get_stage(void);
u8 mvp_a_pet_get_growth(void);
u8 mvp_a_pet_get_mood(void);
u8 mvp_a_pet_get_clean(void);
u8 mvp_a_pet_get_energy(void);
u32 mvp_a_pet_get_diary_flags(void);
mvp_a_result_t mvp_a_pet_mark_diary(u32 flag);
mvp_a_bool_t mvp_a_pet_fast_growth_enabled(void);
mvp_a_result_t mvp_a_pet_set_fast_growth(mvp_a_bool_t enable);
void mvp_a_pet_get_snapshot(mvp_a_pet_snapshot_t *snapshot);
const char *mvp_a_pet_get_stage_name(mvp_a_pet_stage_t stage);
const char *mvp_a_pet_get_wake_prompt(void);

#endif
