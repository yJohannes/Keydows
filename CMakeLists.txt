cmake_minimum_required(VERSION 3.10)

project(Keydows)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)


include_directories(
    src
    vendor
    )

file(GLOB_RECURSE CORE_SRC src/*.cpp)
list(FILTER CORE_SRC EXCLUDE REGEX "src/tools/*.") # Exclude tools
# list(FILTER CORE_SRC EXCLUDE REGEX "src/tools/smooth_scroll") # Exclude

# WinAPI defines
add_definitions(-DUNICODE -D_UNICODE)

# Main executable
add_executable(keydows ${CORE_SRC})
target_link_libraries(keydows user32 gdi32 shcore gdiplus)

# Get all tool source files (each tool should be in its own subdirectory)
file(GLOB TOOL_DIRS RELATIVE ${CMAKE_SOURCE_DIR}/tools ${CMAKE_SOURCE_DIR}/tools/*)

foreach(TOOL ${TOOL_DIRS})
    # Define the sources for each tool
    file(GLOB TOOL_SRC src/tools/${TOOL}/*.cpp)

    # Create a shared library (DLL) for each tool
    add_library(${TOOL} SHARED ${TOOL_SRC})

    # Define macro for DLL export
    target_compile_definitions(${TOOL} PRIVATE BUILD_DLL)

    # Set output directories for the DLLs
    set_target_properties(${TOOL} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tools
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tools
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tools
    )
endforeach()

# # Define SmoothScroll tool sources
# file(GLOB TOOL_SRC src/tools/smooth_scroll.cpp src/core/input/hl_input.cpp src/core/input/ll_input.cpp src/core/hotkeys/hotkey_manager.cpp)

# # SmoothScroll DLL
# add_library(smooth_scroll SHARED ${TOOL_SRC})

# # Define a macro to indicate this is being built as a DLL
# target_compile_definitions(smooth_scroll PRIVATE BUILD_DLL)

# # Set output directory for the DLL inside the `tools` folder
# set_target_properties(smooth_scroll PROPERTIES
#     RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tools
#     LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tools
#     ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tools
# )