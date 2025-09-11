/**
 * @file page_gen.c
 * @description Template source file for LVGL objects
 */

/*********************
 *      INCLUDES
 *********************/
#include "page_gen.h"
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

lv_obj_t * page_create(lv_obj_t * parent, int32_t active_page)
{
    LV_TRACE_OBJ_CREATE("begin");

    static lv_style_t main;
    static lv_style_t selected;

    static bool style_inited = false;

    if (!style_inited) {
        lv_style_init(&main);
        lv_style_set_width(&main, 87);
        lv_style_set_height(&main, 20);
        lv_style_set_pad_all(&main, 0);
        lv_style_set_border_width(&main, 0);
        lv_style_set_text_opa(&main, 120);
        lv_style_set_text_font(&main, font_start_card);
        lv_style_set_text_color(&main, GREY);

        lv_style_init(&selected);
        lv_style_set_text_opa(&selected, 255);

        style_inited = true;
    }

    lv_obj_t * lv_obj_0 = lv_obj_create(parent);
    lv_obj_set_flag(lv_obj_0, LV_OBJ_FLAG_SCROLLABLE, false);

    lv_obj_t * lv_image_0 = lv_image_create(lv_obj_0);
    lv_image_set_src(lv_image_0, page_bg);
    lv_obj_set_style_pad_all(lv_image_0, 0, 0);

    lv_obj_add_style(lv_obj_0, &main, 0);

    lv_obj_t * lv_label_0 = lv_label_create(lv_obj_0);
    lv_label_set_text(lv_label_0, "•");
    lv_obj_set_x(lv_label_0, 24);
    lv_obj_set_y(lv_label_0, -8);
    lv_obj_add_style(lv_label_0, &selected, LV_STATE_CHECKED);
    lv_obj_bind_state_if_eq(lv_label_0, &pageSelect, LV_STATE_CHECKED, 0);


    lv_obj_t * lv_label_1 = lv_label_create(lv_obj_0);
    lv_label_set_text(lv_label_1, "•");
    lv_obj_set_x(lv_label_1, 34);
    lv_obj_set_y(lv_label_1, -8);
    lv_obj_add_style(lv_label_1, &selected, LV_STATE_CHECKED);
    lv_obj_bind_state_if_eq(lv_label_1, &pageSelect, LV_STATE_CHECKED, 1);


    lv_obj_t * lv_label_2 = lv_label_create(lv_obj_0);
    lv_label_set_text(lv_label_2, "•");
    lv_obj_set_x(lv_label_2, 44);
    lv_obj_set_y(lv_label_2, -8);
    lv_obj_add_style(lv_label_2, &selected, LV_STATE_CHECKED);
    lv_obj_bind_state_if_eq(lv_label_2, &pageSelect, LV_STATE_CHECKED, 2);


    lv_obj_t * lv_label_3 = lv_label_create(lv_obj_0);
    lv_label_set_text(lv_label_3, "•");
    lv_obj_set_x(lv_label_3, 54);
    lv_obj_set_y(lv_label_3, -8);
    lv_obj_add_style(lv_label_3, &selected, LV_STATE_CHECKED);
    lv_obj_bind_state_if_eq(lv_label_3, &pageSelect, LV_STATE_CHECKED, 3);



    LV_TRACE_OBJ_CREATE("finished");

    lv_obj_set_name(lv_obj_0, "page_#");

    return lv_obj_0;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/