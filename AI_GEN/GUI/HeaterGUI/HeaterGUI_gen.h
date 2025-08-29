/**
 * @file HeaterGUI_gen.h
 */

#ifndef HEATERGUI_GEN_H
#define HEATERGUI_GEN_H

#ifndef UI_SUBJECT_STRING_LENGTH
#define UI_SUBJECT_STRING_LENGTH 256
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
    #include "lvgl.h"
#else
    #include "lvgl/lvgl.h"
#endif

/*********************
 *      DEFINES
 *********************/



/**********************
 *      TYPEDEFS
 **********************/



/**********************
 * GLOBAL VARIABLES
 **********************/

/*-------------------
 * Permanent screens
 *------------------*/
extern lv_obj_t * home;

/*----------------
 * Global styles
 *----------------*/

extern lv_style_t dot_base;
extern lv_style_t dot_red;

/*----------------
 * Fonts
 *----------------*/
extern lv_font_t * font_status;
extern lv_font_t * font_clock;
extern lv_font_t * font_temp;

/*----------------
 * Images
 *----------------*/
extern const void * img_down;
extern const void * img_up;

/*----------------
 * Subjects
 *----------------*/
extern lv_subject_t btn_left_1;
extern lv_subject_t btn_left_2;
extern lv_subject_t btn_left_3;
extern lv_subject_t btn_right_1;
extern lv_subject_t btn_right_2;
extern lv_subject_t btn_right_3;
extern lv_subject_t btn_center;
extern lv_subject_t chan1_armed;
extern lv_subject_t chan2_armed;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/*----------------
 * Event Callbacks
 *----------------*/

/**
 * Initialize the component library
 */

void HeaterGUI_init_gen(const char * asset_path);

/**********************
 *      MACROS
 **********************/

/**********************
 *   POST INCLUDES
 **********************/

/*Include all the widget and components of this library*/
#include "components/column/column_gen.h"
#include "components/icon/icon_gen.h"
#include "components/row/row_gen.h"
#include "screens/demo_gen.h"
#include "screens/home_gen.h"

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*HEATERGUI_GEN_H*/