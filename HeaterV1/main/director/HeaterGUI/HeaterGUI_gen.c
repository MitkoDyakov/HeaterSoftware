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
static const char * translation_tags[] = {"ARM", "STOP", "CH", "TARGET TEMP", NULL};
static const char * translation_texts[] = {
    "ARM", "ARM", /* ARM */
    "STOP", "HALT", /* STOP */
    "CH", "KN", /* CH */
    "TARGET TEMP", "TARGET TEMP", /* TARGET TEMP */
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
lv_font_t * font_ch_label_temp_small;
extern uint8_t LaugeGothic_mode_ttf_data[];
extern size_t LaugeGothic_mode_ttf_data_size;
lv_font_t * font_start_card;
lv_font_t * font_ch_temp_big;
lv_font_t * font_channel_dot;
lv_font_t * font_target_temp;

/*----------------
 * Images
 *----------------*/
const void * channel_bg;
extern const void * channel_bg_data;
const void * page_bg;
extern const void * page_bg_data;
const void * start_bg;
extern const void * start_bg_data;
const void * target_bg;
extern const void * target_bg_data;
const void * info_bg;
extern const void * info_bg_data;

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
lv_subject_t ch1_temp_big;
lv_subject_t ch1_temp_small;
lv_subject_t ch2_temp_big;
lv_subject_t ch2_temp_small;
lv_subject_t pageSelect;
lv_subject_t targetTemp;
lv_subject_t opTime;
lv_subject_t command;
lv_subject_t settingsSelect;
lv_subject_t default_temp;
lv_subject_t activeCh;
lv_subject_t brightness;
lv_subject_t sleepTimer;
lv_subject_t soundEnable;
lv_subject_t runTime;
lv_subject_t swVer;
lv_subject_t hwVer;
lv_subject_t SN;
lv_subject_t fiveV_available;
lv_subject_t fiveV;
lv_subject_t nineV_available;
lv_subject_t nineV;
lv_subject_t fifteenV_available;
lv_subject_t fifteenV;
lv_subject_t twentyV_available;
lv_subject_t twentyV;
lv_subject_t activePDO;

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
    /* create tiny ttf font 'font_ch_label_temp_small' from C array */
    font_ch_label_temp_small = lv_tiny_ttf_create_data(LaugeGothic_mode_ttf_data, LaugeGothic_mode_ttf_data_size, 24);
    /* create tiny ttf font 'font_start_card' from C array */
    font_start_card = lv_tiny_ttf_create_data(LaugeGothic_mode_ttf_data, LaugeGothic_mode_ttf_data_size, 34);
    /* create tiny ttf font 'font_ch_temp_big' from C array */
    font_ch_temp_big = lv_tiny_ttf_create_data(LaugeGothic_mode_ttf_data, LaugeGothic_mode_ttf_data_size, 46);
    /* create tiny ttf font 'font_channel_dot' from C array */
    font_channel_dot = lv_tiny_ttf_create_data(LaugeGothic_mode_ttf_data, LaugeGothic_mode_ttf_data_size, 50);
    /* create tiny ttf font 'font_target_temp' from C array */
    font_target_temp = lv_tiny_ttf_create_data(LaugeGothic_mode_ttf_data, LaugeGothic_mode_ttf_data_size, 64);

    /*----------------
     * Images
     *----------------*/
    channel_bg = &channel_bg_data;
    page_bg = &page_bg_data;
    start_bg = &start_bg_data;
    target_bg = &target_bg_data;
    info_bg = &info_bg_data;


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
    lv_subject_init_int(&ch1_temp_big, 24);
    lv_subject_init_int(&ch1_temp_small, 9);
    lv_subject_init_int(&ch2_temp_big, 23);
    lv_subject_init_int(&ch2_temp_small, 0);
    lv_subject_init_int(&pageSelect, 0);
    lv_subject_init_int(&targetTemp, 30);
    static char opTime_buf[UI_SUBJECT_STRING_LENGTH];
    static char opTime_prev_buf[UI_SUBJECT_STRING_LENGTH];
    lv_subject_init_string(&opTime,
                            opTime_buf,
                            opTime_prev_buf,
                            UI_SUBJECT_STRING_LENGTH,
                            "00:00"
                          );
    static char command_buf[UI_SUBJECT_STRING_LENGTH];
    static char command_prev_buf[UI_SUBJECT_STRING_LENGTH];
    lv_subject_init_string(&command,
                            command_buf,
                            command_prev_buf,
                            UI_SUBJECT_STRING_LENGTH,
                            "START"
                          );
    lv_subject_init_int(&settingsSelect, 0);
    lv_subject_init_int(&default_temp, 30);
    static char activeCh_buf[UI_SUBJECT_STRING_LENGTH];
    static char activeCh_prev_buf[UI_SUBJECT_STRING_LENGTH];
    lv_subject_init_string(&activeCh,
                            activeCh_buf,
                            activeCh_prev_buf,
                            UI_SUBJECT_STRING_LENGTH,
                            "CH1/2"
                          );
    lv_subject_init_int(&brightness, 75);
    lv_subject_init_int(&sleepTimer, 30);
    static char soundEnable_buf[UI_SUBJECT_STRING_LENGTH];
    static char soundEnable_prev_buf[UI_SUBJECT_STRING_LENGTH];
    lv_subject_init_string(&soundEnable,
                            soundEnable_buf,
                            soundEnable_prev_buf,
                            UI_SUBJECT_STRING_LENGTH,
                            "ON"
                          );
    lv_subject_init_int(&runTime, 0);
    static char swVer_buf[UI_SUBJECT_STRING_LENGTH];
    static char swVer_prev_buf[UI_SUBJECT_STRING_LENGTH];
    lv_subject_init_string(&swVer,
                            swVer_buf,
                            swVer_prev_buf,
                            UI_SUBJECT_STRING_LENGTH,
                            "0.1v"
                          );
    static char hwVer_buf[UI_SUBJECT_STRING_LENGTH];
    static char hwVer_prev_buf[UI_SUBJECT_STRING_LENGTH];
    lv_subject_init_string(&hwVer,
                            hwVer_buf,
                            hwVer_prev_buf,
                            UI_SUBJECT_STRING_LENGTH,
                            "0.2v"
                          );
    static char SN_buf[UI_SUBJECT_STRING_LENGTH];
    static char SN_prev_buf[UI_SUBJECT_STRING_LENGTH];
    lv_subject_init_string(&SN,
                            SN_buf,
                            SN_prev_buf,
                            UI_SUBJECT_STRING_LENGTH,
                            "#T001"
                          );
    lv_subject_init_int(&fiveV_available, 0);
    static char fiveV_buf[UI_SUBJECT_STRING_LENGTH];
    static char fiveV_prev_buf[UI_SUBJECT_STRING_LENGTH];
    lv_subject_init_string(&fiveV,
                            fiveV_buf,
                            fiveV_prev_buf,
                            UI_SUBJECT_STRING_LENGTH,
                            "3.5A"
                          );
    lv_subject_init_int(&nineV_available, 0);
    static char nineV_buf[UI_SUBJECT_STRING_LENGTH];
    static char nineV_prev_buf[UI_SUBJECT_STRING_LENGTH];
    lv_subject_init_string(&nineV,
                            nineV_buf,
                            nineV_prev_buf,
                            UI_SUBJECT_STRING_LENGTH,
                            "3.5A"
                          );
    lv_subject_init_int(&fifteenV_available, 0);
    static char fifteenV_buf[UI_SUBJECT_STRING_LENGTH];
    static char fifteenV_prev_buf[UI_SUBJECT_STRING_LENGTH];
    lv_subject_init_string(&fifteenV,
                            fifteenV_buf,
                            fifteenV_prev_buf,
                            UI_SUBJECT_STRING_LENGTH,
                            "2.5A"
                          );
    lv_subject_init_int(&twentyV_available, 0);
    static char twentyV_buf[UI_SUBJECT_STRING_LENGTH];
    static char twentyV_prev_buf[UI_SUBJECT_STRING_LENGTH];
    lv_subject_init_string(&twentyV,
                            twentyV_buf,
                            twentyV_prev_buf,
                            UI_SUBJECT_STRING_LENGTH,
                            "2.5A"
                          );
    lv_subject_init_int(&activePDO, 0);

    /*----------------
     * Translations
     *----------------*/
    lv_translation_add_static(translation_languages, translation_tags, translation_texts);


#if LV_USE_XML
    /*Register widgets*/

    /* Register fonts */
    lv_xml_register_font(NULL, "font_ch_label_temp_small", font_ch_label_temp_small);
    lv_xml_register_font(NULL, "font_start_card", font_start_card);
    lv_xml_register_font(NULL, "font_ch_temp_big", font_ch_temp_big);
    lv_xml_register_font(NULL, "font_channel_dot", font_channel_dot);
    lv_xml_register_font(NULL, "font_target_temp", font_target_temp);

    /* Register subjects */
    lv_xml_register_subject(NULL, "btn_left_1", &btn_left_1);
    lv_xml_register_subject(NULL, "btn_left_2", &btn_left_2);
    lv_xml_register_subject(NULL, "btn_left_3", &btn_left_3);
    lv_xml_register_subject(NULL, "btn_right_1", &btn_right_1);
    lv_xml_register_subject(NULL, "btn_right_2", &btn_right_2);
    lv_xml_register_subject(NULL, "btn_right_3", &btn_right_3);
    lv_xml_register_subject(NULL, "btn_center", &btn_center);
    lv_xml_register_subject(NULL, "ch1_temp_big", &ch1_temp_big);
    lv_xml_register_subject(NULL, "ch1_temp_small", &ch1_temp_small);
    lv_xml_register_subject(NULL, "ch2_temp_big", &ch2_temp_big);
    lv_xml_register_subject(NULL, "ch2_temp_small", &ch2_temp_small);
    lv_xml_register_subject(NULL, "pageSelect", &pageSelect);
    lv_xml_register_subject(NULL, "targetTemp", &targetTemp);
    lv_xml_register_subject(NULL, "opTime", &opTime);
    lv_xml_register_subject(NULL, "command", &command);
    lv_xml_register_subject(NULL, "settingsSelect", &settingsSelect);
    lv_xml_register_subject(NULL, "default_temp", &default_temp);
    lv_xml_register_subject(NULL, "activeCh", &activeCh);
    lv_xml_register_subject(NULL, "brightness", &brightness);
    lv_xml_register_subject(NULL, "sleepTimer", &sleepTimer);
    lv_xml_register_subject(NULL, "soundEnable", &soundEnable);
    lv_xml_register_subject(NULL, "runTime", &runTime);
    lv_xml_register_subject(NULL, "swVer", &swVer);
    lv_xml_register_subject(NULL, "hwVer", &hwVer);
    lv_xml_register_subject(NULL, "SN", &SN);
    lv_xml_register_subject(NULL, "fiveV_available", &fiveV_available);
    lv_xml_register_subject(NULL, "fiveV", &fiveV);
    lv_xml_register_subject(NULL, "nineV_available", &nineV_available);
    lv_xml_register_subject(NULL, "nineV", &nineV);
    lv_xml_register_subject(NULL, "fifteenV_available", &fifteenV_available);
    lv_xml_register_subject(NULL, "fifteenV", &fifteenV);
    lv_xml_register_subject(NULL, "twentyV_available", &twentyV_available);
    lv_xml_register_subject(NULL, "twentyV", &twentyV);
    lv_xml_register_subject(NULL, "activePDO", &activePDO);

    /* Register callbacks */
#endif

    /* Register all the global assets so that they won't be created again when globals.xml is parsed.
     * While running in the editor skip this step to update the preview when the XML changes */
#if LV_USE_XML && !defined(LV_EDITOR_PREVIEW)

    /* Register images */
    lv_xml_register_image(NULL, "channel_bg", channel_bg);
    lv_xml_register_image(NULL, "page_bg", page_bg);
    lv_xml_register_image(NULL, "start_bg", start_bg);
    lv_xml_register_image(NULL, "target_bg", target_bg);
    lv_xml_register_image(NULL, "info_bg", info_bg);
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