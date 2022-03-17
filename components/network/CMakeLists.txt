idf_component_register(SRCS         "network_wifi.c" "network_request.c" "network_utils.c" "network_qweather.c"
                       INCLUDE_DIRS "."
                       REQUIRES     lvgl bsp nvs_flash esp_wifi esp-tls zlib json
)
