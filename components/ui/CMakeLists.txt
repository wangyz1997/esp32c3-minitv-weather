idf_component_register(SRCS         "ui_main_screen.c" "ui_connecting_screen.c" "ui_qweather_icon.c"
                       INCLUDE_DIRS "."
                       REQUIRES     lvgl lvgl_font network
)
