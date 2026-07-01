#!/usr/bin/env bash
set -euo pipefail

# 脚本所在目录即 Project 目录（要求在 Project 下执行）
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="${SCRIPT_DIR}"
BUILD_DIR="${PROJECT_DIR}/build"
OUTPUT_DIR="${PROJECT_DIR}/output"

BUILD_TYPE="${1:-Release}"   # 用法: ./build.sh [Debug|Release]，默认 Release

echo ">>> Project 目录 : ${PROJECT_DIR}"
echo ">>> Build 目录   : ${BUILD_DIR}"
echo ">>> Output 目录  : ${OUTPUT_DIR}"
echo ">>> Build 类型   : ${BUILD_TYPE}"

mkdir -p "${BUILD_DIR}"
mkdir -p "${OUTPUT_DIR}"

cmake -S "${PROJECT_DIR}" -B "${BUILD_DIR}" -DCMAKE_BUILD_TYPE="${BUILD_TYPE}"

# 并行编译，自动探测 CPU 核数（Linux/macOS 通用写法）
JOBS="$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)"
cmake --build "${BUILD_DIR}" -j"${JOBS}"

echo ""
echo ">>> 编译完成，可执行文件位于: ${OUTPUT_DIR}"
ls -l "${OUTPUT_DIR}"