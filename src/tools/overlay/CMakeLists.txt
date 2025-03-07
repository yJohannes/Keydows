file(GLOB TOOL_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)

list(APPEND TOOL_SRC
    ${CMAKE_SOURCE_DIR}/src/core/input/hl_input.cpp
    ${CMAKE_SOURCE_DIR}/src/core/input/ll_input.cpp
    ${CMAKE_SOURCE_DIR}/src/core/hotkeys/hotkey_manager.cpp
)

add_library(overlay SHARED ${TOOL_SRC})
target_compile_definitions(overlay PRIVATE BUILD_TOOL)
target_include_directories(overlay PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

set_target_properties(overlay PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tools
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tools
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tools
)