project(HexToolTest C)

file(GLOB_RECURSE HEXTOOL_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)

add_executable(HexToolTest ${HEXTOOL_SOURCES})

target_link_libraries(HexToolTest PUBLIC
    HexTool
)
