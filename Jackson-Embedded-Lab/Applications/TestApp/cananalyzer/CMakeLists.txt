project(CanAnalyzerTest C)

file(GLOB_RECURSE CANANALYZERTEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)

add_executable(CanAnalyzerTest ${CANANALYZERTEST_SOURCES})

target_link_libraries(CanAnalyzerTest PUBLIC
    CANAnalyzer
)

target_link_directories(CanAnalyzerTest PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
)
