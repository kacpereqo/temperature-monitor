cmake_minimum_required(VERSION 3.16)

set(IDF_PATH ~/esp/esp-idf)
include(${IDF_PATH}/tools/cmake/project.cmake)
idf_build_set_property(MINIMAL_BUILD ON)
set(TARGET temperature_monitor)

project(${TARGET})
