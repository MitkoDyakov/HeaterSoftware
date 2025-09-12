/**
 * @file info_gen.c
 * @description Template source file for LVGL objects
 */

/*********************
 *      INCLUDES
 *********************/
#include "info_gen.h"
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

lv_obj_t * info_create(lv_obj_t * parent)
{
    LV_TRACE_OBJ_CREATE("begin");

    static lv_style_t main;

    static bool style_inited = false;

    if (!style_inited) {
        lv_style_init(&main);
        lv_style_set_bg_opa(&main, 0);
        lv_style_set_width(&main, 141);
        lv_style_set_height(&main, 126);
        lv_style_set_pad_all(&main, 0);
        lv_style_set_border_width(&main, 0);
        lv_style_set_text_font(&main, font_ch_label_temp_small);

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
    lv_obj_set_style_pad_all(column_0, 5, 0);

    lv_obj_t * row_0 = row_create(column_0);
    lv_obj_set_width(row_0, lv_pct(100));
    lv_obj_set_height(row_0, 24);

    lv_obj_t * column_1 = column_create(row_0);
    lv_obj_set_height(column_1, lv_pct(100));
    lv_obj_set_style_pad_right(column_1, 0, 0);
    lv_obj_set_style_bg_opa(column_1, 0, 0);

    lv_obj_t * lv_label_0 = lv_label_create(column_1);
    lv_label_set_text(lv_label_0, "SOFTWARE:");
    lv_obj_set_style_text_color(lv_label_0, SUBTEXT, 0);
    lv_obj_set_style_text_align(lv_label_0, LV_TEXT_ALIGN_CENTER, 0);



    lv_obj_t * column_2 = column_create(row_0);
    lv_obj_set_height(column_2, lv_pct(100));
    lv_obj_set_style_pad_right(column_2, 0, 0);
    lv_obj_set_style_bg_opa(column_2, 0, 0);
    lv_obj_set_style_pad_left(column_2, 5, 0);

    lv_obj_t * lv_label_1 = lv_label_create(column_2);
    lv_label_set_text(lv_label_1, "0.1v");
    lv_obj_set_style_text_color(lv_label_1, MAINTEXT, 0);
    lv_obj_set_style_text_align(lv_label_1, LV_TEXT_ALIGN_LEFT, 0);




    lv_obj_t * row_1 = row_create(column_0);
    lv_obj_set_width(row_1, lv_pct(100));
    lv_obj_set_height(row_1, 24);

    lv_obj_t * column_3 = column_create(row_1);
    lv_obj_set_height(column_3, lv_pct(100));
    lv_obj_set_style_pad_right(column_3, 0, 0);
    lv_obj_set_style_bg_opa(column_3, 0, 0);

    lv_obj_t * lv_label_2 = lv_label_create(column_3);
    lv_label_set_text(lv_label_2, "HARDWARE:");
    lv_obj_set_style_text_color(lv_label_2, SUBTEXT, 0);
    lv_obj_set_style_text_align(lv_label_2, LV_TEXT_ALIGN_CENTER, 0);



    lv_obj_t * column_4 = column_create(row_1);
    lv_obj_set_height(column_4, lv_pct(100));
    lv_obj_set_style_pad_right(column_4, 0, 0);
    lv_obj_set_style_bg_opa(column_4, 0, 0);
    lv_obj_set_style_pad_left(column_4, 5, 0);

    lv_obj_t * lv_label_3 = lv_label_create(column_4);
    lv_label_set_text(lv_label_3, "2.0v");
    lv_obj_set_style_text_color(lv_label_3, MAINTEXT, 0);
    lv_obj_set_style_text_align(lv_label_3, LV_TEXT_ALIGN_LEFT, 0);




    lv_obj_t * row_2 = row_create(column_0);
    lv_obj_set_width(row_2, lv_pct(100));
    lv_obj_set_height(row_2, 24);

    lv_obj_t * column_5 = column_create(row_2);
    lv_obj_set_height(column_5, lv_pct(100));
    lv_obj_set_style_pad_right(column_5, 0, 0);
    lv_obj_set_style_bg_opa(column_5, 0, 0);

    lv_obj_t * lv_label_4 = lv_label_create(column_5);
    lv_label_set_text(lv_label_4, "SN:");
    lv_obj_set_style_text_color(lv_label_4, SUBTEXT, 0);
    lv_obj_set_style_text_align(lv_label_4, LV_TEXT_ALIGN_CENTER, 0);



    lv_obj_t * column_6 = column_create(row_2);
    lv_obj_set_height(column_6, lv_pct(100));
    lv_obj_set_style_pad_right(column_6, 0, 0);
    lv_obj_set_style_bg_opa(column_6, 0, 0);
    lv_obj_set_style_pad_left(column_6, 5, 0);

    lv_obj_t * lv_label_5 = lv_label_create(column_6);
    lv_label_set_text(lv_label_5, "T001");
    lv_obj_set_style_text_color(lv_label_5, MAINTEXT, 0);
    lv_obj_set_style_text_align(lv_label_5, LV_TEXT_ALIGN_LEFT, 0);




    lv_obj_t * row_3 = row_create(column_0);
    lv_obj_set_width(row_3, lv_pct(100));
    lv_obj_set_height(row_3, 24);

    lv_obj_t * column_7 = column_create(row_3);
    lv_obj_set_height(column_7, lv_pct(100));
    lv_obj_set_style_pad_right(column_7, 0, 0);
    lv_obj_set_style_bg_opa(column_7, 0, 0);

    lv_obj_t * lv_label_6 = lv_label_create(column_7);
    lv_label_set_text(lv_label_6, "RUN-TIME:");
    lv_obj_set_style_text_color(lv_label_6, SUBTEXT, 0);
    lv_obj_set_style_text_align(lv_label_6, LV_TEXT_ALIGN_CENTER, 0);



    lv_obj_t * column_8 = column_create(row_3);
    lv_obj_set_height(column_8, lv_pct(100));
    lv_obj_set_style_pad_right(column_8, 0, 0);
    lv_obj_set_style_bg_opa(column_8, 0, 0);
    lv_obj_set_style_pad_left(column_8, 5, 0);

    lv_obj_t * lv_label_7 = lv_label_create(column_8);
    lv_label_set_text(lv_label_7, "1h");
    lv_obj_set_style_text_color(lv_label_7, MAINTEXT, 0);
    lv_obj_set_style_text_align(lv_label_7, LV_TEXT_ALIGN_LEFT, 0);






    LV_TRACE_OBJ_CREATE("finished");

    lv_obj_set_name(lv_image_0, "info_#");

    return lv_image_0;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/