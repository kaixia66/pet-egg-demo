#include "mvp_a_ui_draw.h"
#include "mvp_a_ui.h"
#include "ui/ui.h"

static void mvp_a_draw_text(void *draw_ctx, const char *text, int x, int y, u16 color)
{
    ui_draw_ascii((struct draw_context *)draw_ctx, (char *)text, strlen(text), x, y, color);
}

static void mvp_a_draw_bottom_bubble(void *draw_ctx, const char *action, u16 color)
{
    ui_draw_ring(draw_ctx, 227, 360, 76, 70, 0, 360, color, 100);
    ui_draw_ring(draw_ctx, 227, 360, 70, 0, 0, 360, RGB565(255, 255, 255), 100);
    mvp_a_draw_text(draw_ctx, action, 190, 352, RGB565(30, 48, 42));
}

static void mvp_a_draw_scene_icon(void *draw_ctx, mvp_a_scene_t scene, u16 color)
{
    switch (scene) {
    case MVP_A_SCENE_BOOT:
        ui_draw_ring(draw_ctx, 227, 210, 82, 0, 0, 360, RGB565(255, 244, 204), 100);
        ui_draw_ring(draw_ctx, 227, 210, 84, 80, 0, 360, color, 100);
        ui_draw_line(draw_ctx, 205, 180, 247, 240, color);
        break;
    case MVP_A_SCENE_HOME:
        ui_draw_ring(draw_ctx, 227, 210, 92, 0, 0, 360, RGB565(244, 238, 205), 100);
        ui_draw_ring(draw_ctx, 188, 196, 10, 0, 0, 360, RGB565(30, 50, 44), 100);
        ui_draw_ring(draw_ctx, 266, 196, 10, 0, 0, 360, RGB565(30, 50, 44), 100);
        ui_draw_line(draw_ctx, 200, 238, 254, 238, RGB565(30, 50, 44));
        break;
    case MVP_A_SCENE_CARE:
        ui_draw_ring(draw_ctx, 227, 205, 82, 78, 0, 360, color, 100);
        ui_draw_ring(draw_ctx, 227, 205, 42, 0, 0, 360, RGB565(190, 240, 210), 100);
        ui_draw_line(draw_ctx, 185, 250, 269, 250, color);
        break;
    case MVP_A_SCENE_TRAINING:
        ui_draw_line(draw_ctx, 175, 260, 275, 160, color);
        ui_draw_line(draw_ctx, 275, 160, 275, 220, color);
        ui_draw_line(draw_ctx, 275, 160, 215, 160, color);
        ui_draw_ring(draw_ctx, 205, 235, 38, 34, 0, 360, RGB565(255, 255, 255), 100);
        break;
    case MVP_A_SCENE_CARD_BAG:
        ui_fill_rect((struct draw_context *)draw_ctx, 165, 150, 124, 92, color);
        ui_fill_rect((struct draw_context *)draw_ctx, 177, 164, 100, 68, RGB565(255, 255, 255));
        ui_draw_line(draw_ctx, 185, 190, 268, 190, color);
        break;
    case MVP_A_SCENE_NFC_READ:
        ui_draw_ring(draw_ctx, 227, 190, 68, 62, 0, 360, color, 100);
        ui_draw_ring(draw_ctx, 227, 190, 42, 36, 0, 360, RGB565(255, 255, 255), 100);
        ui_draw_line(draw_ctx, 227, 258, 227, 298, color);
        break;
    case MVP_A_SCENE_COOP_WAIT:
        ui_draw_ring(draw_ctx, 185, 210, 46, 0, 0, 360, RGB565(255, 244, 204), 100);
        ui_draw_ring(draw_ctx, 269, 210, 46, 0, 0, 360, RGB565(255, 244, 204), 100);
        ui_draw_line(draw_ctx, 215, 210, 239, 210, color);
        break;
    case MVP_A_SCENE_BOSS:
        ui_draw_ring(draw_ctx, 227, 198, 86, 0, 0, 360, RGB565(120, 110, 96), 100);
        ui_draw_ring(draw_ctx, 198, 185, 9, 0, 0, 360, RGB565(0, 0, 0), 100);
        ui_draw_ring(draw_ctx, 256, 185, 9, 0, 0, 360, RGB565(0, 0, 0), 100);
        ui_draw_line(draw_ctx, 190, 235, 264, 235, color);
        break;
    case MVP_A_SCENE_DIARY:
        ui_fill_rect((struct draw_context *)draw_ctx, 165, 145, 124, 120, RGB565(255, 244, 204));
        ui_draw_line(draw_ctx, 185, 178, 268, 178, color);
        ui_draw_line(draw_ctx, 185, 210, 268, 210, color);
        ui_draw_line(draw_ctx, 185, 242, 240, 242, color);
        break;
    case MVP_A_SCENE_DEBUG:
        ui_draw_ring(draw_ctx, 227, 205, 70, 64, 0, 360, color, 100);
        ui_draw_line(draw_ctx, 177, 205, 277, 205, RGB565(255, 255, 255));
        ui_draw_line(draw_ctx, 227, 155, 227, 255, RGB565(255, 255, 255));
        break;
    default:
        break;
    }
}

void mvp_a_ui_draw_page(void *draw_ctx, const struct mvp_a_ui_page *page)
{
    const mvp_a_ui_page_t *p = (const mvp_a_ui_page_t *)page;

    if (!draw_ctx || !p) {
        return;
    }

    ui_custom_draw_clear((struct draw_context *)draw_ctx);
    ui_fill_rect((struct draw_context *)draw_ctx, 0, 0, 454, 454, p->bg_color);
    ui_draw_ring(draw_ctx, 227, 227, 214, 210, 0, 360, RGB565(86, 124, 118), 100);
    ui_draw_ring(draw_ctx, 227, 210, 118, 114, 0, 360, p->accent_color, 100);

    mvp_a_draw_scene_icon(draw_ctx, p->scene, p->accent_color);
    mvp_a_draw_text(draw_ctx, p->title, 190, 48, RGB565(255, 255, 255));
    mvp_a_draw_text(draw_ctx, p->prompt, 176, 302, RGB565(255, 255, 255));
    mvp_a_draw_bottom_bubble(draw_ctx, p->action, p->accent_color);
}
