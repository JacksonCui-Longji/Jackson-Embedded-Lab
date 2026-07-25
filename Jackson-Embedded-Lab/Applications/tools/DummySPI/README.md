# DummySPI

在没有真实 SPI 硬件的情况下,通过 Linux 内核模块模拟一条完整的 SPI 总线(Controller + 从设备),用于练习 SPI 协议编程和 Linux SPI 子系统驱动开发。

## 设计目标

真实 SPI 总线是纯电气层面的主从交互,MISO/MOSI 物理线负责数据传输,内核不需要额外撮合。纯软件模拟没有物理线,因此引入一条"虚拟总线"来代替电气连接,让 Master 和 Slave 两个内核模块能够互相通信。

整体分层参考 AUTOSAR MCAL 思路:上层(Controller 注册骨架)硬件无关、长期可复用;下层(Backend)是可插拔的具体传输实现,现在是虚拟总线,以后接真实硬件时可以整体替换,不影响上层代码。

## 目录结构

```
DummySPI/
├── include/
│   ├── svpi_backend.h        # core <-> backend 的接口契约(真实,长期不变)
│   └── svpi_bus.h            # backend <-> slave 的虚拟总线接口(dummy专用)
│
├── spi_virtual_master/
│   ├── core/
│   │   ├── spi_ctrl_core.c   # 【REAL】spi_controller 注册骨架,transfer_one_message 外层,spidev 挂载
│   │   └── Makefile
│   └── backend/
│       ├── backend_virtual.c # 【DUMMY】虚拟总线转发逻辑,走 svpi_register_slave 那一套
│       ├── backend_hw.c.template  # 【预留】以后接真实硬件时,改名为 .c 并实现 do_transfer()
│       └── Makefile
│
├── spi_virtual_slave/
│   ├── spi_virtual_slave.c   # 【DUMMY】模拟寄存器堆芯片(EEPROM-like),整个模块都是临时的
│   └── Makefile
│
├── test/
│   └── spi_test.c            # 用户态验证程序,通过 /dev/spidev0.0 读写寄存器堆
│
├── build/                    # 编译产物集中目录(rsync 镜像源码后 out-of-tree 编译),可整个删除重来
│
├── build_dummySPI.sh          # 编译脚本
├── load_dummySPI.sh           # 加载/卸载/状态查看脚本
└── README.md
```

## 架构与数据流

```
用户态测试程序 (ioctl SPI_IOC_MESSAGE)
        ↓
spidev.ko                       (内核自带驱动)
        ↓
core/spi_ctrl_core.ko           【REAL】注册 spi_controller,transfer_one_message 转发
        ↓
backend/backend_virtual.ko      【DUMMY】虚拟总线转发(按 chip_select 查表)
        ↓
spi_virtual_slave.ko            【DUMMY】寄存器堆芯片行为模拟
```

### 哪些代码真实可迁移,哪些是临时脚手架

| 部分 | 真实/Dummy | 说明 |
|---|---|---|
| `core/spi_ctrl_core.c` 的注册骨架 | **真实** | `spi_controller` 注册流程、`transfer_one_message` 外层结构、`spi_new_device` 挂载 spidev,真实硬件驱动也要走这套 API |
| `core/spi_ctrl_core.c` 内部转发给 backend | 真实(接口层面) | `svpi_backend_ops.do_transfer()` 这个"seam"本身是真实设计,只是当前实现是虚拟总线 |
| `backend/backend_virtual.c` | **Dummy** | 整个文件都是为了弥补没有物理线而写的,以后接硬件直接删除,换成 `backend_hw.c` |
| `spi_virtual_slave.c` | **Dummy** | 代替不存在的芯片,以后有真实芯片时,对应逻辑挪到 Master 侧的 client driver(spi_device driver),而不是这个目录的继承者 |

## 虚拟寄存器堆协议

模拟对象类似 SPI EEPROM(25 系列)的命令格式:

- 寄存器空间:128 个,每个 1 字节(地址 `0x00`–`0x7F`)
- **命令字节**(每次 transfer 的第一个字节):
  - `bit7 = 1` → 读,`bit7 = 0` → 写
  - `bit6:0` → 起始寄存器地址
- 后续字节为数据,支持地址自增(burst read/write):
  - 写:`tx[1..]` 依次写入 `(addr+i) & 0x7F`
  - 读:`rx[1..]` 依次填入 `(addr+i) & 0x7F` 的值(`tx[1..]` 为 dummy 字节,SPI 全双工特性决定)

## 编译

```bash
./build_dummySPI.sh build   # 编译(默认),源码 rsync 镜像到 build/ 后 out-of-tree 编译
./build_dummySPI.sh clean   # 删除整个 build/ 目录
```

编译顺序固定为 `core → backend → spi_virtual_slave`,因为 backend 依赖 core 导出的符号(`svpi_backend_register`),slave 依赖 backend 导出的符号(`svpi_register_slave`),通过各自 Makefile 里的 `KBUILD_EXTRA_SYMBOLS` 指向上一级的 `Module.symvers`。

## 加载 / 卸载

```bash
./load_dummySPI.sh load     # insmod 三个模块(正确顺序) + 自动绑定 spidev 驱动
./load_dummySPI.sh unload   # rmmod 三个模块(反向顺序)
./load_dummySPI.sh reload   # unload + load,改完代码后最常用
./load_dummySPI.sh status   # 查看当前加载状态、/dev/spidev0.0、最近 dmesg
```

### 为什么需要手动绑定 spidev 驱动

较新内核已废弃通过裸 `modalias = "spidev"` 自动绑定的方式,必须使用官方支持的手动绑定流程,`load_dummySPI.sh` 已经自动处理:

```bash
echo spidev | sudo tee /sys/bus/spi/devices/spi0.0/driver_override
echo spi0.0 | sudo tee /sys/bus/spi/drivers/spidev/bind
```

## 用户态验证

```bash
gcc -o test/spi_test test/spi_test.c
sudo ./test/spi_test
```

`/dev/spidev0.0` 默认权限为 `root:root 0600`,非 root 用户直接运行会 `Permission denied`,需要 `sudo` 或自行配置 udev 规则放开权限。

预期输出:
```
Write done: wrote 0xAB 0xCD 0xEF starting at reg 0x05
Read back:  0xAB 0xCD 0xEF
PASS: register file matches what was written.
```

## 已知的内核版本相关问题(本项目在 6.x/7.x 较新内核上踩过的坑)

以下 API 变化在较老的参考资料里查不到,如果换了内核版本又编译失败,优先怀疑同类原因:

1. `spi_alloc_master()` → `spi_alloc_host()`:SPI 术语从 master/slave 改为 host/target
2. `spi->chip_select` 从 `u8` 变成 `u8 *`(支持多片选),取值需用 `spi_get_chipselect(spi, idx)`
3. `spidev` 驱动不再支持裸 `modalias = "spidev"` 自动绑定,需要手动 `driver_override` + `bind`

## 后续计划

- 编写用户态更完整的 CLI 测试工具(参照 `test/spi_test.c`,自行实现)
- `backend/backend_hw.c.template` 预留:以后接真实硬件时,实现 `do_transfer()` 操作真实寄存器/DMA,替换 Makefile 中的 backend 目标文件即可,`core/` 和 CLI 工具都不需要改动
- I2C(计划使用内核自带 `i2c-stub`)、UART(计划使用 `socat` 虚拟 pty 对)的练习项目
