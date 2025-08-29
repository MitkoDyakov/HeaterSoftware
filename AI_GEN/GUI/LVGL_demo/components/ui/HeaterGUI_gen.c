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

static const char * translation_languages[] = {"en", "de", NULL};
static const char * translation_tags[] = {"ARM", "STOP", NULL};
static const char * translation_texts[] = {
    "ARM", "ARM", /* ARM */
    "STOP", "HALT", /* STOP */
};

/**********************
 *  GLOBAL VARIABLES
 **********************/

/*--------------------
 *  Permanent screens
 *-------------------*/
lv_obj_t * home;

/*----------------
 * Global styles
 *----------------*/
lv_style_t dot_base;
lv_style_t dot_red;

/*----------------
 * Fonts
 *----------------*/
lv_font_t * font_status;
extern uint8_t BebasNeue_Regular_ttf_data[];
extern size_t BebasNeue_Regular_ttf_data_size;
lv_font_t * font_clock;
lv_font_t * font_temp;

/*----------------
 * Images
 *----------------*/
const void * img_down;
extern const void * img_down_data;
const void * img_up;
extern const void * img_up_data;

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
lv_subject_t chan1_armed;
lv_subject_t chan2_armed;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void HeaterGUI_init_gen(const char * asset_path)
{
    char buf[256];

    lv_translation_set_language("en");
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
    /* create tiny ttf font 'font_status' from C array */
    font_status = lv_tiny_ttf_create_data(BebasNeue_Regular_ttf_data, BebasNeue_Regular_ttf_data_size, 22);
    /* create tiny ttf font 'font_clock' from C array */
    font_clock = lv_tiny_ttf_create_data(BebasNeue_Regular_ttf_data, BebasNeue_Regular_ttf_data_size, 25);
    /* create tiny ttf font 'font_temp' from C array */
    font_temp = lv_tiny_ttf_create_data(BebasNeue_Regular_ttf_data, BebasNeue_Regular_ttf_data_size, 57);

    /*----------------
     * Images
     *----------------*/
    img_down = &img_down_data;
    img_up = &img_up_data;


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
    lv_subject_init_int(&chan1_armed, 0);
    lv_subject_init_int(&chan2_armed, 0);

    /*----------------
     * Translations
     *----------------*/
    lv_translation_add_static(translation_languages, translation_tags, translation_texts);


#if LV_USE_XML
    /*Register widgets*/

    /* Register fonts */
    lv_xml_register_font(NULL, "font_status", font_status);
    lv_xml_register_font(NULL, "font_clock", font_clock);
    lv_xml_register_font(NULL, "font_temp", font_temp);

    /* Register subjects */
    lv_xml_register_subject(NULL, "btn_left_1", &btn_left_1);
    lv_xml_register_subject(NULL, "btn_left_2", &btn_left_2);
    lv_xml_register_subject(NULL, "btn_left_3", &btn_left_3);
    lv_xml_register_subject(NULL, "btn_right_1", &btn_right_1);
    lv_xml_register_subject(NULL, "btn_right_2", &btn_right_2);
    lv_xml_register_subject(NULL, "btn_right_3", &btn_right_3);
    lv_xml_register_subject(NULL, "btn_center", &btn_center);
    lv_xml_register_subject(NULL, "chan1_armed", &chan1_armed);
    lv_xml_register_subject(NULL, "chan2_armed", &chan2_armed);

    /* Register callbacks */
#endif

    /* Register all the global assets so that they won't be created again when globals.xml is parsed.
     * While running in the editor skip this step to update the preview when the XML changes */
#if LV_USE_XML && !defined(LV_EDITOR_PREVIEW)

    /* Register images */
    lv_xml_register_image(NULL, "img_down", img_down);
    lv_xml_register_image(NULL, "img_up", img_up);
#endif

#if LV_USE_XML == 0
    /*--------------------
    *  Permanent screens
    *-------------------*/

    /*If XML is enabled it's assumed that the permanent screens are created
     *manaully from XML using lv_xml_create()*/

    home = home_create();
#endif
}

/* callbacks */

/**********************
 *   STATIC FUNCTIONS
 **********************/