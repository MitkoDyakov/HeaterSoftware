/**
 * @file power_gen.c
 * @description Template source file for LVGL objects
 */

/*********************
 *      INCLUDES
 *********************/
#include "power_gen.h"
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

lv_obj_t * power_create(lv_obj_t * parent)
{
    LV_TRACE_OBJ_CREATE("begin");

    static lv_style_t main;
    static lv_style_t hiden;
    static lv_style_t visible;

    static bool style_inited = false;

    if (!style_inited) {
        lv_style_init(&main);
        lv_style_set_width(&main, 141);
        lv_style_set_height(&main, 126);
        lv_style_set_pad_all(&main, 0);
        lv_style_set_border_width(&main, 0);
        lv_style_set_text_font(&main, font_ch_label_temp_small);

        lv_style_init(&hiden);
        lv_style_set_height(&hiden, 0);

        lv_style_init(&visible);
        lv_style_set_text_opa(&visible, 255);

        style_inited = true;
    }

    lv_obj_t * lv_image_0 = lv_image_create(parent);
    lv_obj_set_flag(lv_image_0, LV_OBJ_FLAG_SCROLLABLE, false);
    lv_image_set_src(lv_image_0, info_bg);
    lv_image_set_inner_align(lv_image_0, LV_IMAGE_ALIGN_STRETCH);
    lv_obj_add_style(lv_image_0, &main, 0);

    lv_obj_t * column_0 = column_create(lv_image_0);
    lv_obj_set_width(column_0, lv_pct(100));
    lv_obj_set_height(column_0, lv_pct(100));
    lv_obj_set_style_bg_opa(column_0, 0, 0);

    lv_obj_t * row_0 = row_create(column_0);
    lv_obj_set_width(row_0, lv_pct(100));
    lv_obj_set_height(row_0, 24);
    lv_obj_set_style_margin_top(row_0, 3, 0);

    lv_obj_t * column_1 = column_create(row_0);
    lv_obj_set_height(column_1, lv_pct(100));
    lv_obj_set_style_pad_right(column_1, 0, 0);
    lv_obj_set_style_bg_opa(column_1, 0, 0);
    lv_obj_set_style_pad_left(column_1, 10, 0);

    lv_obj_t * lv_label_0 = lv_label_create(column_1);
    lv_label_set_text(lv_label_0, "Selected PDO");
    lv_obj_set_style_text_color(lv_label_0, SUBTEXT, 0);
    lv_obj_set_style_text_align(lv_label_0, LV_TEXT_ALIGN_CENTER, 0);




    lv_obj_t * row_1 = row_create(column_0);
    lv_obj_set_width(row_1, lv_pct(100));
    lv_obj_set_height(row_1, 102);

    lv_obj_t * column_2 = column_create(row_1);
    lv_obj_set_width(column_2, lv_pct(100));
    lv_obj_set_height(column_2, lv_pct(100));
    lv_obj_set_style_bg_opa(column_2, 0, 0);
    lv_obj_set_style_pad_left(column_2, 10, 0);

    lv_obj_t * row_2 = row_create(column_2);
    lv_obj_set_width(row_2, lv_pct(100));
    lv_obj_set_height(row_2, 24);
    lv_obj_add_style(row_2, &hiden, LV_STATE_CHECKED);
    lv_obj_bind_state_if_eq(row_2, &fiveV_available, LV_STATE_CHECKED, 0);

    lv_obj_t * column_3 = column_create(row_2);
    lv_obj_set_width(column_3, 24);
    lv_obj_set_height(column_3, lv_pct(100));
    lv_obj_set_style_pad_right(column_3, 0, 0);
    lv_obj_set_style_bg_opa(column_3, 0, 0);
    lv_obj_set_style_margin_top(column_3, -14, 0);

    lv_obj_t * lv_label_1 = lv_label_create(column_3);
    lv_label_set_text(lv_label_1, "•");
    lv_obj_set_style_text_font(lv_label_1, font_channel_dot, 0);
    lv_obj_set_style_text_color(lv_label_1, RED, 0);
    lv_obj_set_style_text_opa(lv_label_1, 0, 0);
    lv_obj_set_style_text_align(lv_label_1, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(lv_label_1, 24);
    lv_obj_add_style(lv_label_1, &visible, LV_STATE_CHECKED);
    lv_obj_bind_state_if_eq(lv_label_1, &activePDO, LV_STATE_CHECKED, 5);



    lv_obj_t * column_4 = column_create(row_2);
    lv_obj_set_width(column_4, 30);
    lv_obj_set_height(column_4, lv_pct(100));
    lv_obj_set_style_pad_right(column_4, 0, 0);
    lv_obj_set_style_bg_opa(column_4, 0, 0);

    lv_obj_t * lv_label_2 = lv_label_create(column_4);
    lv_label_set_text(lv_label_2, "5V");
    lv_obj_set_style_text_color(lv_label_2, SUBTEXT, 0);
    lv_obj_set_style_text_align(lv_label_2, LV_TEXT_ALIGN_CENTER, 0);



    lv_obj_t * column_5 = column_create(row_2);
    lv_obj_set_height(column_5, lv_pct(100));
    lv_obj_set_style_pad_right(column_5, 0, 0);
    lv_obj_set_style_bg_opa(column_5, 0, 0);

    lv_obj_t * lv_label_3 = lv_label_create(column_5);
    lv_label_bind_text(lv_label_3, &fiveV, "%.2fA");lv_obj_set_style_text_color(lv_label_3, MAINTEXT, 0);
    lv_obj_set_style_text_align(lv_label_3, LV_TEXT_ALIGN_CENTER, 0);




    lv_obj_t * row_3 = row_create(column_2);
    lv_obj_set_width(row_3, lv_pct(100));
    lv_obj_set_height(row_3, 24);
    lv_obj_add_style(row_3, &hiden, LV_STATE_CHECKED);
    lv_obj_bind_state_if_eq(row_3, &nineV_available, LV_STATE_CHECKED, 0);

    lv_obj_t * column_6 = column_create(row_3);
    lv_obj_set_width(column_6, 24);
    lv_obj_set_height(column_6, lv_pct(100));
    lv_obj_set_style_pad_right(column_6, 0, 0);
    lv_obj_set_style_bg_opa(column_6, 0, 0);
    lv_obj_set_style_margin_top(column_6, -14, 0);

    lv_obj_t * lv_label_4 = lv_label_create(column_6);
    lv_label_set_text(lv_label_4, "•");
    lv_obj_set_style_text_font(lv_label_4, font_channel_dot, 0);
    lv_obj_set_style_text_color(lv_label_4, RED, 0);
    lv_obj_set_style_text_opa(lv_label_4, 0, 0);
    lv_obj_set_style_text_align(lv_label_4, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(lv_label_4, 24);
    lv_obj_add_style(lv_label_4, &visible, LV_STATE_CHECKED);
    lv_obj_bind_state_if_eq(lv_label_4, &activePDO, LV_STATE_CHECKED, 9);



    lv_obj_t * column_7 = column_create(row_3);
    lv_obj_set_width(column_7, 30);
    lv_obj_set_height(column_7, lv_pct(100));
    lv_obj_set_style_pad_right(column_7, 0, 0);
    lv_obj_set_style_bg_opa(column_7, 0, 0);

    lv_obj_t * lv_label_5 = lv_label_create(column_7);
    lv_label_set_text(lv_label_5, "9V");
    lv_obj_set_style_text_color(lv_label_5, SUBTEXT, 0);
    lv_obj_set_style_text_align(lv_label_5, LV_TEXT_ALIGN_CENTER, 0);



    lv_obj_t * column_8 = column_create(row_3);
    lv_obj_set_width(column_8, 50);
    lv_obj_set_height(column_8, lv_pct(100));
    lv_obj_set_style_pad_right(column_8, 0, 0);
    lv_obj_set_style_bg_opa(column_8, 0, 0);

    lv_obj_t * lv_label_6 = lv_label_create(column_8);
    lv_label_set_text(lv_label_6, "3A");
    lv_obj_set_style_text_color(lv_label_6, MAINTEXT, 0);
    lv_obj_set_style_text_align(lv_label_6, LV_TEXT_ALIGN_CENTER, 0);




    lv_obj_t * row_4 = row_create(column_2);
    lv_obj_set_width(row_4, lv_pct(100));
    lv_obj_set_height(row_4, 24);
    lv_obj_add_style(row_4, &hiden, LV_STATE_CHECKED);
    lv_obj_bind_state_if_eq(row_4, &fifteenV_available, LV_STATE_CHECKED, 0);

    lv_obj_t * column_9 = column_create(row_4);
    lv_obj_set_width(column_9, 24);
    lv_obj_set_height(column_9, lv_pct(100));
    lv_obj_set_style_pad_right(column_9, 0, 0);
    lv_obj_set_style_bg_opa(column_9, 0, 0);
    lv_obj_set_style_margin_top(column_9, -14, 0);

    lv_obj_t * lv_label_7 = lv_label_create(column_9);
    lv_label_set_text(lv_label_7, "•");
    lv_obj_set_style_text_font(lv_label_7, font_channel_dot, 0);
    lv_obj_set_style_text_color(lv_label_7, RED, 0);
    lv_obj_set_style_text_opa(lv_label_7, 0, 0);
    lv_obj_set_style_text_align(lv_label_7, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(lv_label_7, 24);
    lv_obj_add_style(lv_label_7, &visible, LV_STATE_CHECKED);
    lv_obj_bind_state_if_eq(lv_label_7, &activePDO, LV_STATE_CHECKED, 15);



    lv_obj_t * column_10 = column_create(row_4);
    lv_obj_set_width(column_10, 30);
    lv_obj_set_height(column_10, lv_pct(100));
    lv_obj_set_style_pad_right(column_10, 0, 0);
    lv_obj_set_style_bg_opa(column_10, 0, 0);

    lv_obj_t * lv_label_8 = lv_label_create(column_10);
    lv_label_set_text(lv_label_8, "15V");
    lv_obj_set_style_text_color(lv_label_8, SUBTEXT, 0);
    lv_obj_set_style_text_align(lv_label_8, LV_TEXT_ALIGN_CENTER, 0);



    lv_obj_t * column_11 = column_create(row_4);
    lv_obj_set_width(column_11, 50);
    lv_obj_set_height(column_11, lv_pct(100));
    lv_obj_set_style_pad_right(column_11, 0, 0);
    lv_obj_set_style_bg_opa(column_11, 0, 0);

    lv_obj_t * lv_label_9 = lv_label_create(column_11);
    lv_label_set_text(lv_label_9, "2.5A");
    lv_obj_set_style_text_color(lv_label_9, MAINTEXT, 0);
    lv_obj_set_style_text_align(lv_label_9, LV_TEXT_ALIGN_CENTER, 0);




    lv_obj_t * row_5 = row_create(column_2);
    lv_obj_set_width(row_5, lv_pct(100));
    lv_obj_set_height(row_5, 24);
    lv_obj_add_style(row_5, &hiden, LV_STATE_CHECKED);
    lv_obj_bind_state_if_eq(row_5, &twentyV_available, LV_STATE_CHECKED, 0);

    lv_obj_t * column_12 = column_create(row_5);
    lv_obj_set_width(column_12, 24);
    lv_obj_set_height(column_12, lv_pct(100));
    lv_obj_set_style_pad_right(column_12, 0, 0);
    lv_obj_set_style_bg_opa(column_12, 0, 0);
    lv_obj_set_style_margin_top(column_12, -14, 0);

    lv_obj_t * lv_label_10 = lv_label_create(column_12);
    lv_label_set_text(lv_label_10, "•");
    lv_obj_set_style_text_font(lv_label_10, font_channel_dot, 0);
    lv_obj_set_style_text_color(lv_label_10, RED, 0);
    lv_obj_set_style_text_align(lv_label_10, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_opa(lv_label_10, 0, 0);
    lv_obj_set_width(lv_label_10, 24);
    lv_obj_add_style(lv_label_10, &visible, LV_STATE_CHECKED);
    lv_obj_bind_state_if_eq(lv_label_10, &activePDO, LV_STATE_CHECKED, 20);



    lv_obj_t * column_13 = column_create(row_5);
    lv_obj_set_width(column_13, 30);
    lv_obj_set_height(column_13, lv_pct(100));
    lv_obj_set_style_pad_right(column_13, 0, 0);
    lv_obj_set_style_bg_opa(column_13, 0, 0);

    lv_obj_t * lv_label_11 = lv_label_create(column_13);
    lv_label_set_text(lv_label_11, "20V");
    lv_obj_set_style_text_color(lv_label_11, SUBTEXT, 0);
    lv_obj_set_style_text_align(lv_label_11, LV_TEXT_ALIGN_CENTER, 0);



    lv_obj_t * column_14 = column_create(row_5);
    lv_obj_set_width(column_14, 50);
    lv_obj_set_height(column_14, lv_pct(100));
    lv_obj_set_style_pad_right(column_14, 0, 0);
    lv_obj_set_style_bg_opa(column_14, 0, 0);

    lv_obj_t * lv_label_12 = lv_label_create(column_14);
    lv_label_set_text(lv_label_12, "2.5A");
    lv_obj_set_style_text_color(lv_label_12, MAINTEXT, 0);
    lv_obj_set_style_text_align(lv_label_12, LV_TEXT_ALIGN_CENTER, 0);








    LV_TRACE_OBJ_CREATE("finished");

    lv_obj_set_name(lv_image_0, "power_#");

    return lv_image_0;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/