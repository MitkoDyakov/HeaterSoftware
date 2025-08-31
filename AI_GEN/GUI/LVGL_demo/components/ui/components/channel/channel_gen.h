/**
 * @file channel_gen.h
 */

#ifndef CHANNEL_H
#define CHANNEL_H

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


lv_obj_t * channel_create(lv_obj_t * parent, const char * channel, const char * temp_big, const char * temp_small, int32_t active, int32_t power);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*CHANNEL_H*/