set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
cmake_minimum_required(VERSION 3.20)
project(OpenGLGame)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_SCAN_FOR_MODULES ON)

include(FetchContent)
FetchContent_Declare(
    glfw
    URL https://github.com/glfw/glfw/releases/download/3.4/glfw-3.4.zip
)
FetchContent_MakeAvailable(glfw)

FetchContent_Declare(
    freetype
    GIT_REPOSITORY https://github.com/freetype/freetype.git
    GIT_TAG        VER-2-13-3
)
FetchContent_MakeAvailable(freetype)

include_directories(${PROJECT_SOURCE_DIR}/include)

file(GLOB MODULE_SOURCES CONFIGURE_DEPENDS src/*.ixx)
add_executable(Game src/main.cpp src/glad.c ${MODULE_SOURCES})
target_compile_features(Game PRIVATE cxx_std_23)
target_link_libraries(Game glfw opengl32 freetype)

add_custom_command(TARGET Game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/shaders"
        "$<TARGET_FILE_DIR:Game>/shaders"
    COMMENT "Copying shaders to output directory..."
)
add_custom_command(TARGET Game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:Game>/assets"
    COMMENT "Copying assets to output directory..."
)

# stb, glm, and miniaudio are included locally in the include/ folder
