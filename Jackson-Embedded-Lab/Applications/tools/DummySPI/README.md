Jackson-Embedded-Lab/
└── SPI/
    ├── include/
    │   ├── svpi_backend.h      # ★核心:core 和 backend 之间的接口契约(真实,长期不变)
    │   └── svpi_bus.h          # 虚拟总线专用(slave 注册那一套),纯 dummy
    │
    ├── spi_virtual_master/
    │   ├── Makefile
    │   ├── core/
    │   │   └── spi_ctrl_core.c        # 【REAL】controller 注册、transfer_one_message 外层骨架、spidev 挂载
    │   └── backend/
    │       ├── backend_virtual.c      # 【DUMMY】走 svpi_register_slave 那套虚拟总线
    │       └── backend_hw.c.template  # 【预留】以后接真硬件时,把这个改名成 .c 实现
    │
    └── spi_virtual_slave/
        ├── Makefile
        └── spi_virtual_slave.c        # 【整体 DUMMY】代替不存在的芯片