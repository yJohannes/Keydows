file(GLOB TOOL_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)

list(APPEND TOOL_SRC
    ${CMAKE_SOURCE_DIR}/src/core/input/hl_input.cpp
    ${CMAKE_SOURCE_DIR}/src/core/input/ll_input.cpp
)

add_library(smooth_scroll SHARED ${TOOL_SRC})
target_compile_definitions(smooth_scroll PRIVATE BUILD_TOOL)
target_include_directories(smooth_scroll PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

set_target_properties(smooth_scroll PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tools
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tools
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tools
)