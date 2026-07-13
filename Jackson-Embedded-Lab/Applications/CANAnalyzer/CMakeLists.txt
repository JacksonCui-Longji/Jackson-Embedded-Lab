project(CANAnalyzer C)

file(GLOB_RECURSE CRC_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)

add_library(CANAnalyzer STATIC ${CRC_SOURCES})

target_link_libraries(CANAnalyzer PUBLIC
    CommonType
)

target_include_directories(CANAnalyzer PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
)
