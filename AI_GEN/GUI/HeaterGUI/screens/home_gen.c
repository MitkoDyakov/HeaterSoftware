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
        lv_style_set_bg_color(&main, BACKGRAUND);

        style_inited = true;
    }

    lv_obj_t * lv_obj_0 = lv_obj_create(NULL);
    lv_obj_set_width(lv_obj_0, 240);
    lv_obj_set_height(lv_obj_0, 135);
    lv_obj_add_style(lv_obj_0, &main, 0);

    lv_obj_t * row_0 = row_create(lv_obj_0);
    lv_obj_set_width(row_0, lv_pct(100));
    lv_obj_set_height(row_0, lv_pct(100));
    lv_obj_set_style_pad_all(row_0, 4, 0);

    lv_obj_t * column_0 = column_create(row_0);
    lv_obj_set_width(column_0, 89);
    lv_obj_set_height(column_0, lv_pct(100));
    lv_obj_set_style_bg_opa(column_0, 0, 0);
    lv_obj_set_style_pad_right(column_0, 2, 0);

    lv_obj_t * row_1 = row_create(column_0);
    lv_obj_set_width(row_1, 89);
    lv_obj_set_height(row_1, 49);

    lv_obj_t * channel_0 = channel_create(row_1, "CH1", "24°", ".9", 0, 0);
    lv_obj_set_style_pad_all(channel_0, 0, 0);



    lv_obj_t * row_2 = row_create(column_0);
    lv_obj_set_width(row_2, 89);
    lv_obj_set_height(row_2, 49);
    lv_obj_set_style_margin_top(row_2, 4, 0);

    lv_obj_t * channel_1 = channel_create(row_2, "CH2", "35°", ".7", 0, 0);
    lv_obj_set_style_pad_all(channel_1, 0, 0);



    lv_obj_t * row_3 = row_create(column_0);
    lv_obj_set_width(row_3, 89);
    lv_obj_set_height(row_3, 20);
    lv_obj_set_style_margin_top(row_3, 4, 0);

    lv_obj_t * page_0 = page_create(row_3, 0);
    lv_obj_set_style_pad_all(page_0, 0, 0);




    lv_obj_t * column_1 = column_create(row_0);
    lv_obj_set_width(column_1, 141);
    lv_obj_set_height(column_1, lv_pct(100));
    lv_obj_set_style_bg_opa(column_1, 0, 0);
    lv_obj_set_style_pad_left(column_1, 2, 0);

    lv_obj_t * row_4 = row_create(column_1);
    lv_obj_set_width(row_4, 141);
    lv_obj_set_height(row_4, 83);

    lv_obj_t * target_tmp_0 = target_tmp_create(row_4, "30°");
    lv_obj_set_style_pad_all(target_tmp_0, 0, 0);



    lv_obj_t * row_5 = row_create(column_1);
    lv_obj_set_width(row_5, 141);
    lv_obj_set_height(row_5, 39);
    lv_obj_set_style_margin_top(row_5, 4, 0);

    lv_obj_t * control_0 = control_create(row_5, "START", "00:00");
    lv_obj_set_style_pad_all(control_0, 0, 0);






    LV_TRACE_OBJ_CREATE("finished");

    lv_obj_set_name(lv_obj_0, "home");

    return lv_obj_0;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/