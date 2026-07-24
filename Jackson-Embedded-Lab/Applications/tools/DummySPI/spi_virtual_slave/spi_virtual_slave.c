// spi_virtual_slave/spi_virtual_slave.c
#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/string.h>
#include <linux/types.h>
#include "svpi_bus.h"

#define SVPI_REG_COUNT   128
#define SVPI_REG_MASK    (SVPI_REG_COUNT - 1)   /* 0x7F */
#define SVPI_CMD_READ    (1 << 7)

#define SVPI_SLAVE_CS    0   /* 对应 master 侧 spi_new_device() 挂的 cs 号 */

struct svpi_slave_dev {
    u8 regs[SVPI_REG_COUNT];
    spinlock_t lock;
};

static struct svpi_slave_dev slave_dev;

/*
 * tx[0] = 命令字节: bit7 = 读(1)/写(0), bit6:0 = 起始寄存器地址
 * tx[1..len-1] = 写数据(写命令) 或 dummy(读命令)
 * rx[1..len-1] = 读命令时填入寄存器值(burst,地址自增); 写命令时无意义
 */
static int svpi_slave_xfer(void *priv, const u8 *tx, u8 *rx, unsigned int len)
{
    struct svpi_slave_dev *dev = priv;
    unsigned long flags;
    u8 cmd, addr;
    unsigned int i;

    if (len == 0)
        return 0;

    cmd  = tx[0];
    addr = cmd & SVPI_REG_MASK;

    spin_lock_irqsave(&dev->lock, flags);

    rx[0] = 0xFF; /* 命令字节传输期间 MISO 无有效数据,和真实 EEPROM 行为一致 */

    if (cmd & SVPI_CMD_READ) {
        for (i = 1; i < len; i++) {
            rx[i] = dev->regs[addr];
            addr = (addr + 1) & SVPI_REG_MASK;
        }
    } else {
        for (i = 1; i < len; i++) {
            dev->regs[addr] = tx[i];
            rx[i] = 0xFF;
            addr = (addr + 1) & SVPI_REG_MASK;
        }
    }

    spin_unlock_irqrestore(&dev->lock, flags);
    return 0;
}

static struct svpi_slave_ops slave_ops = {
    .xfer = svpi_slave_xfer,
};

static int __init spi_virtual_slave_init(void)
{
    int ret;

    spin_lock_init(&slave_dev.lock);
    memset(slave_dev.regs, 0, sizeof(slave_dev.regs));

    ret = svpi_register_slave(SVPI_SLAVE_CS, &slave_ops, &slave_dev);
    if (ret) {
        pr_err("svpi_slave: register failed on cs=%d (%d)\n", SVPI_SLAVE_CS, ret);
        return ret;
    }

    pr_info("svpi_slave: virtual register-file chip registered on cs=%d\n", SVPI_SLAVE_CS);
    return 0;
}

static void __exit spi_virtual_slave_exit(void)
{
    svpi_unregister_slave(SVPI_SLAVE_CS);
    pr_info("svpi_slave: removed\n");
}

module_init(spi_virtual_slave_init);
module_exit(spi_virtual_slave_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Virtual SPI slave - emulated register-file chip (EEPROM-like)");