// test/spi_test.c
// 编译: gcc -o spi_test spi_test.c
// 运行: sudo ./spi_test
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

#define SVPI_CMD_READ  (1 << 7)

static int spi_xfer(int fd, uint8_t *tx, uint8_t *rx, size_t len)
{
    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)tx,
        .rx_buf = (unsigned long)rx,
        .len    = len,
        .speed_hz = 1000000,
        .bits_per_word = 8,
    };
    return ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
}

int main(void)
{
    int fd = open("/dev/spidev0.0", O_RDWR);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    /* --- 写:往寄存器地址 0x05 开始,burst 写 3 个字节 --- */
    uint8_t tx_write[4] = { 0x05, 0xAB, 0xCD, 0xEF };
    uint8_t rx_write[4] = { 0 };

    if (spi_xfer(fd, tx_write, rx_write, sizeof(tx_write)) < 0) {
        perror("write xfer");
        close(fd);
        return 1;
    }
    printf("Write done: wrote 0xAB 0xCD 0xEF starting at reg 0x05\n");

    /* --- 读:从寄存器地址 0x05 开始,burst 读 3 个字节回来验证 --- */
    uint8_t tx_read[4]  = { 0x05 | SVPI_CMD_READ, 0x00, 0x00, 0x00 };
    uint8_t rx_read[4]  = { 0 };

    if (spi_xfer(fd, tx_read, rx_read, sizeof(tx_read)) < 0) {
        perror("read xfer");
        close(fd);
        return 1;
    }

    printf("Read back:  0x%02X 0x%02X 0x%02X\n",
           rx_read[1], rx_read[2], rx_read[3]);

    if (rx_read[1] == 0xAB && rx_read[2] == 0xCD && rx_read[3] == 0xEF)
        printf("PASS: register file matches what was written.\n");
    else
        printf("FAIL: mismatch!\n");

    close(fd);
    return 0;
}