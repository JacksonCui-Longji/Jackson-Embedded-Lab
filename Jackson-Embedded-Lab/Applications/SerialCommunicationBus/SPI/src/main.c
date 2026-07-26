#include "SpiApp.h"
#include <semaphore.h>
#include "string.h"
#include "CommonType.h"
#include <time.h>

typedef struct
{
    sem_t   done;         /* 谁的请求,就等谁的这个信号量 */
    uint8_t ret;
    uint8_t rx_data[SPI_MAX_LEN];
    size_t  len;
} SpiRequestCtx_t;

void MyCallback(uint8_t ret, uint8_t *rx_data, size_t len, void *user_ctx)
{
    SpiRequestCtx_t *ctx = (SpiRequestCtx_t *)user_ctx;

    ctx->ret = ret;
    ctx->len = len;
    memcpy(ctx->rx_data, rx_data, len);

    sem_post(&ctx->done);   /* 只唤醒等在这个特定信箱上的那个线程 */
}

void SendRegThread(void)
{
    SpiRequestCtx_t ctx;
    sem_init(&ctx.done, 0, 0);

    uint8_t tx[4] = {0x01, 0xab, 0xcd, 0xef};

    if (RET_OK != wu8SpiAppSendMessage(tx, sizeof(tx), &ctx))
    {
        printf("SendRegThread: enqueue failed, skip wait.\n");
        sem_destroy(&ctx.done);
        return;                      // 没入队成功,压根不会有人 sem_post,直接放弃等待
    }

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += 2;   /* 最多等 2 秒 */

    if (0 != sem_timedwait(&ctx.done, &ts))
    {
        printf("SendRegThread: taking longer than expected, still waiting...\n");
        sem_wait(&ctx.done);   /* 请求已经入队,一定会被处理,这里必须死等到它完成 */
    }

    printf("SendRegThread ret=%d, data[1]=0x%02X\n", ctx.ret, ctx.rx_data[1]);
    sem_destroy(&ctx.done);
    return;
}

void ReadRegThread(void)
{
    SpiRequestCtx_t ctx;
    sem_init(&ctx.done, 0, 0);

    uint8_t tx[4] = {0x01, 0, 0, 0};

    if (RET_OK != wu8SpiAppReadMessage(tx, sizeof(tx), &ctx))
    {
        printf("ReadRegThread: enqueue failed, skip wait.\n");
        sem_destroy(&ctx.done);
        return;
    }

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += 2;   /* 最多等 2 秒 */

    if (0 != sem_timedwait(&ctx.done, &ts))
    {
        printf("ReadRegThread: taking longer than expected, still waiting...\n");
        sem_wait(&ctx.done);   /* 请求已经入队,一定会被处理,这里必须死等到它完成 */
    }

    printf("ReadRegThread ret=%d, data[1]=0x%02X\n", ctx.ret, ctx.rx_data[1]);
    sem_destroy(&ctx.done);
    return;
}

int main()
{

    u8InitSpiApp(MyCallback);

    SendRegThread();

    ReadRegThread();
    
    u8DeinitSpiApp();

    return 0;
}