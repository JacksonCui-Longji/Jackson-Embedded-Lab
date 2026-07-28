#ifndef I2C_APP_H
#define I2C_APP_H

#include <stdint.h>
#include <stddef.h>

#define I2C_MAX_LEN   (128 + 1)   /* 1 个寄存器地址字节 + 最多 128 个数据字节,跟 SPI 那边保持一致方便对比 */

typedef void (*I2cAppCallback)(uint8_t ret, uint8_t *rx_data, size_t len, void *user_ctx);

extern uint8_t u8InitI2cApp(I2cAppCallback callback);
extern uint8_t u8DeinitI2cApp(void);
extern uint8_t wu8I2cAppSendMessage(uint8_t *tx_data, size_t tx_len, void *user_ctx);
extern uint8_t wu8I2cAppReadMessage(uint8_t *tx_data, size_t tx_len, void *user_ctx);

#endif // I2C_APP_H