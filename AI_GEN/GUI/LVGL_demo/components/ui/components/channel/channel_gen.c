/**
 * @file channel_gen.c
 * @description Template source file for LVGL objects
 */

/*********************
 *      INCLUDES
 *********************/
#include "channel_gen.h"
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

lv_obj_t * channel_create(lv_obj_t * parent, const char * channel, lv_subject_t * temp_big, lv_subject_t * temp_small, lv_subject_t * ch_active)
{
    LV_TRACE_OBJ_CREATE("begin");

    static lv_style_t main;

    static bool style_inited = false;

    if (!style_inited) {
        lv_style_init(&main);
        lv_style_set_width(&main, 87);
        lv_style_set_height(&main, 49);
        lv_style_set_pad_all(&main, 0);
        lv_style_set_margin_all(&main, 0);
        lv_style_set_border_width(&main, 0);

        style_inited = true;
    }

    lv_obj_t * lv_image_0 = lv_image_create(parent);
    lv_obj_set_flag(lv_image_0, LV_OBJ_FLAG_SCROLLABLE, false);
    lv_image_set_src(lv_image_0, channel_bg);
    lv_image_set_inner_align(lv_image_0, LV_IMAGE_ALIGN_STRETCH);
    lv_obj_add_style(lv_image_0, &main, 0);

    lv_obj_t * lv_label_0 = lv_label_create(lv_image_0);
    lv_label_bind_text(lv_label_0, temp_big, "%i°");lv_obj_set_style_text_font(lv_label_0, font_ch_temp_big, 0);
    lv_obj_set_style_text_align(lv_label_0, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_style_text_color(lv_label_0, MAINTEXT, 0);
    lv_obj_set_x(lv_label_0, 26);
    lv_obj_set_y(lv_label_0, -2);
    lv_obj_set_width(lv_label_0, 54);
    lv_obj_set_height(lv_label_0, 54);


    lv_obj_t * lv_label_1 = lv_label_create(lv_image_0);
    lv_label_set_text(lv_label_1, channel);
    lv_obj_set_style_text_font(lv_label_1, font_ch_label_temp_small, 0);
    lv_obj_set_style_text_color(lv_label_1, SUBTEXT, 0);
    lv_obj_set_x(lv_label_1, 6);
    lv_obj_set_y(lv_label_1, 4);


    lv_obj_t * lv_label_2 = lv_label_create(lv_image_0);
    lv_label_bind_text(lv_label_2, temp_small, ".%i");lv_obj_set_style_text_font(lv_label_2, font_ch_label_temp_small, 0);
    lv_obj_set_style_text_color(lv_label_2, MAINTEXT, 0);
    lv_obj_set_x(lv_label_2, 69);
    lv_obj_set_y(lv_label_2, 20);


    lv_obj_t * lv_label_3 = lv_label_create(lv_image_0);
    lv_label_bind_text(lv_label_3, ch_active, NULL);lv_obj_set_style_text_font(lv_label_3, font_channel_dot, 0);
    lv_obj_set_style_text_color(lv_label_3, RED, 0);
    lv_obj_set_x(lv_label_3, 9);
    lv_obj_set_y(lv_label_3, 9);



    LV_TRACE_OBJ_CREATE("finished");

    lv_obj_set_name(lv_image_0, "channel_#");

    return lv_image_0;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/