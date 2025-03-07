cmake_minimum_required(VERSION 3.12)
project(Keydows)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_compile_definitions(-DUNICODE -D_UNICODE)  # WinAPI defines
include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/vendor
)

set(CORE_SRC
    src/main.cpp
    src/core/application.cpp
    src/core/input/hl_input.cpp
    src/core/input/ll_input.cpp
    src/core/hotkeys/hotkey_manager.cpp
)

add_executable(keydows ${CORE_SRC})
target_link_libraries(keydows user32 gdi32 shcore gdiplus)

# CONFIGURE_DEPENDS runs configuration if new tools are added 
file(GLOB TOOL_DIRS CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/tools/*)

# Build each tool
foreach(dir ${TOOL_DIRS})
    if (IS_DIRECTORY ${dir})
        get_filename_component(dir_name ${dir} NAME)
        message(STATUS "Found directory: ${dir_name} (${dir})")
        add_subdirectory(${dir})
    endif()
endforeach()