cmake_minimum_required(VERSION 3.10)

project(Keydows)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Recursively gather all source files in the 'src' directory
file(GLOB_RECURSE SOURCES src/*.cpp)

include_directories(src)
add_definitions(-DUNICODE -D_UNICODE)
add_executable(keydows ${SOURCES})
target_link_libraries(keydows user32 gdi32 shcore gdiplus)