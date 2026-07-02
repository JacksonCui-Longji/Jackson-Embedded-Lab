project(CRCCalculator C)

file(GLOB_RECURSE CRC_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)

# add_executable(CRCCalculator ${CRC_SOURCES})

add_library(CRCCalculator STATIC ${CRC_SOURCES})

target_include_directories(CRCCalculator PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
)
