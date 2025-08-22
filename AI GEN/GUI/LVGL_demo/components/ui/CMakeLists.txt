idf_component_register(
  SRCS
    "ui.c"
    "HeaterGUI.c"
    "HeaterGUI_gen.c"
    "screens/demo_gen.c"
  INCLUDE_DIRS "." "./screens"
  REQUIRES lvgl
)