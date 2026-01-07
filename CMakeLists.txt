cmake_minimum_required(VERSION 3.20)
project(OpenGLGame)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(FetchContent)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw
    GIT_TAG master
)
FetchContent_MakeAvailable(glfw)
include_directories(${PROJECT_SOURCE_DIR}/include)
add_executable(Game src/main.cpp src/glad.c)
target_link_libraries(Game glfw)
