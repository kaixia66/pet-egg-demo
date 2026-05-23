#include "mvp_a_lvgl_page.h"

#if LVGL_TEST_ENABLE

#define MVP_A_LVGL_SCREEN_W 454

static lv_obj_t *mvp_a_lvgl_label_create(lv_obj_t *parent,
                                         const char *text,
                                         lv_coord_t width,
                                         lv_color_t color)
{
    lv_obj_t *label = lv_label_create(parent);

    lv_label_set_text(label, text ? text : "");
    lv_label_set_long_mode(label, LV_LABEL_LONG_DOT);
    lv_obj_set_width(label, width);
    lv_obj_set_style_text_color(label, color, 0);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    return label;
}

lv_color_t mvp_a_lvgl_page_color(u16 rgb565)
{
    u8 r = (u8)(((rgb565 >> 11) & 0x1f) << 3);
    u8 g = (u8)(((rgb565 >> 5) & 0x3f) << 2);
    u8 b = (u8)((rgb565 & 0x1f) << 3);

    r |= (u8)(r >> 5);
    g |= (u8)(g >> 6);
    b |= (u8)(b >> 5);
    return lv_color_make(r, g, b);
}

void mvp_a_lvgl_page_create(lv_obj_t *parent,
                            const mvp_a_ui_page_t *page,
                            const char *detail,
                            mvp_a_lvgl_page_ctx_t *ctx)
{
    lv_color_t bg;
    lv_color_t accent;
    lv_obj_t *ring;

    if (!parent || !page || !ctx) {
        return;
    }

    (void)detail;
    memset(ctx, 0, sizeof(*ctx));
    bg = mvp_a_lvgl_page_color(page->bg_color);
    accent = mvp_a_lvgl_page_color(page->accent_color);

    ctx->root = lv_obj_create(parent);
    lv_obj_remove_style_all(ctx->root);
    lv_obj_set_size(ctx->root, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(ctx->root, bg, 0);
    lv_obj_set_style_bg_opa(ctx->root, LV_OPA_COVER, 0);
    lv_obj_clear_flag(ctx->root, LV_OBJ_FLAG_SCROLLABLE);

    ctx->background_img = lv_img_create(ctx->root);
    lv_obj_clear_flag(ctx->background_img, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(ctx->background_img, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_center(ctx->background_img);

    ring = lv_obj_create(ctx->root);
    lv_obj_set_size(ring, 416, 416);
    lv_obj_set_style_radius(ring, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(ring, bg, 0);
    lv_obj_set_style_bg_opa(ring, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(ring, 6, 0);
    lv_obj_set_style_border_color(ring, accent, 0);
    lv_obj_set_style_pad_all(ring, 0, 0);
    lv_obj_clear_flag(ring, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(ring, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_center(ring);

    ctx->title = mvp_a_lvgl_label_create(ctx->root, page->title,
                                         MVP_A_LVGL_SCREEN_W - 140,
                                         lv_color_hex(0xffffff));
    lv_obj_set_style_text_font(ctx->title, &lv_font_montserrat_16, 0);
    lv_obj_align(ctx->title, LV_ALIGN_TOP_MID, 0, 34);

    ctx->visual = lv_obj_create(ctx->root);
    lv_obj_set_size(ctx->visual, 166, 166);
    lv_obj_set_style_radius(ctx->visual, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(ctx->visual, lv_color_hex(0xf5fbff), 0);
    lv_obj_set_style_bg_opa(ctx->visual, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(ctx->visual, accent, 0);
    lv_obj_set_style_border_width(ctx->visual, 5, 0);
    lv_obj_set_style_pad_all(ctx->visual, 0, 0);
    lv_obj_clear_flag(ctx->visual, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_align(ctx->visual, LV_ALIGN_CENTER, 0, -34);

    ctx->prompt = mvp_a_lvgl_label_create(ctx->root, page->prompt,
                                          MVP_A_LVGL_SCREEN_W - 132,
                                          lv_color_hex(0xffffff));
    lv_obj_align(ctx->prompt, LV_ALIGN_CENTER, 0, 94);

    ctx->button = lv_obj_create(ctx->root);
    lv_obj_set_size(ctx->button, 154, 50);
    lv_obj_set_style_radius(ctx->button, 25, 0);
    lv_obj_set_style_bg_color(ctx->button, accent, 0);
    lv_obj_set_style_bg_opa(ctx->button, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(ctx->button, 0, 0);
    lv_obj_set_style_pad_all(ctx->button, 0, 0);
    lv_obj_clear_flag(ctx->button, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(ctx->button, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_align(ctx->button, LV_ALIGN_BOTTOM_MID, 0, -38);

    ctx->button_label = mvp_a_lvgl_label_create(ctx->button, page->action, 140,
                                                lv_color_hex(0x102028));
    lv_obj_center(ctx->button_label);
}

void mvp_a_lvgl_page_set_visual_text(mvp_a_lvgl_page_ctx_t *ctx,
                                     const char *primary,
                                     const char *secondary)
{
    lv_obj_t *label;

    if (!ctx || !ctx->visual) {
        return;
    }

    lv_obj_clean(ctx->visual);

    label = mvp_a_lvgl_label_create(ctx->visual, primary, 132, lv_color_hex(0x102028));
    lv_obj_set_style_text_font(label, &lv_font_montserrat_16, 0);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, -14);

    label = mvp_a_lvgl_label_create(ctx->visual, secondary, 128, lv_color_hex(0x496070));
    lv_obj_set_style_text_font(label, &lv_font_montserrat_12, 0);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 18);
}

void mvp_a_lvgl_page_set_background_image(mvp_a_lvgl_page_ctx_t *ctx,
                                          const void *src)
{
    if (!ctx || !ctx->background_img || !src) {
        return;
    }

    lv_img_set_src(ctx->background_img, src);
    lv_obj_center(ctx->background_img);
}

void mvp_a_lvgl_page_set_visual_image(mvp_a_lvgl_page_ctx_t *ctx,
                                      const void *src)
{
    if (!ctx || !ctx->visual || !src) {
        return;
    }

    lv_obj_clean(ctx->visual);
    ctx->visual_img = lv_img_create(ctx->visual);
    lv_img_set_src(ctx->visual_img, src);
    lv_obj_center(ctx->visual_img);
}

void mvp_a_lvgl_page_set_detail(mvp_a_lvgl_page_ctx_t *ctx, const char *detail)
{
    (void)ctx;
    (void)detail;
}

#endif
