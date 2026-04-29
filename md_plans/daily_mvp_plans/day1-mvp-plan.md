# Day 1 独立规划：上位机调试增强 + 下位机本地感知 MVP

## 当日成功标准（MVP）

当天结束时，必须形成“上位机可演示 + 下位机可运行”的双端最小模型：

上位机（GEC-6818）：
1. 主界面、触控、后视、录像链路可连续运行；
2. 时间显示在上位机线程中稳定刷新；
3. 天气显示问题完成定位并给出当天可运行替代方案；
4. 历史记录可显示“每段录像首帧缩略图并可回放对应录像”的最小流程；
5. 循环录像守护逻辑可触发“超上限删除最旧录像后继续录制”。

下位机（独立）：
1. 超声波与温湿度采样可稳定输出；
2. 告警可触发（蜂鸣器）；
3. 串口日志可对齐上位机调试字段。

---

## 上位机整体代码深度分析（基于现有源码）

### 当前核心入口与职责

1. `/mnt/d/My R&D/Yunx_Driving_Recorder V2 --union_machine/Sourse/main.c`
   - 主入口：初始化 LCD/TS，播放开机动画，显示主界面；
   - 启动线程：`weather_time_thread`（天气时间） + `touch_thread`（触控处理）。

2. `/mnt/d/My R&D/Yunx_Driving_Recorder V2 --union_machine/Sourse/main_2.c`
   - 单线程触控主循环版本，便于回退调试；
   - 可作为 Day1 问题复现与最小验证入口。

3. `/mnt/d/My R&D/Yunx_Driving_Recorder V2 --union_machine/Sourse/TouchScreen4.c`
   - 触控按钮映射与按钮行为分发（`handleButtonPress` / `handleTouchEvent`）。

4. `/mnt/d/My R&D/Yunx_Driving_Recorder V2 --union_machine/Sourse/VideoPlayer3.c`
   - 后视、录像、历史、循环录像的主要实现；
   - 包含 `record_video()`、`show_history()`、`circular_recording()`。

5. `/mnt/d/My R&D/Yunx_Driving_Recorder V2 --union_machine/Sourse/test.c` + `testtime.c` + `get_weathe2.c`
   - 天气时间线程实现源；
   - `show_time()` 时间函数完整可用；
   - 天气读取依赖 `weather.txt` 文件解析（非板端在线拉取）。

### 关键问题定位（与你的 7 项功能对应）

1. LCD 开机动画：已实现，主流程在 `main.c` 稳定。
2. 主界面触控：已实现，但坐标处理分支里 `ABS_X/ABS_Y` 未显式 `break`，存在偶发坐标覆盖风险。
3. 天气时间显示：已有线程框架，但 `main.c` 线程创建传参为 `NULL`，而 `weather_time_thread` 实现按 `lcd_mp` 指针使用，造成上位机显示异常风险（这是当前未在 GEC-6818 正常显示的高概率主因）。
4. 倒车影像：已实现，卡顿属于后续优化项（先不抢 Day1）。
5. 录像功能：已实现，但主存储为 YUV 原始流。
6. 历史记录：当前实现仅“读目录第一条并显示一帧”，尚未形成“首帧缩略图 + 点击回放”闭环。
7. 循环录像守护：已有 while 循环与超限删除逻辑框架，但路径与计数对象一致性、最旧文件选择和阻塞式运行策略还不完整。

---

## 你今天要做的上位机开发（精细到调用项）

### 功能 1/2（开机动画、触控）今天目标

- 状态：保持已实现。
- 今天要调用：
  - `/mnt/d/My R&D/Yunx_Driving_Recorder V2 --union_machine/Sourse/main.c`
  - `/mnt/d/My R&D/Yunx_Driving_Recorder V2 --union_machine/Sourse/TouchScreen4.c`
- 今天完成标准：
  - 按钮点击日志正确、无误触连发；
  - 触控坐标输出与按钮区域一致。

### 功能 3（天气时间显示）今天目标

- 状态：有问题，优先修复“上位机板端显示”。
- 今天要调用：
  - `/mnt/d/My R&D/Yunx_Driving_Recorder V2 --union_machine/Sourse/main.c`
  - `/mnt/d/My R&D/Yunx_Driving_Recorder V2 --union_machine/Sourse/test.c`
  - `/mnt/d/My R&D/Yunx_Driving_Recorder V2 --union_machine/Sourse/testtime.c`
  - `/mnt/d/My R&D/Yunx_Driving_Recorder V2 --union_machine/Sourse/get_weathe2.c`
- 今天完成标准（MVP）：
  - 时间可每秒刷新；
  - 天气若在线不可用，至少显示“缓存天气或暂无数据”；
  - 不阻塞触控线程与视频线程。

### 功能 4（倒车影像）今天目标

- 状态：已实现，今天不做性能优化。
- 今天要调用：
  - `/mnt/d/My R&D/Yunx_Driving_Recorder V2 --union_machine/Sourse/VideoPlayer3.c`
  - `/mnt/d/My R&D/Yunx_Driving_Recorder V2 --union_machine/Sourse/API_Camera2.c`
- 今天完成标准：
  - 可稳定进入/退出后视模式；
  - 仅记录卡顿场景与触发条件，优化排到全功能完成后。

### 功能 5（录像，YUV）今天目标

- 状态：已实现，保持可用并补齐元数据组织。
- 今天要调用：
  - `/mnt/d/My R&D/Yunx_Driving_Recorder V2 --union_machine/Sourse/VideoPlayer3.c`
  - `/mnt/d/My R&D/Yunx_Driving_Recorder V2 --union_machine/Include/camera.h`
- 今天完成标准：
  - 录像文件可持续落盘；
  - 每段录像能绑定唯一时间戳目录；
  - 保留 YUV 主文件作为回放源。

### 功能 6（历史记录）今天目标

- 状态：未实现完整目标，今天补到 MVP。
- 今天要调用：
  - `/mnt/d/My R&D/Yunx_Driving_Recorder V2 --union_machine/Sourse/VideoPlayer3.c`
  - `/mnt/d/My R&D/Yunx_Driving_Recorder V2 --union_machine/Sourse/TouchScreen4.c`
- 今天完成标准（MVP）：
  - 录制时保存“第一帧 bmp”作为缩略图；
  - 历史页至少支持“选中一条记录 -> 播放对应录像文件”；
  - 不追求完整图库 UI，仅实现最小点击回放链路。

### 功能 7（循环录像守护）今天目标

- 状态：框架有，未完全实现。
- 今天要调用：
  - `/mnt/d/My R&D/Yunx_Driving_Recorder V2 --union_machine/Sourse/VideoPlayer3.c`
  - `/mnt/d/My R&D/Yunx_Driving_Recorder V2 --union_machine/Include/camera.h`
- 今天完成标准（MVP）：
  - 计数达到 `MAX_RECORDS` 时自动删除最旧录像；
  - 删除后立刻继续新录像，不中断主流程；
  - 守护进程模式可从按钮触发并可观察日志。

---

## 下位机 Day1 必调模块（保持原计划）

1. `/mnt/d/My R&D/Yunx_Driving_Recorder V2 --union_machine/portable_code/001__获取hc_sr04超声波模块的策略距离+使用UART1(A-A口线)打印出来/MYM4_LIB/hc_sr04/hc_sr04.c`
2. `/mnt/d/My R&D/Yunx_Driving_Recorder V2 --union_machine/portable_code/002__获取dht11温湿度模块的数据+使用UART1(A-A)口线打印出来/MYM4_LIB/dht11/dht11.c`
3. `/mnt/d/My R&D/Yunx_Driving_Recorder V2 --union_machine/portable_code/007__UART3+使用ESP8266模块连接热点实现上网+访问别人的服务器+使用MQTT通信方式和巴法云物联网进行通信/SYSTEM/uart/uart.c`
4. `/mnt/d/My R&D/Yunx_Driving_Recorder V2 --union_machine/portable_code/001__获取hc_sr04超声波模块的策略距离+使用UART1(A-A口线)打印出来/SYSTEM/delay/delay.c`
5. `/mnt/d/My R&D/Yunx_Driving_Recorder V2 --union_machine/portable_code/009__计数信号量+环境监测触发警报装置/MYM4_LIB/buzzer/buzzer.c`

---

## 今日执行节奏（建议）

1. 上午：先修天气时间线程在 GEC-6818 的显示稳定性（功能 3）；
2. 中午前：打通录像目录与首帧缩略图绑定（功能 5 + 6）；
3. 下午：完成历史点击回放最小链路（功能 6）；
4. 傍晚：完成循环录像计数删除与守护模式可观测日志（功能 7）；
5. 晚间：联测主界面触控、后视、录像、历史、守护五大入口。

---

## Day 1 验收清单（必须全通过）

上位机：
- 开机动画、主界面、触控、后视、录像可稳定演示；
- 时间稳定刷新，天气至少有降级显示；
- 历史页可从首帧缩略图进入对应录像回放；
- 守护进程可在超限后自动删除最旧文件并继续录制。

下位机：
- 超声波与温湿度稳定输出；
- 告警可触发恢复；
- 串口日志字段固定可解析。

---

## 当日风险与回退

- 风险 1（上位机）：天气接口在板端不可用导致 UI 空白；
  - 回退：先展示时间 + 缓存天气文本，不阻塞主流程验收。
- 风险 2（上位机）：循环录像删除策略误删；
  - 回退：先启用“只删最旧 1 条 + 详细日志”保守模式。
- 风险 3（下位机）：DHT11 偶发抖动；
  - 回退：保留超声波 + 告警先过线，温湿度标记降级并记录补救项。
