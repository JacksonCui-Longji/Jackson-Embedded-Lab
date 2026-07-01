cmake_minimum_required(VERSION 3.16)
project(JEL C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Project 的上一级就是仓库根目录，Applications 在这里
set(REPO_ROOT ${CMAKE_SOURCE_DIR}/..)
set(APPLICATIONS_DIR ${REPO_ROOT}/Applications)

# 所有子模块最终生成的可执行文件统一放到 Project/output
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output)
# 兼容多配置生成器（如 Visual Studio）
foreach(CFG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${CFG} CFG_UPPER)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CFG_UPPER} ${CMAKE_SOURCE_DIR}/output)
endforeach()

# 自动扫描 Applications 目录下所有含 CMakeLists.txt 的子目录
file(GLOB APP_CANDIDATES RELATIVE ${APPLICATIONS_DIR} ${APPLICATIONS_DIR}/*)

foreach(APP_NAME ${APP_CANDIDATES})
    set(APP_PATH ${APPLICATIONS_DIR}/${APP_NAME})
    if(IS_DIRECTORY ${APP_PATH} AND EXISTS ${APP_PATH}/CMakeLists.txt)
        message(STATUS "==> 加入编译: ${APP_NAME}")
        # 显式指定 binary_dir，让每个子模块的中间产物
        # 落在 Project/build/<子模块名>/ 下，互不混淆
        add_subdirectory(${APP_PATH} ${CMAKE_BINARY_DIR}/${APP_NAME})
    endif()
endforeach()