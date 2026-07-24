#!/usr/bin/env bash
#
# build_dummySPI.sh - 编译 DummySPI 三个内核模块
#
# 用法:
#   ./build_dummySPI.sh          # 编译(默认)
#   ./build_dummySPI.sh build    # 同上
#   ./build_dummySPI.sh clean    # 清理

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# 编译顺序不能乱:backend 依赖 core/Module.symvers,slave 依赖 backend/Module.symvers
BUILD_DIRS=(
    "${SCRIPT_DIR}/spi_virtual_master/core"
    "${SCRIPT_DIR}/spi_virtual_master/backend"
    "${SCRIPT_DIR}/spi_virtual_slave"
)

ACTION="${1:-build}"

case "${ACTION}" in
    build)
        for dir in "${BUILD_DIRS[@]}"; do
            echo "==> Building: ${dir}"
            make -C "${dir}"
        done
        echo "==> All DummySPI modules built."
        ;;
    clean)
        for dir in "${BUILD_DIRS[@]}"; do
            echo "==> Cleaning: ${dir}"
            make -C "${dir}" clean
        done
        ;;
    *)
        echo "Usage: $0 [build|clean]"
        exit 1
        ;;
esac