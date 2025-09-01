/**
 * @file control_gen.c
 * @description Template source file for LVGL objects
 */

/*********************
 *      INCLUDES
 *********************/
#include "control_gen.h"
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

lv_obj_t * control_create(lv_obj_t * parent, lv_subject_t * command, lv_subject_t * time)
{
    LV_TRACE_OBJ_CREATE("begin");

    static lv_style_t main;

    static bool style_inited = false;

    if (!style_inited) {
        lv_style_init(&main);
        lv_style_set_width(&main, 141);
        lv_style_set_height(&main, 39);
        lv_style_set_pad_all(&main, 0);
        lv_style_set_margin_all(&main, 0);
        lv_style_set_border_width(&main, 0);
        lv_style_set_text_font(&main, font_start_card);

        style_inited = true;
    }

    lv_obj_t * lv_obj_0 = lv_obj_create(parent);
    lv_obj_set_flag(lv_obj_0, LV_OBJ_FLAG_SCROLLABLE, false);
    lv_obj_add_style(lv_obj_0, &main, 0);

    lv_obj_t * lv_image_0 = lv_image_create(lv_obj_0);
    lv_image_set_src(lv_image_0, start_bg);


    lv_obj_t * lv_label_0 = lv_label_create(lv_obj_0);
    lv_obj_set_style_text_align(lv_label_0, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_bind_text(lv_label_0, command, NULL);lv_obj_set_x(lv_label_0, 70);
    lv_obj_set_width(lv_label_0, 65);
    lv_obj_set_height(lv_label_0, 40);


    lv_obj_t * lv_label_1 = lv_label_create(lv_obj_0);
    lv_label_bind_text(lv_label_1, time, NULL);lv_obj_set_x(lv_label_1, 5);
    lv_obj_set_y(lv_label_1, 0);



    LV_TRACE_OBJ_CREATE("finished");

    // lv_obj_set_name(lv_obj_0, "control_#");

    return lv_obj_0;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/