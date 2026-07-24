#!/usr/bin/env bash
#
# build_dummySPI.sh - 编译 DummySPI 三个内核模块(out-of-tree,产物集中在 build/)
#
# 用法:
#   ./build_dummySPI.sh          # 编译(默认)
#   ./build_dummySPI.sh build    # 同上
#   ./build_dummySPI.sh clean    # 删掉整个 build/ 目录

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build"

# 需要镜像到 build/ 下的顶层源码目录(build 自己不在这个列表里)
SRC_TOPDIRS=(include spi_virtual_master spi_virtual_slave)

# 编译顺序:backend 依赖 core/Module.symvers,slave 依赖 backend/Module.symvers
BUILD_SUBDIRS=(
    "${BUILD_DIR}/spi_virtual_master/core"
    "${BUILD_DIR}/spi_virtual_master/backend"
    "${BUILD_DIR}/spi_virtual_slave"
)

sync_sources() {
    mkdir -p "${BUILD_DIR}"
    for d in "${SRC_TOPDIRS[@]}"; do
        # 只同步源码文件(.c/.h/Makefile/.template),不带 --delete,
        # 这样已经编译过的 .o/.ko 增量保留,改了哪个 .c 就只重编哪个
        rsync -a \
            --include='*/' \
            --include='*.c' \
            --include='*.h' \
            --include='Makefile' \
            --include='*.template' \
            --exclude='*' \
            "${SCRIPT_DIR}/${d}/" "${BUILD_DIR}/${d}/"
    done
}

ACTION="${1:-build}"

case "${ACTION}" in
    build)
        sync_sources
        for dir in "${BUILD_SUBDIRS[@]}"; do
            echo "==> Building: ${dir}"
            make -C "${dir}"
        done
        echo "==> All DummySPI modules built under ${BUILD_DIR}"
        ;;
    clean)
        echo "==> Removing ${BUILD_DIR}"
        rm -rf "${BUILD_DIR}"
        ;;
    *)
        echo "Usage: $0 [build|clean]"
        exit 1
        ;;
esac