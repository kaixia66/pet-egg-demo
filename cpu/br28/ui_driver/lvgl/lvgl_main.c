/*--------------------------------------------------------------------------*/
/**@file    UI_common.c
   @brief   UI 显示公共函数
   @details
   @author  bingquan Cai
   @date    2012-8-30
   @note    AC319N
*/
/*----------------------------------------------------------------------------*/
#include "app_config.h"
#include "includes.h"
/* #include "ui/ui_api.h" */
/* #include "ui/ui.h" */
#include "typedef.h"
#include "clock_cfg.h"
#include "app_task.h"
#include "key_event_deal.h"

#include "lvgl.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"

#include "lv_demo_conf.h"
#include "mvp_a_lvgl_shell.h"

#if LVGL_TEST_ENABLE

#define UI_TASK_NAME 	"ui"

#if LV_USE_LOG && LV_LOG_PRINTF
static void lv_rt_log(const char *buf)
{
    printf(buf);
}
#endif

static void lvgl_test_tick(void *param)
{
    lv_tick_inc(2);
}

static void lvgl_task(void *p)
{
    int msg[32];
    int ret;

#if LV_USE_LOG && LV_LOG_PRINTF
    lv_log_register_print_cb(lv_rt_log);
#endif

    lv_init();

    lv_port_disp_init(p);
    lv_port_indev_init();

    sys_s_hi_timer_add(NULL, lvgl_test_tick, 2);

    mvp_a_lvgl_shell_create();

    while (1) {
        ret = os_taskq_accept(ARRAY_SIZE(msg), msg);
        if (ret == OS_TASKQ) {
            // msg deal
        }
        mvp_a_lvgl_shell_tick();
        lv_timer_handler();
        os_time_dly(1);
    }
}

int lvgl_test_init(void *param)
{
    int err = 0;
    clock_add_set(DEC_UI_CLK);
    err = task_create(lvgl_task, param, UI_TASK_NAME);
    if (err) {
        r_printf("ui task create err:%d \n", err);
    }
    return err;
}

#endif /* #if LVGL_TEST_ENABLE */
