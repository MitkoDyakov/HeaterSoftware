    idf_component_register(SRCS "hello_world_main.c"
                    PRIV_REQUIRES spi_flash
                    REQUIRES ui lvgl 
                    INCLUDE_DIRS ".")
    
    idf_build_set_property(COMPILE_OPTIONS "-DLV_CONF_PATH=\"${CMAKE_SOURCE_DIR}/main/lv_conf.h\"" APPEND)
    idf_build_set_property(COMPILE_OPTIONS "-DLV_LVGL_H_INCLUDE_SIMPLE=1" APPEND)