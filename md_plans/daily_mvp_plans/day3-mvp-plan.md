# Day 3 独立规划：并发稳定 + 上位机交互闭环 MVP

## 当日成功标准（MVP）

当天结束时，必须把 Day2 的“能通信”升级为“并发稳定可演示”。

下位机：
1. 采集任务、通信任务、告警任务在 RTOS 下稳定并发；
2. 计数信号量驱动告警可靠；
3. 长时间运行无死锁、无明显任务饥饿。

上位机：
1. 历史回放入口与实时状态页面可并行切换；
2. 录像过程中可持续看到联调状态（在线/告警/采样数据）；
3. 触控触发的控制命令在高负载下仍可到达下位机。

---

## Day 3 必调模块（下位机 + 上位机）

### 下位机（RTOS 主线）

1. `/mnt/d/My R&D/Yunx_Driving_Recorder V2 --union_machine/portable_code/009__计数信号量+环境监测触发警报装置/USER/main.c`
2. `/mnt/d/My R&D/Yunx_Driving_Recorder V2 --union_machine/portable_code/009__计数信号量+环境监测触发警报装置/FreeRTOS/FreeRTOSConfig.h`
3. `/mnt/d/My R&D/Yunx_Driving_Recorder V2 --union_machine/portable_code/009__计数信号量+环境监测触发警报装置/FreeRTOS/src/tasks.c`
4. `/mnt/d/My R&D/Yunx_Driving_Recorder V2 --union_machine/portable_code/009__计数信号量+环境监测触发警报装置/FreeRTOS/src/queue.c`
5. `/mnt/d/My R&D/Yunx_Driving_Recorder V2 --union_machine/portable_code/009__计数信号量+环境监测触发警报装置/FreeRTOS/src/timers.c`
6. `/mnt/d/My R&D/Yunx_Driving_Recorder V2 --union_machine/portable_code/009__计数信号量+环境监测触发警报装置/MYM4_LIB/buzzer/buzzer.c`
7. `/mnt/d/My R&D/Yunx_Driving_Recorder V2 --union_machine/portable_code/009__计数信号量+环境监测触发警报装置/MYM4_LIB/hc_sr04/hc_sr04.c`
8. `/mnt/d/My R&D/Yunx_Driving_Recorder V2 --union_machine/portable_code/009__计数信号量+环境监测触发警报装置/MYM4_LIB/dht11/dht11.c`

### 上位机（并发与交互稳定）

1. `/mnt/d/My R&D/Yunx_Driving_Recorder V2 --union_machine/Sourse/main.c`
   - 目标：时间天气线程、触控线程、主状态同步稳定。
2. `/mnt/d/My R&D/Yunx_Driving_Recorder V2 --union_machine/Sourse/TouchScreen4.c`
   - 目标：高频点击下仍能正确分发按钮逻辑。
3. `/mnt/d/My R&D/Yunx_Driving_Recorder V2 --union_machine/Sourse/VideoPlayer3.c`
   - 目标：录像/历史/守护功能与联调状态共存。
4. `/mnt/d/My R&D/Yunx_Driving_Recorder V2 --union_machine/Sourse/test.c`
   - 目标：时间天气线程在高负载下不闪退不阻塞。

---

## 精细开发步骤（今天执行顺序）

### 1) 下位机任务分层定稿

- 任务 A：采集任务（超声波 + 温湿度）；
- 任务 B：通信任务（MQTT 发布/订阅/重连）；
- 任务 C：告警任务（蜂鸣器 + 告警状态输出）。

### 2) 信号量与消息流闭环

- 采集异常触发 `Give`；
- 告警任务 `Take` 后执行；
- 通信任务同步发布当前告警态；
- 上位机页面实时显示告警变化与时间戳。

### 3) 上位机高负载联测

- 连续触发“录像/历史/返回/守护”；
- 验证通信状态是否持续在线；
- 验证天气时间显示是否受影响。

### 4) 关键一致性检查

- 上位机展示状态与 MQTT 上报字段一致；
- 录像期间状态仍更新；
- 控制下发回执不丢失。

---

## 当日与 Day 2/Day 4 依赖关系

- 依赖 Day2：协议与基础联通已稳定；
- 输出 Day4：为整合展示（OLED + 上位机 + 云端）提供稳定任务基线。

---

## Day 3 验收清单（必须全通过）

下位机：
- 三任务并发运行 30 分钟无死锁；
- 告警触发响应可重复、可恢复；
- MQTT 持续发布无长时间中断。

上位机：
- 连续切换页面不导致线程异常；
- 录像时仍能看到实时状态更新；
- 触控控制命令在压力场景下可达。

---

## 当日风险与回退

- 风险 1：任务优先级不当导致通信或采集饥饿；
  - 回退：先固定“采集+告警”双任务，通信降频但必须可恢复。
- 风险 2：上位机线程竞争导致显示异常；
  - 回退：优先保证状态文字显示，延后非核心 UI 刷新。

---

## 关键提醒（延续）

- 温湿度阈值字段必须复核，避免条件误写；
- 串口日志粒度要受控，防止影响 RTOS 实时性与上位机刷新流畅度。
