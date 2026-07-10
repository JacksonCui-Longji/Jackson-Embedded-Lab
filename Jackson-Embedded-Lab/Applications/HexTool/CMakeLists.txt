project(HexTool C)

file(GLOB_RECURSE HEXTOOL_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)

add_library(HexTool STATIC ${HEXTOOL_SOURCES})

target_link_libraries(HexTool PUBLIC
    CommonType
)

target_include_directories(HexTool PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
)
