#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <stdint.h>
#include "CommonType.h"
#include "SpiApp.h"

#define SVPI_READ_MASK      (1 << 7)
#define SPI_MAX_LEN         (128 + 1)   /* 1 命令字节 + 最多 128 个寄存器 */
#define SPI_REQ_QUEUE_DEPTH 8U

typedef enum
{
    SPI_REQ_SEND = 0,
    SPI_REQ_READ
} SpiReqType_t;

typedef struct
{
    SpiReqType_t type;
    uint8_t      tx_data[SPI_MAX_LEN];
    size_t       len;
    void        *user_ctx;
} SpiRequest_t;

static int             g_fd = -1;
static pthread_t       g_worker_thread;
static SpiAppCallback  g_callback = NULL;
static volatile uint8_t g_thread_stop = 0U;

static SpiRequest_t     g_req_queue[SPI_REQ_QUEUE_DEPTH];
static size_t           g_queue_head  = 0U;   /* 下一个要出队的位置 */
static size_t           g_queue_tail  = 0U;   /* 下一个空闲入队位置 */
static size_t           g_queue_count = 0U;
static pthread_mutex_t  g_queue_lock  = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t   g_queue_cond  = PTHREAD_COND_INITIALIZER;

static uint8_t u8SpiAppxfer(int fd, uint8_t *tx, uint8_t *rx, size_t tx_rx_len)
{
    int ret = 0;
    struct spi_ioc_transfer tr = {
        .tx_buf        = (unsigned long)tx,
        .rx_buf        = (unsigned long)rx,
        .len           = tx_rx_len,
        .speed_hz      = 1000000,
        .bits_per_word = 8,
    };

    ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
    return (ret >= 0) ? RET_OK : RET_NG;
}

/* 保留不变:worker 线程内部会直接调用这两个函数 */
static uint8_t u8SpiAppSendMessage(uint8_t *tx_data, uint8_t *rx_data, size_t tx_rx_len)
{
    uint8_t ret = RET_NG;

    if (0 > g_fd)
    {
        printf("Please use u8InitSpiApp first\n");
        return RET_NG;
    }
    if ((tx_data == NULL) || (rx_data == NULL) || (0U == tx_rx_len))
    {
        printf("input buffer invalid.\n");
        return RET_NG;
    }
    ret = u8SpiAppxfer(g_fd, tx_data, rx_data, tx_rx_len);
    return ret;
}

static uint8_t u8SpiAppReadMessage(uint8_t *tx_data, uint8_t *rx_data, size_t tx_rx_len)
{
    uint8_t ret = RET_NG;

    if (0 > g_fd)
    {
        printf("Please use u8InitSpiApp first\n");
        return RET_NG;
    }
    if ((tx_data == NULL) || (rx_data == NULL) || (0U == tx_rx_len))
    {
        printf("input buffer invalid.\n");
        return RET_NG;
    }
    tx_data[0] |= SVPI_READ_MASK;
    ret = u8SpiAppxfer(g_fd, tx_data, rx_data, tx_rx_len);
    return ret;
}

/* ---- 队列相关 ---- */

static uint8_t u8SpiAppEnqueue(SpiReqType_t type, uint8_t *tx_data, size_t tx_len, void *user_ctx)
{
    uint8_t ret = RET_NG;

    if ((tx_data == NULL) || (0U == tx_len) || (tx_len > SPI_MAX_LEN))
    {
        printf("input buffer invalid.\n");
        return RET_NG;
    }

    pthread_mutex_lock(&g_queue_lock);

    if (g_queue_count >= SPI_REQ_QUEUE_DEPTH)
    {
        pthread_mutex_unlock(&g_queue_lock);
        printf("request queue full.\n");
        return RET_NG;
    }

    SpiRequest_t *slot = &g_req_queue[g_queue_tail];
    slot->type     = type;
    slot->len      = tx_len;
    slot->user_ctx = user_ctx;
    memset(slot->tx_data, 0, sizeof(slot->tx_data));
    memcpy(slot->tx_data, tx_data, tx_len);

    g_queue_tail = (g_queue_tail + 1U) % SPI_REQ_QUEUE_DEPTH;
    g_queue_count++;

    pthread_cond_signal(&g_queue_cond);
    pthread_mutex_unlock(&g_queue_lock);

    ret = RET_OK;
    return ret;
}

static void *SpiAppWorkerThread(void *arg)
{
    (void)arg;

    for (;;)
    {
        SpiRequest_t req;
        uint8_t      rx_data[SPI_MAX_LEN];
        uint8_t      ret;

        pthread_mutex_lock(&g_queue_lock);
        while ((0U == g_queue_count) && (0U == g_thread_stop))
        {
            pthread_cond_wait(&g_queue_cond, &g_queue_lock);
        }

        if ((0U == g_queue_count) && (0U != g_thread_stop))
        {
            /* 队列已清空,且收到停止信号,退出线程 */
            pthread_mutex_unlock(&g_queue_lock);
            break;
        }

        req = g_req_queue[g_queue_head];
        g_queue_head = (g_queue_head + 1U) % SPI_REQ_QUEUE_DEPTH;
        g_queue_count--;
        pthread_mutex_unlock(&g_queue_lock);

        memset(rx_data, 0, sizeof(rx_data));

        if (SPI_REQ_READ == req.type)
        {
            ret = u8SpiAppReadMessage(req.tx_data, rx_data, req.len);
        }
        else
        {
            ret = u8SpiAppSendMessage(req.tx_data, rx_data, req.len);
        }

        if (g_callback != NULL)
        {
            g_callback(ret, rx_data, req.len, req.user_ctx);
        }
    }

    return NULL;
}

/* ---- 对外接口 ---- */

uint8_t wu8SpiAppSendMessage(uint8_t *tx_data, size_t tx_len, void *user_ctx)
{
    if (0 > g_fd)
    {
        printf("Please use u8InitSpiApp first\n");
        return RET_NG;
    }
    return u8SpiAppEnqueue(SPI_REQ_SEND, tx_data, tx_len, user_ctx);
}

uint8_t wu8SpiAppReadMessage(uint8_t *tx_data, size_t tx_len, void *user_ctx)
{
    if (0 > g_fd)
    {
        printf("Please use u8InitSpiApp first\n");
        return RET_NG;
    }
    return u8SpiAppEnqueue(SPI_REQ_READ, tx_data, tx_len, user_ctx);
}

uint8_t u8InitSpiApp(SpiAppCallback callback)
{
    g_fd = open("/dev/spidev0.0", O_RDWR);
    if (0 > g_fd)
    {
        perror("open");
        return RET_NG;
    }

    g_callback    = callback;
    g_thread_stop = 0U;
    g_queue_head  = 0U;
    g_queue_tail  = 0U;
    g_queue_count = 0U;

    if (0 != pthread_create(&g_worker_thread, NULL, SpiAppWorkerThread, NULL))
    {
        perror("pthread_create");
        close(g_fd);
        g_fd = -1;
        return RET_NG;
    }

    return RET_OK;
}

uint8_t u8DeinitSpiApp(void)
{
    pthread_mutex_lock(&g_queue_lock);
    g_thread_stop = 1U;
    pthread_cond_signal(&g_queue_cond);
    pthread_mutex_unlock(&g_queue_lock);

    pthread_join(g_worker_thread, NULL);

    close(g_fd);
    g_fd = -1;
    g_callback = NULL;

    return RET_OK;
}