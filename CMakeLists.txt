# For more information about build system see
# https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html
# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(PDcalibrator)

idf_build_set_property(COMPILE_OPTIONS "-DSDKCONFIG_DEFAULTS" APPEND)
idf_build_set_property(COMPILE_DEFINITIONS "-DSDKCONFIG_DEFAULTS" APPEND)

idf_build_set_property(COMPILE_OPTIONS "-DPROJECT_CONFIG" APPEND)
idf_build_set_property(COMPILE_DEFINITIONS "-DPROJECT_CONFIG" APPEND)

set(SPIFFS_IMAGE_PARTITION spiffs)
set(SPIFFS_IMAGE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/spiffs)
set(SPIFFS_IMAGE_ROMFS)

list(APPEND EXTRA_COMPONENT_DIRS ${PROJECT_DIR}/components)

set(EXTRA_COMPONENT_DIRS ${CMAKE_SOURCE_DIR}/components)

set(EXTRA_COMPONENT_DIRS "components/TFT_eSPI")
