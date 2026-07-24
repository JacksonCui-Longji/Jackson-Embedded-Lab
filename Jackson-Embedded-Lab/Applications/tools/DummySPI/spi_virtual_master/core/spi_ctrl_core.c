// core/spi_ctrl_core.c
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/spi/spi.h>
#include <linux/slab.h>
#include <linux/err.h>
#include "svpi_backend.h"

#define SVPI_BUS_NUM   0
#define SVPI_NUM_CS    4   /* 真实硬件里这个数字来自控制器手册,这里我们自己定 */

static struct platform_device *svpi_pdev;
static struct spi_controller  *svpi_ctlr;
static const struct svpi_backend_ops *backend_ops;

/* backend 模块在自己的 init 里调用这个,把自己"插"给 core。
 * core 完全不知道对方是虚拟总线还是真硬件,只认这个函数指针表。 */
void svpi_backend_register(const struct svpi_backend_ops *ops)
{
    backend_ops = ops;
}
EXPORT_SYMBOL(svpi_backend_register);

/* 真实硬件驱动这里应该是:配置 DMA 描述符/写 TX FIFO/踢一次传输/等中断或轮询状态位。
 * 我们这里直接转发给 backend,backend 是谁由运行时注册决定,core 本身不关心。 */
static int svpi_transfer_one_message(struct spi_controller *ctlr, struct spi_message *msg)
{
    struct spi_device *spi = msg->spi;
    struct spi_transfer *xfer;
    int ret = 0;

    if (!backend_ops || !backend_ops->do_transfer) {
        ret = -ENODEV;
        goto out;
    }

    list_for_each_entry(xfer, &msg->transfers, transfer_list) {
        u8 *tx_tmp = NULL, *rx_tmp = NULL;
        const u8 *tx_buf = xfer->tx_buf;
        u8 *rx_buf = xfer->rx_buf;

        /* SPI 全双工特性:即使调用方只关心一侧,电气上两条线都在动,
         * 所以 tx/rx 任一为 NULL 时都要垫一个临时 buffer 传给 backend */
        if (!tx_buf) {
            tx_tmp = kzalloc(xfer->len, GFP_KERNEL);
            if (!tx_tmp) { ret = -ENOMEM; goto out; }
            tx_buf = tx_tmp;
        }
        if (!rx_buf) {
            rx_tmp = kzalloc(xfer->len, GFP_KERNEL);
            if (!rx_tmp) { kfree(tx_tmp); ret = -ENOMEM; goto out; }
            rx_buf = rx_tmp;
        }

        ret = backend_ops->do_transfer(spi, tx_buf, rx_buf, xfer->len);

        kfree(tx_tmp);
        kfree(rx_tmp);

        if (ret)
            goto out;

        msg->actual_length += xfer->len;
    }

out:
    msg->status = ret;
    spi_finalize_current_message(ctlr);
    return ret;
}

/* 真实驱动这里要把 bits_per_word/mode/speed 落地到硬件寄存器,
 * 我们只做参数合法性校验,这个校验动作本身是真实的 */
static int svpi_setup(struct spi_device *spi)
{
    if (spi->bits_per_word && spi->bits_per_word != 8)
        return -EINVAL;
    return 0;
}

static int __init spi_ctrl_core_init(void)
{
    struct spi_board_info board_info = {
        .modalias     = "spidev",
        .max_speed_hz = 1000000,
        .bus_num      = SVPI_BUS_NUM,
        .chip_select  = 0,
        .mode         = SPI_MODE_0,
    };
    int ret;

    /* spi_alloc_master() 需要一个 parent device,真实硬件这里是
     * platform_device/SoC 自带的那个 device,我们造一个假的 platform_device 顶上 */
    svpi_pdev = platform_device_register_simple("svpi_ctrl", -1, NULL, 0);
    if (IS_ERR(svpi_pdev))
        return PTR_ERR(svpi_pdev);

    svpi_ctlr = spi_alloc_host(&svpi_pdev->dev, 0);
    if (!svpi_ctlr) {
        ret = -ENOMEM;
        goto err_pdev;
    }

    svpi_ctlr->bus_num            = SVPI_BUS_NUM;
    svpi_ctlr->num_chipselect     = SVPI_NUM_CS;
    svpi_ctlr->mode_bits          = SPI_MODE_0 | SPI_MODE_1 | SPI_MODE_2 | SPI_MODE_3;
    svpi_ctlr->bits_per_word_mask = SPI_BPW_MASK(8);
    svpi_ctlr->setup              = svpi_setup;
    svpi_ctlr->transfer_one_message = svpi_transfer_one_message;

    ret = spi_register_controller(svpi_ctlr);
    if (ret)
        goto err_ctlr;

    if (!spi_new_device(svpi_ctlr, &board_info)) {
        ret = -ENODEV;
        goto err_new_device;
    }

    pr_info("svpi_core: virtual SPI controller registered on bus %d\n", SVPI_BUS_NUM);
    return 0;

err_new_device:
    spi_unregister_controller(svpi_ctlr);
    goto err_pdev;
err_ctlr:
    spi_controller_put(svpi_ctlr);
err_pdev:
    platform_device_unregister(svpi_pdev);
    return ret;
}

static void __exit spi_ctrl_core_exit(void)
{
    if (svpi_ctlr)
        spi_unregister_controller(svpi_ctlr); /* 会顺带清掉 spi_new_device 挂的 spidev 设备 */
    platform_device_unregister(svpi_pdev);
    pr_info("svpi_core: removed\n");
}

module_init(spi_ctrl_core_init);
module_exit(spi_ctrl_core_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Virtual SPI controller core - hardware-agnostic skeleton");