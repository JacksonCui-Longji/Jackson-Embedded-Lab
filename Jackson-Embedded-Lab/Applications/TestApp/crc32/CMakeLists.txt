project(CRCTest C)

file(GLOB_RECURSE CRC_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)

add_executable(CRCTest ${CRC_SOURCES})

target_link_libraries(CRCTest PUBLIC
    CRCCalculator
)
