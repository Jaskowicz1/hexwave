# Define CMake Minimum.
cmake_minimum_required(VERSION 3.20)
# Project
project("Hexwave" VERSION 0.2 DESCRIPTION "An engine designed to make Interactive Films!")

# GLFW stuff
option(GLFW_BUILD_DOCS OFF)
option(GLFW_BUILD_EXAMPLES OFF)
option(GLFW_BUILD_TESTS OFF)
option(GLFW_BUILD_WAYLAND OFF)
add_subdirectory(libs/glfw)

# We're following the C++17 standard here.
set(CMAKE_CXX_STANDARD 17)

file(GLOB src_code CONFIGURE_DEPENDS "src/engine/*.cpp" "include/engine/*.h" "src/engine/*/*.cpp" "include/engine/*/*.h")
file(GLOB ext_code CONFIGURE_DEPENDS "libs/imgui/*.cpp")

add_subdirectory(libs/glad)

if(WIN32)
    find_package(FFMPEG REQUIRED)
else()
    add_subdirectory(libs/FFmpeg)
endif()

include_directories(
        libs/glad/
        libs/glfw/include/
        libs/glm/
        libs/imgui/
        include/
        include/engine/
        include/stb/
        include/ImGuiNotify/
)

add_definitions("-DGLFW_INCLUDE_NONE")

add_executable(${PROJECT_NAME}
        ${src_code}
        ${ext_code}
        include/miniaudio/miniaudio.h
        include/stb/stb_image.h
)

if(WIN32)
    target_include_directories(${PROJECT_NAME} PRIVATE ${FFMPEG_INCLUDE_DIRS})
    target_link_directories(${PROJECT_NAME} PRIVATE ${FFMPEG_LIBRARY_DIRS})
    target_link_libraries(${PROJECT_NAME}
            glad
            glfw
            ${FFMPEG_LIBRARIES}
            ${GLFW_LIBRARIES}
    )
else()
    target_link_libraries(${PROJECT_NAME}
        glad
        glfw
        FFmpeg
        ${GLFW_LIBRARIES}
)
endif()

file(COPY extras DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
