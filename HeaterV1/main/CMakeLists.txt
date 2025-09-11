idf_component_register(
    SRCS
        "main.c"
        "mailman/i2c_task.c"
        "mailman/ap33772s.c"
        "mailman/ads7142.c"
        "mailman/TMP110.c"
        "switchboard/user_input.c"
        "composer/buzzer.c"
        "fireman/fireman.c"
        "wiseman/wiseman.c"
    INCLUDE_DIRS
        "."
        "mailman"
        "switchboard"
        "composer"
        "fireman"
        "wiseman"
)

idf_build_set_property(COMPILE_OPTIONS "-DLV_CONF_PATH=\"${CMAKE_SOURCE_DIR}/main/lv_conf.h\"" APPEND)
idf_build_set_property(COMPILE_OPTIONS "-DLV_LVGL_H_INCLUDE_SIMPLE=1" APPEND)