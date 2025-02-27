cmake_minimum_required(VERSION 3.10)

project(Keydows)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)


include_directories(
    src
    vendor
    )

file(GLOB_RECURSE KEYDOWS_SRC src/*.cpp)

# WinAPI defines
add_definitions(-DUNICODE -D_UNICODE)

# Main executable
add_executable(keydows ${KEYDOWS_SRC})
target_link_libraries(keydows user32 gdi32 shcore gdiplus)


# cmake_minimum_required(VERSION 3.10)

# project(Keydows)
# set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_CXX_STANDARD_REQUIRED True)


# include_directories(
#     src
#     vendor
#     )

# file(GLOB_RECURSE KEYDOWS_SRC src/*.cpp)
# file(GLOB_RECURSE TOOL_SRC src/tools/*.cpp)

# # Remove tools from keydows
# list(REMOVE_ITEM KEYDOWS_SRC ${TOOL_SRC})

# # WinAPI defines
# add_definitions(-DUNICODE -D_UNICODE)

# # Main executable
# add_executable(keydows ${KEYDOWS_SRC})
# target_compile_definitions(keydows PRIVATE BUILD_CORE)  # Enables exports
# target_link_libraries(keydows user32 gdi32 shcore gdiplus)

# # Build libs (ADD!!!!!!!!)

# # Build tools
# foreach(tool  ${TOOL_SRC})
#     get_filename_component(tool_name ${tool} NAME_WE)
#     add_library(${tool_name} SHARED ${tool})
#     set_target_properties(${tool_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tools)

#     # Link system libraries but NOT the main executable
#     target_link_libraries(${tool_name} user32 gdi32 shcore gdiplus)
# endforeach()
