project(CRCCalculator C)

file(GLOB_RECURSE CRC_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)

add_executable(CRCCalculator ${CRC_SOURCES})

target_include_directories(CRCCalculator PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

# 如果这个模块有自己的 third_party 依赖，可以在这里处理，例如：
# add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/third_party ${CMAKE_CURRENT_BINARY_DIR}/third_party)