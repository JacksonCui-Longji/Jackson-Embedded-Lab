#ifndef _SVPI_BUS_H_
#define _SVPI_BUS_H_

#define SVPI_MAX_CS   4

struct svpi_slave_ops {
    int (*xfer)(void *priv, const u8 *tx, u8 *rx, unsigned int len);
};

int  svpi_register_slave(unsigned int cs, struct svpi_slave_ops *ops, void *priv);
void svpi_unregister_slave(unsigned int cs);

#endif