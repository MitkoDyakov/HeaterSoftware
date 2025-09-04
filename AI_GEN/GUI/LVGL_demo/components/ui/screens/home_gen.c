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
lv_obj_t * lv_tabview_0;
lv_obj_t * home_create(void)
{
    LV_TRACE_OBJ_CREATE("begin");

    static lv_style_t main;
    static lv_style_t nopad;

    static bool style_inited = false;

    if (!style_inited) {
        lv_style_init(&main);
        lv_style_set_bg_color(&main, BACKGRAUND);

        lv_style_init(&nopad);
        lv_style_set_margin_all(&nopad, 0);
        lv_style_set_pad_all(&nopad, 0);

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

    lv_obj_t * channel_0 = channel_create(row_1, "CH1", &ch1_temp_big, &ch1_temp_small, &ch1_active);
    lv_obj_set_style_pad_all(channel_0, 0, 0);



    lv_obj_t * row_2 = row_create(column_0);
    lv_obj_set_width(row_2, 89);
    lv_obj_set_height(row_2, 49);
    lv_obj_set_style_margin_top(row_2, 4, 0);

    lv_obj_t * channel_1 = channel_create(row_2, "CH2", &ch2_temp_big, &ch2_temp_small, &ch2_active);
    lv_obj_set_style_pad_all(channel_1, 0, 0);



    lv_obj_t * row_3 = row_create(column_0);
    lv_obj_set_width(row_3, 89);
    lv_obj_set_height(row_3, 20);
    lv_obj_set_style_margin_top(row_3, 4, 0);

    lv_obj_t * page_0 = page_create(row_3, 0);
    lv_obj_set_style_pad_all(page_0, 0, 0);




    lv_obj_t * column_1 = column_create(row_0);
    lv_obj_set_width(column_1, 141);
    lv_obj_set_height(column_1, 126);
    lv_obj_set_style_bg_opa(column_1, 0, 0);
    lv_obj_set_style_margin_all(column_1, 0, 0);
    lv_obj_set_style_pad_left(column_1, 2, 0);

    lv_tabview_0 = lv_tabview_create(column_1);
    lv_tabview_set_tab_bar_position(lv_tabview_0, LV_DIR_BOTTOM);
    lv_obj_set_width(lv_tabview_0, 141);
    lv_obj_set_height(lv_tabview_0, 126);
    lv_obj_set_flag(lv_tabview_0, LV_OBJ_FLAG_SCROLLABLE, false);
    lv_obj_set_style_bg_opa(lv_tabview_0, 0, 0);
    lv_obj_t * lv_tabview_tab_bar_0 = lv_tabview_get_tab_bar(lv_tabview_0);
    lv_obj_set_height(lv_tabview_tab_bar_0, 0);
    lv_obj_t * lv_tabview_tab_0 = lv_tabview_add_tab(lv_tabview_0, "1");
    lv_obj_set_flag(lv_tabview_0, LV_OBJ_FLAG_SCROLLABLE, false);
        lv_obj_add_style(lv_tabview_tab_0, &nopad, 0);

        lv_obj_t * column_2 = column_create(lv_tabview_tab_0);
        lv_obj_set_flag(column_2, LV_OBJ_FLAG_SCROLLABLE, false);
        lv_obj_set_width(column_2, 141);
        lv_obj_set_height(column_2, 126);
        lv_obj_set_style_bg_opa(column_2, 0, 0);

        lv_obj_t * row_4 = row_create(column_2);
        lv_obj_set_width(row_4, 141);
        lv_obj_set_height(row_4, 83);

        lv_obj_t * target_tmp_0 = target_tmp_create(row_4, &targetTemp);



        lv_obj_t * row_5 = row_create(column_2);
        lv_obj_set_width(row_5, 141);
        lv_obj_set_height(row_5, 39);
        lv_obj_set_style_margin_top(row_5, 4, 0);

        lv_obj_t * control_0 = control_create(row_5, &command, &opTime);



    lv_obj_t * lv_tabview_tab_1 = lv_tabview_add_tab(lv_tabview_0, "2");
    lv_obj_set_flag(lv_tabview_0, LV_OBJ_FLAG_SCROLLABLE, false);
        lv_obj_add_style(lv_tabview_tab_1, &nopad, 0);

        lv_obj_t * info_0 = info_create(lv_tabview_tab_1, "Page 2");

    lv_obj_t * lv_tabview_tab_2 = lv_tabview_add_tab(lv_tabview_0, "3");
    lv_obj_set_flag(lv_tabview_0, LV_OBJ_FLAG_SCROLLABLE, false);
        lv_obj_add_style(lv_tabview_tab_2, &nopad, 0);

        lv_obj_t * info_1 = info_create(lv_tabview_tab_2, "Page 3");

    lv_obj_t * lv_tabview_tab_3 = lv_tabview_add_tab(lv_tabview_0, "4");
    lv_obj_set_flag(lv_tabview_0, LV_OBJ_FLAG_SCROLLABLE, false);
        lv_obj_add_style(lv_tabview_tab_3, &nopad, 0);

        lv_obj_t * info_2 = info_create(lv_tabview_tab_3, "Page 4");






    LV_TRACE_OBJ_CREATE("finished");

    // lv_obj_set_name(lv_obj_0, "home");

    return lv_obj_0;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/