/**
 * @file settings_gen.c
 * @description Template source file for LVGL objects
 */

/*********************
 *      INCLUDES
 *********************/
#include "settings_gen.h"
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

lv_obj_t * settings_create(lv_obj_t * parent)
{
    LV_TRACE_OBJ_CREATE("begin");

    static lv_style_t main;
    static lv_style_t selected;

    static bool style_inited = false;

    if (!style_inited) {
        lv_style_init(&main);
        lv_style_set_bg_opa(&main, 0);
        lv_style_set_width(&main, 141);
        lv_style_set_height(&main, 126);
        lv_style_set_pad_all(&main, 0);
        lv_style_set_border_width(&main, 0);
        lv_style_set_text_font(&main, font_ch_label_temp_small);

        lv_style_init(&selected);
        lv_style_set_text_color(&selected, RED);

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
    lv_obj_set_style_pad_top(column_0, 3, 0);

    lv_obj_t * row_0 = row_create(column_0);
    lv_obj_set_width(row_0, lv_pct(100));
    lv_obj_set_height(row_0, 24);

    lv_obj_t * column_1 = column_create(row_0);
    lv_obj_set_width(column_1, 24);
    lv_obj_set_height(column_1, lv_pct(100));
    lv_obj_set_style_pad_right(column_1, 0, 0);
    lv_obj_set_style_bg_opa(column_1, 0, 0);
    lv_obj_set_style_margin_top(column_1, -14, 0);

    lv_obj_t * lv_label_0 = lv_label_create(column_1);
    lv_label_set_text(lv_label_0, "•");
    lv_obj_set_style_text_font(lv_label_0, font_channel_dot, 0);
    lv_obj_set_style_text_color(lv_label_0, SUBTEXT, 0);
    lv_obj_set_style_text_align(lv_label_0, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(lv_label_0, 24);
    lv_obj_add_style(lv_label_0, &selected, LV_STATE_CHECKED);
    lv_obj_bind_state_if_eq(lv_label_0, &settingsSelect, LV_STATE_CHECKED, 0);



    lv_obj_t * column_2 = column_create(row_0);
    lv_obj_set_width(column_2, 67);
    lv_obj_set_height(column_2, lv_pct(100));
    lv_obj_set_style_pad_right(column_2, 0, 0);
    lv_obj_set_style_bg_opa(column_2, 0, 0);

    lv_obj_t * lv_label_1 = lv_label_create(column_2);
    lv_label_set_text(lv_label_1, "CH1:");
    lv_obj_set_style_text_color(lv_label_1, SUBTEXT, 0);
    lv_obj_set_style_text_align(lv_label_1, LV_TEXT_ALIGN_CENTER, 0);



    lv_obj_t * column_3 = column_create(row_0);
    lv_obj_set_width(column_3, 50);
    lv_obj_set_height(column_3, lv_pct(100));
    lv_obj_set_style_pad_right(column_3, 0, 0);
    lv_obj_set_style_bg_opa(column_3, 0, 0);

    lv_obj_t * lv_label_2 = lv_label_create(column_3);
    lv_label_bind_text(lv_label_2, &ch1_active, NULL);lv_obj_set_style_text_color(lv_label_2, MAINTEXT, 0);
    lv_obj_set_style_text_align(lv_label_2, LV_TEXT_ALIGN_CENTER, 0);




    lv_obj_t * row_1 = row_create(column_0);
    lv_obj_set_width(row_1, lv_pct(100));
    lv_obj_set_height(row_1, 24);

    lv_obj_t * column_4 = column_create(row_1);
    lv_obj_set_width(column_4, 24);
    lv_obj_set_height(column_4, lv_pct(100));
    lv_obj_set_style_pad_right(column_4, 0, 0);
    lv_obj_set_style_bg_opa(column_4, 0, 0);
    lv_obj_set_style_margin_top(column_4, -14, 0);

    lv_obj_t * lv_label_3 = lv_label_create(column_4);
    lv_label_set_text(lv_label_3, "•");
    lv_obj_set_style_text_font(lv_label_3, font_channel_dot, 0);
    lv_obj_set_style_text_color(lv_label_3, SUBTEXT, 0);
    lv_obj_set_style_text_align(lv_label_3, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(lv_label_3, 24);
    lv_obj_add_style(lv_label_3, &selected, LV_STATE_CHECKED);
    lv_obj_bind_state_if_eq(lv_label_3, &settingsSelect, LV_STATE_CHECKED, 1);



    lv_obj_t * column_5 = column_create(row_1);
    lv_obj_set_width(column_5, 67);
    lv_obj_set_height(column_5, lv_pct(100));
    lv_obj_set_style_pad_right(column_5, 0, 0);
    lv_obj_set_style_bg_opa(column_5, 0, 0);

    lv_obj_t * lv_label_4 = lv_label_create(column_5);
    lv_label_set_text(lv_label_4, "CH2:");
    lv_obj_set_style_text_color(lv_label_4, SUBTEXT, 0);
    lv_obj_set_style_text_align(lv_label_4, LV_TEXT_ALIGN_CENTER, 0);



    lv_obj_t * column_6 = column_create(row_1);
    lv_obj_set_width(column_6, 50);
    lv_obj_set_height(column_6, lv_pct(100));
    lv_obj_set_style_pad_right(column_6, 0, 0);
    lv_obj_set_style_bg_opa(column_6, 0, 0);

    lv_obj_t * lv_label_5 = lv_label_create(column_6);
    lv_label_bind_text(lv_label_5, &ch2_active, NULL);lv_obj_set_style_text_color(lv_label_5, MAINTEXT, 0);
    lv_obj_set_style_text_align(lv_label_5, LV_TEXT_ALIGN_CENTER, 0);




    lv_obj_t * row_2 = row_create(column_0);
    lv_obj_set_width(row_2, lv_pct(100));
    lv_obj_set_height(row_2, 24);

    lv_obj_t * column_7 = column_create(row_2);
    lv_obj_set_width(column_7, 24);
    lv_obj_set_height(column_7, lv_pct(100));
    lv_obj_set_style_pad_right(column_7, 0, 0);
    lv_obj_set_style_bg_opa(column_7, 0, 0);
    lv_obj_set_style_margin_top(column_7, -14, 0);

    lv_obj_t * lv_label_6 = lv_label_create(column_7);
    lv_label_set_text(lv_label_6, "•");
    lv_obj_set_style_text_font(lv_label_6, font_channel_dot, 0);
    lv_obj_set_style_text_color(lv_label_6, SUBTEXT, 0);
    lv_obj_set_style_text_align(lv_label_6, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(lv_label_6, 24);
    lv_obj_add_style(lv_label_6, &selected, LV_STATE_CHECKED);
    lv_obj_bind_state_if_eq(lv_label_6, &settingsSelect, LV_STATE_CHECKED, 2);



    lv_obj_t * column_8 = column_create(row_2);
    lv_obj_set_width(column_8, 67);
    lv_obj_set_height(column_8, lv_pct(100));
    lv_obj_set_style_pad_right(column_8, 0, 0);
    lv_obj_set_style_bg_opa(column_8, 0, 0);

    lv_obj_t * lv_label_7 = lv_label_create(column_8);
    lv_label_set_text(lv_label_7, "SOUND:");
    lv_obj_set_style_text_color(lv_label_7, SUBTEXT, 0);
    lv_obj_set_style_text_align(lv_label_7, LV_TEXT_ALIGN_CENTER, 0);



    lv_obj_t * column_9 = column_create(row_2);
    lv_obj_set_width(column_9, 50);
    lv_obj_set_height(column_9, lv_pct(100));
    lv_obj_set_style_pad_right(column_9, 0, 0);
    lv_obj_set_style_bg_opa(column_9, 0, 0);

    lv_obj_t * lv_label_8 = lv_label_create(column_9);
    lv_label_bind_text(lv_label_8, &soundEnable, NULL);lv_obj_set_style_text_color(lv_label_8, MAINTEXT, 0);
    lv_obj_set_style_text_align(lv_label_8, LV_TEXT_ALIGN_CENTER, 0);




    lv_obj_t * row_3 = row_create(column_0);
    lv_obj_set_width(row_3, lv_pct(100));
    lv_obj_set_height(row_3, 24);

    lv_obj_t * column_10 = column_create(row_3);
    lv_obj_set_width(column_10, 24);
    lv_obj_set_height(column_10, lv_pct(100));
    lv_obj_set_style_pad_right(column_10, 0, 0);
    lv_obj_set_style_bg_opa(column_10, 0, 0);
    lv_obj_set_style_margin_top(column_10, -14, 0);

    lv_obj_t * lv_label_9 = lv_label_create(column_10);
    lv_label_set_text(lv_label_9, "•");
    lv_obj_set_style_text_font(lv_label_9, font_channel_dot, 0);
    lv_obj_set_style_text_color(lv_label_9, SUBTEXT, 0);
    lv_obj_set_style_text_align(lv_label_9, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(lv_label_9, 24);
    lv_obj_add_style(lv_label_9, &selected, LV_STATE_CHECKED);
    lv_obj_bind_state_if_eq(lv_label_9, &settingsSelect, LV_STATE_CHECKED, 3);



    lv_obj_t * column_11 = column_create(row_3);
    lv_obj_set_width(column_11, 67);
    lv_obj_set_height(column_11, lv_pct(100));
    lv_obj_set_style_pad_right(column_11, 0, 0);
    lv_obj_set_style_bg_opa(column_11, 0, 0);

    lv_obj_t * lv_label_10 = lv_label_create(column_11);
    lv_label_set_text(lv_label_10, "DISPLAY:");
    lv_obj_set_style_text_color(lv_label_10, SUBTEXT, 0);
    lv_obj_set_style_text_align(lv_label_10, LV_TEXT_ALIGN_CENTER, 0);



    lv_obj_t * column_12 = column_create(row_3);
    lv_obj_set_width(column_12, 50);
    lv_obj_set_height(column_12, lv_pct(100));
    lv_obj_set_style_pad_right(column_12, 0, 0);
    lv_obj_set_style_bg_opa(column_12, 0, 0);

    lv_obj_t * lv_label_11 = lv_label_create(column_12);
    lv_label_bind_text(lv_label_11, &brightness, "%d%%");lv_obj_set_style_text_color(lv_label_11, MAINTEXT, 0);
    lv_obj_set_style_text_align(lv_label_11, LV_TEXT_ALIGN_CENTER, 0);




    lv_obj_t * row_4 = row_create(column_0);
    lv_obj_set_width(row_4, lv_pct(100));
    lv_obj_set_height(row_4, 24);

    lv_obj_t * column_13 = column_create(row_4);
    lv_obj_set_width(column_13, 24);
    lv_obj_set_height(column_13, lv_pct(100));
    lv_obj_set_style_pad_right(column_13, 0, 0);
    lv_obj_set_style_bg_opa(column_13, 0, 0);
    lv_obj_set_style_margin_top(column_13, -14, 0);

    lv_obj_t * lv_label_12 = lv_label_create(column_13);
    lv_label_set_text(lv_label_12, "•");
    lv_obj_set_style_text_font(lv_label_12, font_channel_dot, 0);
    lv_obj_set_style_text_color(lv_label_12, SUBTEXT, 0);
    lv_obj_set_style_text_align(lv_label_12, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(lv_label_12, 24);
    lv_obj_add_style(lv_label_12, &selected, LV_STATE_CHECKED);
    lv_obj_bind_state_if_eq(lv_label_12, &settingsSelect, LV_STATE_CHECKED, 4);



    lv_obj_t * column_14 = column_create(row_4);
    lv_obj_set_width(column_14, 67);
    lv_obj_set_height(column_14, lv_pct(100));
    lv_obj_set_style_pad_right(column_14, 0, 0);
    lv_obj_set_style_bg_opa(column_14, 0, 0);

    lv_obj_t * lv_label_13 = lv_label_create(column_14);
    lv_label_set_text(lv_label_13, "SLEEP:");
    lv_obj_set_style_text_color(lv_label_13, SUBTEXT, 0);
    lv_obj_set_style_text_align(lv_label_13, LV_TEXT_ALIGN_CENTER, 0);



    lv_obj_t * column_15 = column_create(row_4);
    lv_obj_set_width(column_15, 50);
    lv_obj_set_height(column_15, lv_pct(100));
    lv_obj_set_style_pad_right(column_15, 0, 0);
    lv_obj_set_style_bg_opa(column_15, 0, 0);

    lv_obj_t * lv_label_14 = lv_label_create(column_15);
    lv_label_bind_text(lv_label_14, &sleepTimer, "%ds");lv_obj_set_style_text_color(lv_label_14, MAINTEXT, 0);
    lv_obj_set_style_text_align(lv_label_14, LV_TEXT_ALIGN_CENTER, 0);






    LV_TRACE_OBJ_CREATE("finished");

    lv_obj_set_name(lv_image_0, "settings_#");

    return lv_image_0;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/