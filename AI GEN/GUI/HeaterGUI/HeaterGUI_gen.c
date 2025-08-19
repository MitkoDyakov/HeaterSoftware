/**
 * @file HeaterGUI_gen.c
 */

/*********************
 *      INCLUDES
 *********************/
#include "HeaterGUI_gen.h"

#if LV_USE_XML
#endif

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/*----------------
 * Translations
 *----------------*/

/**********************
 *  GLOBAL VARIABLES
 **********************/

/*--------------------
 *  Permanent screens
 *-------------------*/

/*----------------
 * Global styles
 *----------------*/
lv_style_t dot_base;
lv_style_t dot_red;

/*----------------
 * Fonts
 *----------------*/


/*----------------
 * Images
 *----------------*/

/*----------------
 * Subjects
 *----------------*/
lv_subject_t btn_left_1;
lv_subject_t btn_left_2;
lv_subject_t btn_left_3;
lv_subject_t btn_right_1;
lv_subject_t btn_right_2;
lv_subject_t btn_right_3;
lv_subject_t btn_center;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void HeaterGUI_init_gen(const char * asset_path)
{
    char buf[256];

    /*----------------
     * Global styles
     *----------------*/
    static bool style_inited = false;

    if (!style_inited) {
        lv_style_init(&dot_base);
        lv_style_set_width(&dot_base, 20);
        lv_style_set_height(&dot_base, 20);
        lv_style_set_radius(&dot_base, 10);
        lv_style_set_bg_opa(&dot_base, (255 * 100 / 100));
        lv_style_set_bg_color(&dot_base, lv_color_hex(0x0000FF));

        lv_style_init(&dot_red);
        lv_style_set_bg_color(&dot_red, lv_color_hex(0xFF0000));

        style_inited = true;
    }

    /*----------------
     * Fonts
     *----------------*/

    /*----------------
     * Images
     *----------------*/


    /*----------------
     * Subjects
     *----------------*/
    lv_subject_init_int(&btn_left_1, 0);
    lv_subject_init_int(&btn_left_2, 0);
    lv_subject_init_int(&btn_left_3, 0);
    lv_subject_init_int(&btn_right_1, 0);
    lv_subject_init_int(&btn_right_2, 0);
    lv_subject_init_int(&btn_right_3, 0);
    lv_subject_init_int(&btn_center, 0);

    /*----------------
     * Translations
     *----------------*/


#if LV_USE_XML
    /*Register widgets*/

    /* Register fonts */

    /* Register subjects */
    lv_xml_register_subject(NULL, "btn_left_1", &btn_left_1);
    lv_xml_register_subject(NULL, "btn_left_2", &btn_left_2);
    lv_xml_register_subject(NULL, "btn_left_3", &btn_left_3);
    lv_xml_register_subject(NULL, "btn_right_1", &btn_right_1);
    lv_xml_register_subject(NULL, "btn_right_2", &btn_right_2);
    lv_xml_register_subject(NULL, "btn_right_3", &btn_right_3);
    lv_xml_register_subject(NULL, "btn_center", &btn_center);

    /* Register callbacks */
#endif

    /* Register all the global assets so that they won't be created again when globals.xml is parsed.
     * While running in the editor skip this step to update the preview when the XML changes */
#if LV_USE_XML && !defined(LV_EDITOR_PREVIEW)

    /* Register images */
#endif

#if LV_USE_XML == 0
    /*--------------------
    *  Permanent screens
    *-------------------*/

    /*If XML is enabled it's assumed that the permanent screens are created
     *manaully from XML using lv_xml_create()*/

#endif
}

/* callbacks */

/**********************
 *   STATIC FUNCTIONS
 **********************/