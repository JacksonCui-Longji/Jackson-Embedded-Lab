project(SpiApp)

file(GLOB_RECURSE SPIAPP_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)

add_executable(spi_app ${SPIAPP_SOURCE})

target_link_libraries(spi_app PUBLIC
    CommonType
)

target_include_directories(spi_app PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
)