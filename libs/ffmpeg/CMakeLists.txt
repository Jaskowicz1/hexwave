# Define CMake Minimum.
cmake_minimum_required(VERSION 3.20)
# Project
project(ffmpeg)

find_package(PkgConfig REQUIRED)

pkg_check_modules(AVCODEC       REQUIRED IMPORTED_TARGET libavcodec)
pkg_check_modules(AVFORMAT      REQUIRED IMPORTED_TARGET libavformat)
pkg_check_modules(AVUTIL        REQUIRED IMPORTED_TARGET libavutil)
pkg_check_modules(SWSCALE       REQUIRED IMPORTED_TARGET libswscale)

add_library(ffmpeg INTERFACE IMPORTED GLOBAL)

target_link_libraries(ffmpeg INTERFACE
        PkgConfig::AVCODEC
        PkgConfig::AVFORMAT
        PkgConfig::AVUTIL
        PkgConfig::SWSCALE
)