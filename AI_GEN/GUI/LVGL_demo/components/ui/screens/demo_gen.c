/**
 * @file demo_gen.c
 * @description Template source file for LVGL objects
 */

/*********************
 *      INCLUDES
 *********************/
#include "demo_gen.h"
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

lv_obj_t * demo_create(void)
{
    LV_TRACE_OBJ_CREATE("begin");


    static bool style_inited = false;

    if (!style_inited) {

        style_inited = true;
    }

    lv_obj_t * lv_obj_0 = lv_obj_create(NULL);
    lv_obj_set_width(lv_obj_0, 240);
    lv_obj_set_height(lv_obj_0, 135);

    lv_obj_t * lv_obj_1 = lv_obj_create(lv_obj_0);
    lv_obj_set_align(lv_obj_1, LV_ALIGN_CENTER);
    lv_obj_add_style(lv_obj_1, &dot_base, 0);
    lv_obj_add_style(lv_obj_1, &dot_red, LV_STATE_CHECKED);
    lv_obj_bind_state_if_eq(lv_obj_1, &btn_center, LV_STATE_CHECKED, 1);


    lv_obj_t * lv_obj_2 = lv_obj_create(lv_obj_0);
    lv_obj_set_align(lv_obj_2, LV_ALIGN_LEFT_MID);
    lv_obj_set_x(lv_obj_2, 10);
    lv_obj_set_y(lv_obj_2, -45);
    lv_obj_add_style(lv_obj_2, &dot_base, 0);
    lv_obj_add_style(lv_obj_2, &dot_red, LV_STATE_CHECKED);
    lv_obj_bind_state_if_eq(lv_obj_2, &btn_left_1, LV_STATE_CHECKED, 1);


    lv_obj_t * lv_obj_3 = lv_obj_create(lv_obj_0);
    lv_obj_set_align(lv_obj_3, LV_ALIGN_LEFT_MID);
    lv_obj_set_x(lv_obj_3, 10);
    lv_obj_set_y(lv_obj_3, 0);
    lv_obj_add_style(lv_obj_3, &dot_base, 0);
    lv_obj_add_style(lv_obj_3, &dot_red, LV_STATE_CHECKED);
    lv_obj_bind_state_if_eq(lv_obj_3, &btn_left_2, LV_STATE_CHECKED, 1);


    lv_obj_t * lv_obj_4 = lv_obj_create(lv_obj_0);
    lv_obj_set_align(lv_obj_4, LV_ALIGN_LEFT_MID);
    lv_obj_set_x(lv_obj_4, 10);
    lv_obj_set_y(lv_obj_4, 45);
    lv_obj_add_style(lv_obj_4, &dot_base, 0);
    lv_obj_add_style(lv_obj_4, &dot_red, LV_STATE_CHECKED);
    lv_obj_bind_state_if_eq(lv_obj_4, &btn_left_3, LV_STATE_CHECKED, 1);


    lv_obj_t * lv_obj_5 = lv_obj_create(lv_obj_0);
    lv_obj_set_align(lv_obj_5, LV_ALIGN_RIGHT_MID);
    lv_obj_set_x(lv_obj_5, -10);
    lv_obj_set_y(lv_obj_5, -45);
    lv_obj_add_style(lv_obj_5, &dot_base, 0);
    lv_obj_add_style(lv_obj_5, &dot_red, LV_STATE_CHECKED);
    lv_obj_bind_state_if_eq(lv_obj_5, &btn_right_1, LV_STATE_CHECKED, 1);


    lv_obj_t * lv_obj_6 = lv_obj_create(lv_obj_0);
    lv_obj_set_align(lv_obj_6, LV_ALIGN_RIGHT_MID);
    lv_obj_set_x(lv_obj_6, -10);
    lv_obj_set_y(lv_obj_6, 0);
    lv_obj_add_style(lv_obj_6, &dot_base, 0);
    lv_obj_add_style(lv_obj_6, &dot_red, LV_STATE_CHECKED);
    lv_obj_bind_state_if_eq(lv_obj_6, &btn_right_2, LV_STATE_CHECKED, 1);


    lv_obj_t * lv_obj_7 = lv_obj_create(lv_obj_0);
    lv_obj_set_align(lv_obj_7, LV_ALIGN_RIGHT_MID);
    lv_obj_set_x(lv_obj_7, -10);
    lv_obj_set_y(lv_obj_7, 45);
    lv_obj_add_style(lv_obj_7, &dot_base, 0);
    lv_obj_add_style(lv_obj_7, &dot_red, LV_STATE_CHECKED);
    lv_obj_bind_state_if_eq(lv_obj_7, &btn_right_3, LV_STATE_CHECKED, 1);



    LV_TRACE_OBJ_CREATE("finished");

    return lv_obj_0;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/