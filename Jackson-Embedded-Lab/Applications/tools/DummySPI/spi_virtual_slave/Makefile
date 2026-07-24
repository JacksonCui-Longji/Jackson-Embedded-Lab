KDIR ?= /lib/modules/$(shell uname -r)/build
PWD  := $(shell pwd)

obj-m += spi_virtual_slave.o
ccflags-y += -I$(src)/../include

# 这次依赖的是 backend_virtual 导出的 svpi_register_slave
KBUILD_EXTRA_SYMBOLS := $(PWD)/../spi_virtual_master/backend/Module.symvers

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean