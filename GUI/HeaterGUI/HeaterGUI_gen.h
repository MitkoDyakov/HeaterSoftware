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

/*----------------
 * Global styles
 *----------------*/

extern lv_style_t dot_base;
extern lv_style_t dot_red;

/*----------------
 * Fonts
 *----------------*/


/*----------------
 * Images
 *----------------*/

/*----------------
 * Subjects
 *----------------*/
extern lv_subject_t btn_left_1;
extern lv_subject_t btn_left_2;
extern lv_subject_t btn_left_3;
extern lv_subject_t btn_right_1;
extern lv_subject_t btn_right_2;
extern lv_subject_t btn_right_3;

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
#include "screens/demo_gen.h"

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*HEATERGUI_GEN_H*/