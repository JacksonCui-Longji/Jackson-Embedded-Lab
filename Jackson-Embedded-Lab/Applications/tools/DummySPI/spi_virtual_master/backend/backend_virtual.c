#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/string.h>
#include <linux/spi/spi.h>
#include "svpi_backend.h"
#include "svpi_bus.h"

static struct {
    struct svpi_slave_ops *ops;
    void *priv;
} slots[SVPI_MAX_CS];

static DEFINE_SPINLOCK(slots_lock);

int svpi_register_slave(unsigned int cs, struct svpi_slave_ops *ops, void *priv)
{
    unsigned long flags;

    if (cs >= SVPI_MAX_CS)
        return -EINVAL;

    spin_lock_irqsave(&slots_lock, flags);
    if (slots[cs].ops) {
        spin_unlock_irqrestore(&slots_lock, flags);
        return -EBUSY;
    }
    slots[cs].ops  = ops;
    slots[cs].priv = priv;
    spin_unlock_irqrestore(&slots_lock, flags);
    return 0;
}
EXPORT_SYMBOL(svpi_register_slave);

void svpi_unregister_slave(unsigned int cs)
{
    unsigned long flags;

    if (cs >= SVPI_MAX_CS)
        return;

    spin_lock_irqsave(&slots_lock, flags);
    slots[cs].ops  = NULL;
    slots[cs].priv = NULL;
    spin_unlock_irqrestore(&slots_lock, flags);
}
EXPORT_SYMBOL(svpi_unregister_slave);

static int svpi_virtual_do_transfer(struct spi_device *spi, const u8 *tx, u8 *rx, unsigned int len)
{
    unsigned int cs = spi_get_chipselect(spi, 0);
    struct svpi_slave_ops *ops;
    void *priv;
    unsigned long flags;

    if (cs >= SVPI_MAX_CS)
        return -EINVAL;

    spin_lock_irqsave(&slots_lock, flags);
    ops  = slots[cs].ops;
    priv = slots[cs].priv;
    spin_unlock_irqrestore(&slots_lock, flags);

    if (!ops || !ops->xfer) {
        /* 这个 cs 上没挂从设备,模拟总线上没人应答,MISO 悬空拉高 */
        memset(rx, 0xFF, len);
        return 0;
    }

    return ops->xfer(priv, tx, rx, len);
}

static const struct svpi_backend_ops virtual_backend_ops = {
    .do_transfer = svpi_virtual_do_transfer,
};

static int __init backend_virtual_init(void)
{
    svpi_backend_register(&virtual_backend_ops);
    pr_info("svpi: virtual backend registered\n");
    return 0;
}

static void __exit backend_virtual_exit(void)
{
    pr_info("svpi: virtual backend removed\n");
}

module_init(backend_virtual_init);
module_exit(backend_virtual_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Virtual SPI bus backend - dummy transport, no physical wire");