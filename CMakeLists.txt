cmake_minimum_required(VERSION 3.20.0)

set(ZEPHYR_EXTRA_MODULES "${CMAKE_CURRENT_SOURCE_DIR}/camera_service/")

find_package(Zephyr)
project(radar)

zephyr_include_directories(include/)

FILE(GLOB app_sources src/*.c)
target_sources(app PRIVATE ${app_sources})