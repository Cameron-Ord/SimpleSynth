cmake_minimum_required(VERSION 4.4.3)
set(CMAKE_C_COMPILER "clang")
set(CMAKE_CXX_COMPILER "clang++")

set(CMAKE_BUILD_TYPE Debug)

project(synthesizer LANGUAGES CXX)
set(SRCS 
    src/main.cpp
    src/keyboard.cpp
    src/audio.cpp
)

add_subdirectory(third_party/SDL3)
add_subdirectory(third_party/rtmidi)
add_executable(synthesizer ${SRCS})
target_link_libraries(synthesizer PRIVATE rtmidi SDL3::SDL3)