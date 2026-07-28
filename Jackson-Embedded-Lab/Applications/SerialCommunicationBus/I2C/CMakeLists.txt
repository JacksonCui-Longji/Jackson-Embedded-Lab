project(I2CApp)

file(GLOB_RECURSE I2CAPP_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)

add_executable(i2c_app ${I2CAPP_SOURCE})

target_link_libraries(i2c_app PUBLIC
    CommonType
)

target_include_directories(i2c_app PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
)