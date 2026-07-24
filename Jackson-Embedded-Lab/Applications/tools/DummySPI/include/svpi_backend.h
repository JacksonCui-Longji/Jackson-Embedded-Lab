#ifndef _SVPI_BACKEND_H_
#define _SVPI_BACKEND_H_

struct svpi_backend_ops {
    /* 不管底层是虚拟总线还是真寄存器,core 只认这一个函数 */
    int (*do_transfer)(struct spi_device *spi, const u8 *tx, u8 *rx, unsigned int len);
};

/* backend 模块在自己的 module_init 里调用,把自己"插"给 core */
void svpi_backend_register(const struct svpi_backend_ops *ops);

#endif