/**
 * @file target_tmp_gen.h
 */

#ifndef TARGET_TMP_H
#define TARGET_TMP_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "lvgl.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/


lv_obj_t * target_tmp_create(lv_obj_t * parent, const char * target_temp);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*TARGET_TMP_H*/