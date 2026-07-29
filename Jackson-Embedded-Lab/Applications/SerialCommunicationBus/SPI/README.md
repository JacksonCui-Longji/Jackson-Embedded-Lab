# SpiApp

用户态 SPI 通信库,基于 `/dev/spidev0.0` 封装,内部用 pthread worker 线程 + 固定深度请求队列实现异步收发,调用者通过回调拿结果。

## 前置依赖:必须先启动 DummySPI 虚拟外设

`SpiApp` 本身不模拟任何硬件,它只是一个标准的 spidev 用户态客户端——**运行前必须先把 `Applications/tools/DummySPI` 里的虚拟 SPI 总线(内核模块)编译并加载起来**,`/dev/spidev0.0` 这个设备节点才会存在,否则 `u8InitSpiApp()` 会在 `open()` 那一步直接失败。

```bash
cd Applications/tools/DummySPI

# 1. 编译三个内核模块(core -> backend -> slave),产物集中在 build/ 目录
./build_dummySPI.sh build

# 2. 加载模块(自动按正确顺序 insmod,并完成 spidev 驱动绑定)
./load_dummySPI.sh load

# 3. 确认 /dev/spidev0.0 已经出现
./load_dummySPI.sh status
```

看到 `status` 输出里 `/dev/spidev0.0` 存在,再继续下一步。用完之后如果要卸载:

```bash
./load_dummySPI.sh unload
```

详细的架构说明、协议设计、内核版本相关的坑,见 `DummySPI/README.md`,这里不重复。

## 使用 SpiApp

### 头文件接口(`SpiApp.h`)

```c
typedef void (*SpiAppCallback)(uint8_t ret, uint8_t *rx_data, size_t len, void *user_ctx);

uint8_t u8InitSpiApp(SpiAppCallback callback);
uint8_t u8DeinitSpiApp(void);
uint8_t wu8SpiAppSendMessage(uint8_t *tx_data, size_t tx_len, void *user_ctx);
uint8_t wu8SpiAppReadMessage(uint8_t *tx_data, size_t tx_len, void *user_ctx);
```

### 基本用法

1. **`u8InitSpiApp(callback)`**——打开 `/dev/spidev0.0`,启动内部 worker 线程,注册全局回调。同一进程内只允许成功初始化一次,重复调用会返回 `RET_NG`。
2. **`wu8SpiAppSendMessage(tx_data, tx_len, user_ctx)`** / **`wu8SpiAppReadMessage(tx_data, tx_len, user_ctx)`**——把请求塞进队列,**立刻返回**(不阻塞、不等待传输完成),队列满或参数非法时返回 `RET_NG`。
   - `tx_data[0]`:寄存器起始地址(对应虚拟寄存器堆的 `0x00`–`0x7F`)
   - `tx_data[1..]`(仅写):要写入的数据,支持 burst,地址自动自增
   - `wu8SpiAppReadMessage` 内部会自动给 `tx_data[0]` 或上读命令位,调用者不需要自己处理
   - `user_ctx`:调用者自定义的透传指针,回调触发时原样带回,用于识别"这次结果对应哪次请求"(尤其在多线程并发调用同一个全局回调时,靠这个做路由)
3. **实际的 `ioctl` 传输在 worker 线程里异步完成**,处理完后调用初始化时传入的 `callback`,把结果(`ret`/`rx_data`/`len`/`user_ctx`)传出来。
4. **`u8DeinitSpiApp()`**——通知 worker 线程停止,等队列剩余请求处理完再退出线程,关闭 fd。

## 关键设计要点 / 使用注意事项

- **队列是固定深度的循环数组(`SPI_REQ_QUEUE_DEPTH = 8`),不使用 `malloc`**,车规风格,队列满了直接拒绝新请求(返回 `RET_NG`),不做动态扩容。
- **`tx_data` 在入队时会被拷贝进队列**,调用完 `wu8SpiAppSendMessage`/`wu8SpiAppReadMessage` 之后,调用者自己的 `tx_data` buffer 可以立刻释放/复用,不需要等处理完成。
- **`user_ctx` 生命周期陷阱**:如果用信号量把异步接口包装成"同步等待"的写法(如上面示例),`sem_timedwait` 超时后**绝不能直接放弃等待并释放 `ctx`**——请求已经在队列里,一定会被 worker 处理,worker 处理完时仍会往 `user_ctx` 指向的地址写数据、`sem_post`。如果此时 `ctx` 所在的栈内存已经因为函数返回而失效,就是一次真实的 use-after-free。正确做法是超时后继续无限等待(`sem_wait`),把超时当作"处理得比预期慢"的提示,而不是"放弃"的信号。
- **回调 `g_callback(...)` 是在 worker 线程里直接执行的**,不会切回调用者线程,如果回调里做了耗时操作,会拖慢队列后续请求的处理速度。
- 同一时刻只有一份全局队列和一份全局回调,支持多线程并发调用 `wu8SpiAppSendMessage`/`wu8SpiAppReadMessage`,但**回调本身是共享的**,多个调用者必须依靠各自传入不同的 `user_ctx` 来分辨"这次触发的回调该发给谁",`SpiApp.c` 内部不做任何路由,只负责原样转发。

## 目录建议

```
SPI/
├── src/SpiApp.c
├── include/SpiApp.h
└── (依赖 Applications/tools/DummySPI 提供 /dev/spidev0.0)
```
