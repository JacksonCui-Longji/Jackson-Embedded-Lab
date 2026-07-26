#ifndef SPI_APP_H
#define SPI_APP_H

#include <stdint.h>
#include <stdio.h>

typedef void (*SpiAppCallback)(uint8_t ret, uint8_t *rx_data, size_t len, void *user_ctx);

extern uint8_t u8InitSpiApp(SpiAppCallback callback);
extern uint8_t u8DeinitSpiApp(void);
extern uint8_t wu8SpiAppSendMessage(uint8_t *tx_data, size_t tx_len, void *user_ctx);
extern uint8_t wu8SpiAppReadMessage(uint8_t *tx_data, size_t tx_len, void *user_ctx);

#endif // SPI_APP_H