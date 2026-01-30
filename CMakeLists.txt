cmake_minimum_required(VERSION 3.16)

set(IDF_PATH /home/debilian/esp/esp-idf)
include(${IDF_PATH}/tools/cmake/project.cmake)

idf_build_set_property(MINIMAL_BUILD ON)

project(temperature_monitor)
