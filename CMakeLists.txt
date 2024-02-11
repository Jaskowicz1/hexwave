# Define CMake Minimum.
cmake_minimum_required(VERSION 3.25)
# Project
project("interactive_film_engine" VERSION 0.1 DESCRIPTION "")

# GLFW stuff
option(GLFW_BUILD_DOCS OFF)
option(GLFW_BUILD_EXAMPLES OFF)
option(GLFW_BUILD_TESTS OFF)
add_subdirectory(libs/glfw)

# We're following the C++17 standard here.
set(CMAKE_CXX_STANDARD 17)

file(GLOB src_code CONFIGURE_DEPENDS "src/engine/*.cpp" "include/engine/*.h" "src/engine/*/*.cpp" "include/engine/*/*.h")
file(GLOB ext_code CONFIGURE_DEPENDS "libs/imgui/*.cpp")

if(NOT WIN32)
    set(GLAD_LIBRARIES dl)
endif()

add_subdirectory(libs/ffmpeg)
add_subdirectory(libs/glad)

include_directories(
        include/
        include/engine/
        libs/glad/
        libs/glfw/include/
        libs/glm/
        libs/imgui/
        include/ImGuiNotify/
)

add_definitions(-DGLFW_INCLUDE_NONE -DPROJECT_SOURCE_DIR=\"${PROJECT_SOURCE_DIR}\")

add_executable(${PROJECT_NAME}
        ${src_code}
        ${ext_code}
)

target_link_libraries(${PROJECT_NAME}
        glad
        glfw
        ffmpeg
        ${GLFW_LIBRARIES}
)

# Required to ensure that include files are able to be accessed via "main.h" rather than "../../include/engine/main.h".
#target_include_directories(${PROJECT_NAME} PUBLIC
#        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
#        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/engine>
#        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/ImGuiNotify>
#)
