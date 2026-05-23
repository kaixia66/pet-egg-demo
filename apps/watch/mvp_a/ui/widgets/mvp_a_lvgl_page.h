#ifndef MVP_A_LVGL_PAGE_H
#define MVP_A_LVGL_PAGE_H

#include "app_config.h"
#include "lvgl.h"
#include "mvp_a_ui.h"

#if LVGL_TEST_ENABLE

typedef struct {
    lv_obj_t *root;
    lv_obj_t *background_img;
    lv_obj_t *title;
    lv_obj_t *visual;
    lv_obj_t *visual_img;
    lv_obj_t *prompt;
    lv_obj_t *detail;
    lv_obj_t *button;
    lv_obj_t *button_label;
} mvp_a_lvgl_page_ctx_t;

void mvp_a_lvgl_page_create(lv_obj_t *parent,
                            const mvp_a_ui_page_t *page,
                            const char *detail,
                            mvp_a_lvgl_page_ctx_t *ctx);
void mvp_a_lvgl_page_set_visual_text(mvp_a_lvgl_page_ctx_t *ctx,
                                     const char *primary,
                                     const char *secondary);
void mvp_a_lvgl_page_set_background_image(mvp_a_lvgl_page_ctx_t *ctx,
                                          const void *src);
void mvp_a_lvgl_page_set_visual_image(mvp_a_lvgl_page_ctx_t *ctx,
                                      const void *src);
void mvp_a_lvgl_page_set_detail(mvp_a_lvgl_page_ctx_t *ctx, const char *detail);
lv_color_t mvp_a_lvgl_page_color(u16 rgb565);

#endif

#endif
