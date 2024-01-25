# Required for FetchContent_Declare()
include(FetchContent)
# Define CMake Minimum.
cmake_minimum_required(VERSION 3.20)
# Project
project("interactive_film_engine" VERSION 0.1 DESCRIPTION "")

# We're following the C++17 standard here.
set(CMAKE_CXX_STANDARD 17)

file(GLOB src_code CONFIGURE_DEPENDS "src/engine/*.cpp" "include/engine/*.h" "src/engine/*/*.cpp" "include/engine/*/*.h")

# Huge thanks to https://github.com/cmmw/imgui-glfw-glad-glm for making this possible and making my life easier.
FetchContent_Declare(
        imgui-glfw-glad-glm
        GIT_REPOSITORY https://github.com/cmmw/imgui-glfw-glad-glm.git
        GIT_TAG v4.1.0
)

FetchContent_MakeAvailable(imgui-glfw-glad-glm)

add_subdirectory(libs/ffmpeg)

add_executable(${PROJECT_NAME}
        ${src_code}
)

# imgui will link glfw and glad, so we only need to link glm.
target_link_libraries(${PROJECT_NAME}
        imgui
        glm
        ffmpeg
)

# Required to ensure that include files are able to be accessed via "main.h" rather than "../../include/engine/main.h".
target_include_directories(${PROJECT_NAME} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/engine>
)
