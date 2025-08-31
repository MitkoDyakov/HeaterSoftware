/**
 * @file target_tmp_gen.c
 * @description Template source file for LVGL objects
 */

/*********************
 *      INCLUDES
 *********************/
#include "target_tmp_gen.h"
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

lv_obj_t * target_tmp_create(lv_obj_t * parent, const char * target_temp)
{
    LV_TRACE_OBJ_CREATE("begin");

    static lv_style_t main;

    static bool style_inited = false;

    if (!style_inited) {
        lv_style_init(&main);
        lv_style_set_width(&main, 141);
        lv_style_set_height(&main, 83);
        lv_style_set_pad_all(&main, 0);
        lv_style_set_border_width(&main, 0);

        style_inited = true;
    }

    lv_obj_t * lv_obj_0 = lv_obj_create(parent);
    lv_obj_set_flag(lv_obj_0, LV_OBJ_FLAG_SCROLLABLE, false);
    lv_obj_add_style(lv_obj_0, &main, 0);

    lv_obj_t * lv_image_0 = lv_image_create(lv_obj_0);
    lv_image_set_src(lv_image_0, target_bg);
    lv_obj_set_style_pad_all(lv_image_0, 0, 0);


    lv_obj_t * lv_label_0 = lv_label_create(lv_obj_0);
    lv_obj_set_style_text_font(lv_label_0, font_ch_label_temp_small, 0);
    lv_label_set_text(lv_label_0, lv_tr("TARGET TEMP"));
    lv_obj_set_style_text_color(lv_label_0, SUBTEXT, 0);
    lv_obj_set_style_text_align(lv_label_0, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_x(lv_label_0, 7);
    lv_obj_set_y(lv_label_0, 17);
    lv_obj_set_width(lv_label_0, 52);
    lv_obj_set_height(lv_label_0, 51);


    lv_obj_t * lv_label_1 = lv_label_create(lv_obj_0);
    lv_obj_set_style_text_font(lv_label_1, font_target_temp, 0);
    lv_label_set_text(lv_label_1, target_temp);
    lv_obj_set_style_text_color(lv_label_1, MAINTEXT, 0);
    lv_obj_set_x(lv_label_1, 62);
    lv_obj_set_y(lv_label_1, 5);



    LV_TRACE_OBJ_CREATE("finished");

    //lv_obj_set_name(lv_obj_0, "target_tmp_#");

    return lv_obj_0;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/