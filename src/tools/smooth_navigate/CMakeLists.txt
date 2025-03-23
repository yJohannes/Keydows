file(GLOB TOOL_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)

list(APPEND TOOL_SRC
    ${CMAKE_SOURCE_DIR}/src/core/input/hl_input.cpp
    ${CMAKE_SOURCE_DIR}/src/core/input/ll_input.cpp
)

add_library(smooth_navigate SHARED ${TOOL_SRC})
target_compile_definitions(smooth_navigate PRIVATE BUILD_TOOL)
target_include_directories(smooth_navigate PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(smooth_navigate PRIVATE winmm)

set_target_properties(smooth_navigate PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tools
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tools
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tools
)