KDIR ?= /lib/modules/$(shell uname -r)/build
PWD  := $(shell pwd)

obj-m += backend_virtual.o
ccflags-y += -I$(src)/../../include

# 让 modpost 认识 core 模块导出的符号(svpi_backend_register),
# 否则编译时会报 "Unknown symbol" warning(不是 error,但最好消掉)
KBUILD_EXTRA_SYMBOLS := $(PWD)/../core/Module.symvers

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean