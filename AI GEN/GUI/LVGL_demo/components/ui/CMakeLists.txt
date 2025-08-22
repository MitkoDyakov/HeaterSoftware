idf_component_register(
  SRCS
    "ui.c"
    "HeaterGUI.c"
    "HeaterGUI_gen.c"
    "screens/demo_gen.c"
    # add every generated assets/*.c here if present
  INCLUDE_DIRS "." "./screens"
)