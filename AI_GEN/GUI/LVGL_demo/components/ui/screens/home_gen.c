/**
 * @file home_gen.c
 * @description Template source file for LVGL objects
 */

/*********************
 *      INCLUDES
 *********************/
#include "home_gen.h"
#include "ui.h"

/*********************
 *      DEFINES
 *********************/



/**********************
 *      TYPEDEFS
 **********************/

/***********************
 *  STATIC VARIABLES
 **********************/

/***********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

lv_obj_t * home_create(void)
{
    LV_TRACE_OBJ_CREATE("begin");

    static lv_style_t main;

    static bool style_inited = false;

    if (!style_inited) {
        lv_style_init(&main);
        lv_style_set_pad_all(&main, 0);
        lv_style_set_border_width(&main, 0);
        lv_style_set_radius(&main, 0);
        lv_style_set_layout(&main, LV_LAYOUT_FLEX);
        lv_style_set_flex_flow(&main, LV_FLEX_FLOW_COLUMN);
        lv_style_set_pad_row(&main, 0);

        style_inited = true;
    }

    lv_obj_t * lv_obj_0 = lv_obj_create(NULL);
    lv_obj_set_width(lv_obj_0, lv_pct(100));
    lv_obj_set_height(lv_obj_0, lv_pct(100));
    lv_obj_add_style(lv_obj_0, &main, 0);

    lv_obj_t * row_0 = row_create(lv_obj_0);
    lv_obj_set_flex_grow(row_0, 1);
    lv_obj_set_style_bg_color(row_0, lv_color_hex(0xf72121), 0);
    lv_obj_set_style_bg_opa(row_0, 255, 0);
    lv_obj_set_style_pad_all(row_0, 0, 0);
    lv_obj_set_width(row_0, lv_pct(100));
    lv_obj_set_flex_flow(row_0, LV_FLEX_FLOW_ROW_WRAP);

    lv_obj_t * column_0 = column_create(row_0);
    lv_obj_set_width(column_0, 25);
    lv_obj_set_height(column_0, lv_pct(100));
    lv_obj_set_style_bg_color(column_0, lv_color_hex(0x20f774), 0);
    lv_obj_set_style_pad_all(column_0, 0, 0);

    lv_obj_t * row_1 = row_create(column_0);
    lv_obj_set_flex_grow(row_1, 0);
    lv_obj_set_style_bg_color(row_1, lv_color_hex(0x20f774), 0);
    lv_obj_set_style_bg_opa(row_1, 255, 0);
    lv_obj_set_style_pad_all(row_1, 0, 0);
    lv_obj_set_width(row_1, 25);
    lv_obj_set_height(row_1, 25);
    lv_obj_set_flex_flow(row_1, LV_FLEX_FLOW_ROW_WRAP);

    // lv_obj_t * icon_0 = icon_create(row_1);
    // lv_image_set_src(icon_0, img_up);



    lv_obj_t * row_2 = row_create(column_0);
    lv_obj_set_flex_grow(row_2, 1);
    lv_obj_set_style_bg_color(row_2, lv_color_hex(0xf72121), 0);
    lv_obj_set_style_bg_opa(row_2, 255, 0);
    lv_obj_set_width(row_2, 25);

    lv_obj_t * lv_label_0 = lv_label_create(row_2);
    lv_obj_set_style_text_font(lv_label_0, font_status, 0);
    lv_obj_set_style_text_line_space(lv_label_0, -8, 0);
    lv_label_set_text(lv_label_0, lv_tr("ARM"));
    lv_obj_set_style_text_align(lv_label_0, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_margin_ver(lv_label_0, 13, 0);
    lv_obj_set_style_margin_left(lv_label_0, 5, 0);
    lv_label_set_long_mode(lv_label_0, LV_LABEL_LONG_WRAP);
    lv_obj_set_height(lv_label_0, 60);
    lv_obj_set_width(lv_label_0, 12);
    lv_obj_set_style_bg_opa(lv_label_0, 255, 0);
    lv_obj_set_style_bg_color(lv_label_0, lv_color_hex(0xffffff), 0);



    lv_obj_t * row_3 = row_create(column_0);
    lv_obj_set_flex_grow(row_3, 0);
    lv_obj_set_style_bg_color(row_3, lv_color_hex(0x20f774), 0);
    lv_obj_set_style_bg_opa(row_3, 255, 0);
    lv_obj_set_style_pad_all(row_3, 0, 0);
    lv_obj_set_width(row_3, 25);
    lv_obj_set_height(row_3, 25);
    lv_obj_set_flex_flow(row_3, LV_FLEX_FLOW_ROW_WRAP);

    // lv_obj_t * icon_1 = icon_create(row_3);
    // lv_image_set_src(icon_1, img_down);




    lv_obj_t * column_1 = column_create(row_0);
    lv_obj_set_width(column_1, 95);
    lv_obj_set_height(column_1, lv_pct(100));
    lv_obj_set_style_pad_all(column_1, 0, 0);
    lv_obj_set_style_bg_color(column_1, lv_color_hex(0xff8000), 0);

    lv_obj_t * row_4 = row_create(column_1);
    lv_obj_set_flex_grow(row_4, 0);
    lv_obj_set_style_bg_color(row_4, lv_color_hex(0xdee3e0), 0);
    lv_obj_set_style_bg_opa(row_4, 255, 0);
    lv_obj_set_style_pad_top(row_4, 0, 0);
    lv_obj_set_height(row_4, 25);
    lv_obj_set_width(row_4, 95);
    lv_obj_set_flex_flow(row_4, LV_FLEX_FLOW_ROW_WRAP);

    lv_obj_t * lv_label_1 = lv_label_create(row_4);
    lv_obj_set_style_text_font(lv_label_1, font_clock, 0);
    lv_label_set_text(lv_label_1, "00:00");
    lv_obj_set_width(lv_label_1, lv_pct(100));
    lv_obj_set_style_text_align(lv_label_1, LV_TEXT_ALIGN_CENTER, 0);



    lv_obj_t * row_5 = row_create(column_1);
    lv_obj_set_flex_grow(row_5, 1);
    lv_obj_set_style_bg_color(row_5, lv_color_hex(0x20f774), 0);
    lv_obj_set_style_bg_opa(row_5, 255, 0);
    lv_obj_set_style_pad_all(row_5, 0, 0);
    lv_obj_set_width(row_5, 95);
    lv_obj_set_flex_flow(row_5, LV_FLEX_FLOW_ROW_WRAP);

    lv_obj_t * column_2 = column_create(row_5);
    lv_obj_set_width(column_2, 20);
    lv_obj_set_height(column_2, lv_pct(100));
    lv_obj_set_style_pad_all(column_2, 0, 0);
    lv_obj_set_style_bg_color(column_2, lv_color_hex(0xff8000), 0);

    lv_obj_t * lv_label_2 = lv_label_create(column_2);
    lv_label_set_text(lv_label_2, "a");



    lv_obj_t * column_3 = column_create(row_5);
    lv_obj_set_width(column_3, 75);
    lv_obj_set_height(column_3, lv_pct(100));
    lv_obj_set_style_pad_all(column_3, 0, 0);
    lv_obj_set_style_bg_color(column_3, lv_color_hex(0x402509), 0);

    lv_obj_t * lv_label_3 = lv_label_create(column_3);
    lv_obj_set_style_text_font(lv_label_3, font_temp, 0);
    lv_obj_set_width(lv_label_3, lv_pct(100));
    lv_obj_set_style_text_align(lv_label_3, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(lv_label_3, "29°");




    lv_obj_t * row_6 = row_create(column_1);
    lv_obj_set_flex_grow(row_6, 1);
    lv_obj_set_style_bg_color(row_6, lv_color_hex(0xf9fffb), 0);
    lv_obj_set_style_bg_opa(row_6, 255, 0);
    lv_obj_set_style_pad_all(row_6, 0, 0);
    lv_obj_set_width(row_6, 95);
    lv_obj_set_flex_flow(row_6, LV_FLEX_FLOW_ROW_WRAP);

    lv_obj_t * column_4 = column_create(row_6);
    lv_obj_set_width(column_4, 20);
    lv_obj_set_height(column_4, lv_pct(100));
    lv_obj_set_style_pad_all(column_4, 0, 0);
    lv_obj_set_style_bg_color(column_4, lv_color_hex(0xff8000), 0);

    lv_obj_t * lv_label_4 = lv_label_create(column_4);
    lv_label_set_text(lv_label_4, "a");



    lv_obj_t * column_5 = column_create(row_6);
    lv_obj_set_width(column_5, 75);
    lv_obj_set_height(column_5, lv_pct(100));
    lv_obj_set_style_pad_all(column_5, 0, 0);
    lv_obj_set_style_bg_color(column_5, lv_color_hex(0x402509), 0);

    lv_obj_t * lv_label_5 = lv_label_create(column_5);
    lv_obj_set_style_text_font(lv_label_5, font_temp, 0);
    lv_obj_set_width(lv_label_5, lv_pct(100));
    lv_obj_set_style_text_align(lv_label_5, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(lv_label_5, "30°");





    lv_obj_t * column_6 = column_create(row_0);
    lv_obj_set_width(column_6, 95);
    lv_obj_set_height(column_6, lv_pct(100));
    lv_obj_set_style_pad_all(column_6, 0, 0);
    lv_obj_set_style_bg_color(column_6, lv_color_hex(0x20f774), 0);

    lv_obj_t * column_7 = column_create(column_6);
    lv_obj_set_width(column_7, 95);
    lv_obj_set_height(column_7, lv_pct(100));
    lv_obj_set_style_pad_all(column_7, 0, 0);
    lv_obj_set_style_bg_color(column_7, lv_color_hex(0xff8000), 0);

    lv_obj_t * row_7 = row_create(column_7);
    lv_obj_set_flex_grow(row_7, 0);
    lv_obj_set_style_bg_color(row_7, lv_color_hex(0x71d898), 0);
    lv_obj_set_style_bg_opa(row_7, 255, 0);
    lv_obj_set_style_pad_top(row_7, 0, 0);
    lv_obj_set_height(row_7, 25);
    lv_obj_set_width(row_7, 95);
    lv_obj_set_flex_flow(row_7, LV_FLEX_FLOW_ROW_WRAP);

    lv_obj_t * lv_label_6 = lv_label_create(row_7);
    lv_obj_set_style_text_align(lv_label_6, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(lv_label_6, font_clock, 0);
    lv_obj_set_width(lv_label_6, lv_pct(100));
    lv_label_set_text(lv_label_6, "31:00");



    lv_obj_t * row_8 = row_create(column_7);
    lv_obj_set_flex_grow(row_8, 1);
    lv_obj_set_style_bg_color(row_8, lv_color_hex(0x20f774), 0);
    lv_obj_set_style_bg_opa(row_8, 255, 0);
    lv_obj_set_style_pad_all(row_8, 0, 0);
    lv_obj_set_width(row_8, 95);
    lv_obj_set_flex_flow(row_8, LV_FLEX_FLOW_ROW_WRAP);

    lv_obj_t * column_8 = column_create(row_8);
    lv_obj_set_width(column_8, 20);
    lv_obj_set_height(column_8, lv_pct(100));
    lv_obj_set_style_pad_all(column_8, 0, 0);
    lv_obj_set_style_bg_color(column_8, lv_color_hex(0xff8000), 0);

    lv_obj_t * lv_label_7 = lv_label_create(column_8);
    lv_label_set_text(lv_label_7, "a");



    lv_obj_t * column_9 = column_create(row_8);
    lv_obj_set_width(column_9, 75);
    lv_obj_set_height(column_9, lv_pct(100));
    lv_obj_set_style_pad_all(column_9, 0, 0);
    lv_obj_set_style_bg_color(column_9, lv_color_hex(0xff8000), 0);

    lv_obj_t * lv_label_8 = lv_label_create(column_9);
    lv_obj_set_style_text_font(lv_label_8, font_temp, 0);
    lv_obj_set_width(lv_label_8, lv_pct(100));
    lv_obj_set_style_text_align(lv_label_8, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(lv_label_8, "29°");




    lv_obj_t * row_9 = row_create(column_7);
    lv_obj_set_flex_grow(row_9, 1);
    lv_obj_set_style_bg_color(row_9, lv_color_hex(0x20f774), 0);
    lv_obj_set_style_bg_opa(row_9, 255, 0);
    lv_obj_set_style_pad_all(row_9, 0, 0);
    lv_obj_set_width(row_9, 95);
    lv_obj_set_flex_flow(row_9, LV_FLEX_FLOW_ROW_WRAP);

    lv_obj_t * column_10 = column_create(row_9);
    lv_obj_set_width(column_10, 20);
    lv_obj_set_height(column_10, lv_pct(100));
    lv_obj_set_style_pad_all(column_10, 0, 0);
    lv_obj_set_style_bg_color(column_10, lv_color_hex(0xff8000), 0);

    lv_obj_t * lv_label_9 = lv_label_create(column_10);
    lv_label_set_text(lv_label_9, "a");



    lv_obj_t * column_11 = column_create(row_9);
    lv_obj_set_width(column_11, 75);
    lv_obj_set_height(column_11, lv_pct(100));
    lv_obj_set_style_pad_all(column_11, 0, 0);
    lv_obj_set_style_bg_color(column_11, lv_color_hex(0x402509), 0);

    lv_obj_t * lv_label_10 = lv_label_create(column_11);
    lv_obj_set_style_text_font(lv_label_10, font_temp, 0);
    lv_obj_set_width(lv_label_10, lv_pct(100));
    lv_obj_set_style_text_align(lv_label_10, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(lv_label_10, "30°");






    lv_obj_t * column_12 = column_create(row_0);
    lv_obj_set_width(column_12, 25);
    lv_obj_set_height(column_12, lv_pct(100));
    lv_obj_set_style_pad_all(column_12, 0, 0);

    lv_obj_t * row_10 = row_create(column_12);
    lv_obj_set_flex_grow(row_10, 0);
    lv_obj_set_style_bg_color(row_10, lv_color_hex(0x20f774), 0);
    lv_obj_set_style_bg_opa(row_10, 255, 0);
    lv_obj_set_style_pad_all(row_10, 0, 0);
    lv_obj_set_width(row_10, 25);
    lv_obj_set_height(row_10, 25);

    // lv_obj_t * icon_2 = icon_create(row_10);
    // lv_image_set_src(icon_2, img_up);



    lv_obj_t * row_11 = row_create(column_12);
    lv_obj_set_flex_grow(row_11, 1);
    lv_obj_set_style_bg_color(row_11, lv_color_hex(0xf72121), 0);
    lv_obj_set_style_bg_opa(row_11, 255, 0);
    lv_obj_set_width(row_11, 25);

    lv_obj_t * lv_label_11 = lv_label_create(row_11);
    lv_obj_set_style_text_font(lv_label_11, font_status, 0);
    lv_obj_set_style_text_line_space(lv_label_11, -8, 0);
    lv_label_set_text(lv_label_11, lv_tr("STOP"));
    lv_obj_set_style_text_align(lv_label_11, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_margin_ver(lv_label_11, 5, 0);
    lv_obj_set_style_margin_left(lv_label_11, 6, 0);
    lv_label_set_long_mode(lv_label_11, LV_LABEL_LONG_WRAP);
    lv_obj_set_height(lv_label_11, 75);
    lv_obj_set_width(lv_label_11, 13);
    lv_obj_set_style_bg_opa(lv_label_11, 255, 0);
    lv_obj_set_style_bg_color(lv_label_11, lv_color_hex(0xffffff), 0);



    lv_obj_t * row_12 = row_create(column_12);
    lv_obj_set_flex_grow(row_12, 0);
    lv_obj_set_style_bg_color(row_12, lv_color_hex(0x20f774), 0);
    lv_obj_set_style_bg_opa(row_12, 255, 0);
    lv_obj_set_style_pad_all(row_12, 0, 0);
    lv_obj_set_width(row_12, 25);
    lv_obj_set_height(row_12, 25);

    // lv_obj_t * icon_3 = icon_create(row_12);
    // lv_image_set_src(icon_3, img_down);






    LV_TRACE_OBJ_CREATE("finished");

    // lv_obj_set_name(lv_obj_0, "home");

    return lv_obj_0;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/