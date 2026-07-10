project(LogAnalyzerTest C)

file(GLOB_RECURSE LOGANALYZERTEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)

add_executable(LogAnalyzerTest ${LOGANALYZERTEST_SOURCES})

target_link_libraries(LogAnalyzerTest PUBLIC
    LogAnalyzer
)
