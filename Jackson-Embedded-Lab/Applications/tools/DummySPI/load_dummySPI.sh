#!/usr/bin/env bash
#
# load_dummySPI.sh - 加载/卸载 DummySPI 三个内核模块
#
# 用法:
#   ./load_dummySPI.sh load     # sudo insmod 三个模块(正确顺序)
#   ./load_dummySPI.sh unload   # sudo rmmod 三个模块(反向顺序)
#   ./load_dummySPI.sh reload   # unload + load
#   ./load_dummySPI.sh status   # 查看当前加载状态 + /dev/spidev0.0 + 最近 dmesg

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

CORE_KO="${SCRIPT_DIR}/spi_virtual_master/core/spi_ctrl_core.ko"
BACKEND_KO="${SCRIPT_DIR}/spi_virtual_master/backend/backend_virtual.ko"
SLAVE_KO="${SCRIPT_DIR}/spi_virtual_slave/spi_virtual_slave.ko"

do_load() {
    for ko in "${CORE_KO}" "${BACKEND_KO}" "${SLAVE_KO}"; do
        [[ -f "${ko}" ]] || { echo "!! Missing: ${ko} (先跑 build_dummySPI.sh)"; exit 1; }
    done

    echo "==> insmod ${CORE_KO}"
    sudo insmod "${CORE_KO}"
    echo "==> insmod ${BACKEND_KO}"
    sudo insmod "${BACKEND_KO}"
    echo "==> insmod ${SLAVE_KO}"
    sudo insmod "${SLAVE_KO}"

    echo "==> Loaded. Checking /dev/spidev0.0 ..."
    if [[ -e /dev/spidev0.0 ]]; then
        echo "    /dev/spidev0.0 exists."
    else
        echo "    !! /dev/spidev0.0 NOT found, check dmesg."
    fi
}

do_unload() {
    for mod in spi_virtual_slave backend_virtual spi_ctrl_core; do
        if lsmod | grep -q "^${mod} "; then
            echo "==> rmmod ${mod}"
            sudo rmmod "${mod}"
        else
            echo "==> ${mod} not loaded, skip"
        fi
    done
}

do_status() {
    echo "==> lsmod:"
    lsmod | grep -E "spi_ctrl_core|backend_virtual|spi_virtual_slave" || echo "   (none loaded)"
    echo "==> /dev/spidev0.0:"
    [[ -e /dev/spidev0.0 ]] && echo "   exists" || echo "   not present"
    echo "==> recent dmesg (svpi):"
    dmesg | grep -i svpi | tail -n 10 || true
}

ACTION="${1:-}"

case "${ACTION}" in
    load)    do_load ;;
    unload)  do_unload ;;
    reload)  do_unload; do_load ;;
    status)  do_status ;;
    *)
        echo "Usage: $0 [load|unload|reload|status]"
        exit 1
        ;;
esac