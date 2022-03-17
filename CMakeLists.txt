cmake_minimum_required(VERSION 3.5)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(esp32c3-st7789-lvgl-weather)

target_add_binary_data(esp32c3-st7789-lvgl-weather.elf "static/ca_certs/aaa.pem" TEXT)
# target_add_binary_data(esp32c3-st7789-lvgl-weather.elf "static/ca_certs/digicert.pem" TEXT)

# target_add_binary_data(esp32c3-st7789-lvgl-weather.elf "static/images/hutao.gif" TEXT)
target_add_binary_data(esp32c3-st7789-lvgl-weather.elf "static/images/miko.gif" TEXT)
