project(LogAnalyzer C)

file(GLOB_RECURSE LOGANALYZER_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)

add_library(LogAnalyzer STATIC ${LOGANALYZER_SOURCES})

target_link_libraries(LogAnalyzer PUBLIC
    CommonType
)

target_include_directories(LogAnalyzer PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
)
