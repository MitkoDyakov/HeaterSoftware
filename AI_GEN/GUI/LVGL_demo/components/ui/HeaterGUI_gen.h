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

#define BACKGRAUND lv_color_hex(0x34404D)

#define SUBTEXT lv_color_hex(0x7F90A1)

#define MAINTEXT lv_color_hex(0xE7E1CD)

#define RED lv_color_hex(0x9F4949)

#define GREY lv_color_hex(0xD9D9D9)

#define YELLOW lv_color_hex(0xEC8140)

#define CARD_BG lv_color_hex(0x000000)

#define TIMER_TEXT lv_color_hex(0x1E2630)

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
extern lv_font_t * font_ch_label_temp_small;
extern lv_font_t * font_start_card;
extern lv_font_t * font_ch_temp_big;
extern lv_font_t * font_channel_dot;
extern lv_font_t * font_target_temp;

/*----------------
 * Images
 *----------------*/
extern const void * channel_bg;
extern const void * page_bg;
extern const void * start_bg;
extern const void * target_bg;

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
extern lv_subject_t ch1_active;
extern lv_subject_t ch1_power;
extern lv_subject_t ch1_temp_big;
extern lv_subject_t ch1_temp_small;
extern lv_subject_t ch2_active;
extern lv_subject_t ch2_power;
extern lv_subject_t ch2_temp_big;
extern lv_subject_t ch2_temp_small;
extern lv_subject_t pageSelect;
extern lv_subject_t targetTemp;
extern lv_subject_t opTime;
extern lv_subject_t command;

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
#include "components/channel/channel_gen.h"
#include "components/column/column_gen.h"
#include "components/control/control_gen.h"
#include "components/page/page_gen.h"
#include "components/row/row_gen.h"
#include "components/target_tmp/target_tmp_gen.h"
#include "screens/demo_gen.h"
#include "screens/home_gen.h"

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*HEATERGUI_GEN_H*/