# Define CMake Minimum.
cmake_minimum_required(VERSION 3.25)
# Project
project("Hexwave" VERSION 0.2 DESCRIPTION "An engine designed to make Interactive Films!")

# GLFW stuff
option(GLFW_BUILD_DOCS OFF)
option(GLFW_BUILD_EXAMPLES OFF)
option(GLFW_BUILD_TESTS OFF)
add_subdirectory(libs/glfw)

# We're following the C++17 standard here.
set(CMAKE_CXX_STANDARD 17)

file(GLOB src_code CONFIGURE_DEPENDS "src/engine/*.cpp" "include/engine/*.h" "src/engine/*/*.cpp" "include/engine/*/*.h")
file(GLOB ext_code CONFIGURE_DEPENDS "libs/imgui/*.cpp")
file(GLOB FFmpeg_libs CONFIGURE_DEPENDS "libs/FFmpeg/lib/*.so")

add_subdirectory(libs/glad)

include_directories(
        libs/glad/
        libs/glfw/include/
        libs/glm/
        libs/imgui/
        include/
        include/engine/
        include/ImGuiNotify/
        libs/FFmpeg/include/
)

add_definitions("-DGLFW_INCLUDE_NONE")

add_executable(${PROJECT_NAME}
        ${src_code}
        ${ext_code}
)

target_link_libraries(${PROJECT_NAME}
        glad
        glfw
        ${GLFW_LIBRARIES}
        ${FFmpeg_libs}
)
