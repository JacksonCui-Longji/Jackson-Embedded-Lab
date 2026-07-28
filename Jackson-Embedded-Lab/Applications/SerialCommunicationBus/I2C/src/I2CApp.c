#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <stdint.h>
#include "CommonType.h"
#include "I2CApp.h"

#define I2C_TARGET_ADDR     0x50U   /* 对应 i2c-stub chip_addr=0x50 */
#define I2C_REQ_QUEUE_DEPTH 8U

typedef enum
{
    I2C_REQ_SEND = 0,
    I2C_REQ_READ
} I2cReqType_t;

typedef struct
{
    I2cReqType_t type;
    uint8_t      tx_data[I2C_MAX_LEN];
    size_t       len;
    void        *user_ctx;
} I2cRequest_t;

static int               g_fd = -1;
static pthread_t         g_worker_thread;
static uint8_t           g_initialized = 0U;
static I2cAppCallback    g_callback = NULL;
static volatile uint8_t  g_thread_stop = 0U;

static I2cRequest_t      g_req_queue[I2C_REQ_QUEUE_DEPTH];
static size_t            g_queue_head  = 0U;
static size_t            g_queue_tail  = 0U;
static size_t            g_queue_count = 0U;
static pthread_mutex_t   g_queue_lock  = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t   g_init_lock   = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t    g_queue_cond  = PTHREAD_COND_INITIALIZER;

/* ---- SMBus Byte Data 底层封装,不依赖 libi2c-dev,直接用裸 ioctl ---- */

static int i2c_smbus_write_byte_data_raw(int fd, uint8_t reg, uint8_t value)
{
    union i2c_smbus_data data;
    struct i2c_smbus_ioctl_data args;

    data.byte = value;
    args.read_write = I2C_SMBUS_WRITE;
    args.command     = reg;
    args.size        = I2C_SMBUS_BYTE_DATA;
    args.data        = &data;

    return ioctl(fd, I2C_SMBUS, &args);
}

static int i2c_smbus_read_byte_data_raw(int fd, uint8_t reg, uint8_t *value)
{
    union i2c_smbus_data data;
    struct i2c_smbus_ioctl_data args;
    int ret;

    args.read_write = I2C_SMBUS_READ;
    args.command     = reg;
    args.size        = I2C_SMBUS_BYTE_DATA;
    args.data        = &data;

    ret = ioctl(fd, I2C_SMBUS, &args);
    if (ret >= 0)
    {
        *value = data.byte;
    }
    return ret;
}

/* tx_data[0] = 起始寄存器地址,tx_data[1..len-1] = 待写数据,逐字节地址自增 */
static uint8_t u8I2cAppSendMessage(uint8_t *tx_data, uint8_t *rx_data, size_t tx_rx_len)
{
    uint8_t addr;
    size_t  i;

    if (0 > g_fd)
    {
        printf("Please use u8InitI2cApp first\n");
        return RET_NG;
    }
    if ((tx_data == NULL) || (rx_data == NULL) || (0U == tx_rx_len))
    {
        printf("input buffer invalid.\n");
        return RET_NG;
    }

    addr = tx_data[0];
    rx_data[0] = 0xFF;   /* 占位,跟 SPI 那边"命令字节期间 MISO 无意义"的约定保持一致,方便对比 */

    for (i = 1U; i < tx_rx_len; i++)
    {
        if (i2c_smbus_write_byte_data_raw(g_fd, addr, tx_data[i]) < 0)
        {
            return RET_NG;
        }
        rx_data[i] = 0xFF;
        addr++;
    }

    return RET_OK;
}

/* tx_data[0] = 起始寄存器地址; rx_data[1..len-1] = 读回的数据,逐字节地址自增 */
static uint8_t u8I2cAppReadMessage(uint8_t *tx_data, uint8_t *rx_data, size_t tx_rx_len)
{
    uint8_t addr;
    size_t  i;

    if (0 > g_fd)
    {
        printf("Please use u8InitI2cApp first\n");
        return RET_NG;
    }
    if ((tx_data == NULL) || (rx_data == NULL) || (0U == tx_rx_len))
    {
        printf("input buffer invalid.\n");
        return RET_NG;
    }

    addr = tx_data[0];
    rx_data[0] = 0xFF;

    for (i = 1U; i < tx_rx_len; i++)
    {
        if (i2c_smbus_read_byte_data_raw(g_fd, addr, &rx_data[i]) < 0)
        {
            return RET_NG;
        }
        addr++;
    }

    return RET_OK;
}

/* ---- 队列相关(跟 SpiApp.c 完全一致,没有改动) ---- */

static uint8_t u8I2cAppEnqueue(I2cReqType_t type, uint8_t *tx_data, size_t tx_len, void *user_ctx)
{
    if ((tx_data == NULL) || (0U == tx_len) || (tx_len > I2C_MAX_LEN))
    {
        printf("input buffer invalid.\n");
        return RET_NG;
    }

    pthread_mutex_lock(&g_queue_lock);

    if (g_queue_count >= I2C_REQ_QUEUE_DEPTH)
    {
        pthread_mutex_unlock(&g_queue_lock);
        printf("request queue full.\n");
        return RET_NG;
    }

    I2cRequest_t *slot = &g_req_queue[g_queue_tail];
    slot->type     = type;
    slot->len      = tx_len;
    slot->user_ctx = user_ctx;
    memset(slot->tx_data, 0, sizeof(slot->tx_data));
    memcpy(slot->tx_data, tx_data, tx_len);

    g_queue_tail = (g_queue_tail + 1U) % I2C_REQ_QUEUE_DEPTH;
    g_queue_count++;

    pthread_cond_signal(&g_queue_cond);
    pthread_mutex_unlock(&g_queue_lock);

    return RET_OK;
}

static void *I2cAppWorkerThread(void *arg)
{
    (void)arg;

    for (;;)
    {
        I2cRequest_t req;
        uint8_t      rx_data[I2C_MAX_LEN];
        uint8_t      ret;

        pthread_mutex_lock(&g_queue_lock);
        while ((0U == g_queue_count) && (0U == g_thread_stop))
        {
            pthread_cond_wait(&g_queue_cond, &g_queue_lock);
        }

        if ((0U == g_queue_count) && (0U != g_thread_stop))
        {
            pthread_mutex_unlock(&g_queue_lock);
            break;
        }

        req = g_req_queue[g_queue_head];
        g_queue_head = (g_queue_head + 1U) % I2C_REQ_QUEUE_DEPTH;
        g_queue_count--;
        pthread_mutex_unlock(&g_queue_lock);

        memset(rx_data, 0, sizeof(rx_data));

        if (I2C_REQ_READ == req.type)
        {
            ret = u8I2cAppReadMessage(req.tx_data, rx_data, req.len);
        }
        else
        {
            ret = u8I2cAppSendMessage(req.tx_data, rx_data, req.len);
        }

        if (g_callback != NULL)
        {
            g_callback(ret, rx_data, req.len, req.user_ctx);
        }
    }

    return NULL;
}

/* ---- 对外接口 ---- */

uint8_t wu8I2cAppSendMessage(uint8_t *tx_data, size_t tx_len, void *user_ctx)
{
    if (0 > g_fd)
    {
        printf("Please use u8InitI2cApp first\n");
        return RET_NG;
    }
    return u8I2cAppEnqueue(I2C_REQ_SEND, tx_data, tx_len, user_ctx);
}

uint8_t wu8I2cAppReadMessage(uint8_t *tx_data, size_t tx_len, void *user_ctx)
{
    if (0 > g_fd)
    {
        printf("Please use u8InitI2cApp first\n");
        return RET_NG;
    }
    return u8I2cAppEnqueue(I2C_REQ_READ, tx_data, tx_len, user_ctx);
}

uint8_t u8InitI2cApp(I2cAppCallback callback)
{
    pthread_mutex_lock(&g_init_lock);
    if (g_initialized)
    {
        pthread_mutex_unlock(&g_init_lock);
        printf("I2cApp already initialized.\n");
        return RET_NG;
    }

    g_fd = open("/dev/i2c-0", O_RDWR);
    if (0 > g_fd)
    {
        perror("open");
        pthread_mutex_unlock(&g_init_lock);
        return RET_NG;
    }

    /* SMBus 这套 ioctl 不像 SPI 每次传输都带地址,而是先绑定目标从设备地址到这个 fd 上,
       之后这个 fd 上所有读写默认都是对 I2C_TARGET_ADDR 这个从设备操作 */
    if (ioctl(g_fd, I2C_SLAVE, I2C_TARGET_ADDR) < 0)
    {
        perror("ioctl I2C_SLAVE");
        close(g_fd);
        g_fd = -1;
        pthread_mutex_unlock(&g_init_lock);
        return RET_NG;
    }

    g_callback    = callback;
    g_thread_stop = 0U;
    g_queue_head  = 0U;
    g_queue_tail  = 0U;
    g_queue_count = 0U;

    if (0 != pthread_create(&g_worker_thread, NULL, I2cAppWorkerThread, NULL))
    {
        perror("pthread_create");
        close(g_fd);
        g_fd = -1;
        pthread_mutex_unlock(&g_init_lock);
        return RET_NG;
    }

    g_initialized = 1U;
    pthread_mutex_unlock(&g_init_lock);
    return RET_OK;
}

uint8_t u8DeinitI2cApp(void)
{
    pthread_mutex_lock(&g_init_lock);
    if (!g_initialized)
    {
        pthread_mutex_unlock(&g_init_lock);
        return RET_NG;
    }

    pthread_mutex_lock(&g_queue_lock);
}