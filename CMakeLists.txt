cmake_minimum_required(VERSION 3.16.0)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(RoomController)

# Prevents a segfault (environment issue)
set(EXCLUDE_COMPONENTS esp_lcd)