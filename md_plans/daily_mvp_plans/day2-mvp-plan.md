# Day 2 独立规划：双端联调通信 MVP 必达

## 当日成功标准（MVP）

当天结束时，必须形成“上位机可观测 + 下位机可上报 + 双向可控”的最小闭环。

上位机（GEC-6818）：
1. 可接收并显示下位机关键状态（距离/温湿度/告警/在线状态）；
2. 可发出至少 1 条控制指令（如告警开关或采样频率）；
3. 历史记录入口不阻塞通信线程（功能 6 与联调并行）。

下位机：
1. ESP8266 联网成功；
2. MQTT 连接、订阅、发布稳定；
3. 能接收上位机控制并回执结果。

---

## Day 2 必调模块（下位机 + 上位机）

### 下位机（portable_code）

1. `/mnt/d/My R&D/Yunx_Driving_Recorder V2 --union_machine/portable_code/007__UART3+使用ESP8266模块连接热点实现上网+访问别人的服务器+使用MQTT通信方式和巴法云物联网进行通信/MYM4_LIB/esp8266/esp8266.c`
   - 目标：AT 基础流程（连 AP、建链、透传）。
2. `/mnt/d/My R&D/Yunx_Driving_Recorder V2 --union_machine/portable_code/007__UART3+使用ESP8266模块连接热点实现上网+访问别人的服务器+使用MQTT通信方式和巴法云物联网进行通信/MYM4_LIB/esp8266/esp8266_bemfayun_mqtt.c`
   - 目标：MQTT 连接、订阅、发布、心跳。
3. `/mnt/d/My R&D/Yunx_Driving_Recorder V2 --union_machine/portable_code/007__UART3+使用ESP8266模块连接热点实现上网+访问别人的服务器+使用MQTT通信方式和巴法云物联网进行通信/MYM4_LIB/timer/timer.c`
   - 目标：PING 节拍与重连判定。
4. `/mnt/d/My R&D/Yunx_Driving_Recorder V2 --union_machine/portable_code/007__UART3+使用ESP8266模块连接热点实现上网+访问别人的服务器+使用MQTT通信方式和巴法云物联网进行通信/SYSTEM/uart/uart.c`
   - 目标：串口收发与日志观测。
5. `/mnt/d/My R&D/Yunx_Driving_Recorder V2 --union_machine/portable_code/007__UART3+使用ESP8266模块连接热点实现上网+访问别人的服务器+使用MQTT通信方式和巴法云物联网进行通信/CJSON/cJSON.c`
   - 目标：JSON payload 封装/解析。

### 上位机（联调入口）

1. `/mnt/d/My R&D/Yunx_Driving_Recorder V2 --union_machine/Sourse/main.c`
   - 目标：主线程 + 触控线程 + 时间天气线程保持稳定。
2. `/mnt/d/My R&D/Yunx_Driving_Recorder V2 --union_machine/Sourse/TouchScreen4.c`
   - 目标：新增/复用“联调按钮行为”（发送控制指令、刷新状态）。
3. `/mnt/d/My R&D/Yunx_Driving_Recorder V2 --union_machine/Sourse/VideoPlayer3.c`
   - 目标：保证录像/历史入口在联调时不阻塞主流程。
4. `/mnt/d/My R&D/Yunx_Driving_Recorder V2 --union_machine/Sourse/test.c`
   - 目标：天气时间线程与联调显示共存验证。

---

## 精细开发步骤（今天执行顺序）

### 1) 协议冻结（先做）

- 冻结最小 topic：
  - 上报：`device/status`
  - 下发：`device/control`
  - 心跳：`device/heartbeat`
- 冻结 payload 字段：
  - `distance_cm` `temp_c` `humidity` `alarm_state` `device_online` `ts`

### 2) 下位机上报链路

- 先跑“联网 + 心跳”；
- 再跑“周期状态上报（1~3 秒）”；
- 异常时回传错误码（采样失败/串口超时/重连中）。

### 3) 上位机控制链路

- 从触控入口发出控制命令；
- 控制命令至少支持：
  - 告警启停；
  - 采样周期切换（快/慢）；
- 显示回执结果（成功/失败 + 原因）。

### 4) 与功能 6/7 并行约束

- 历史记录和守护进程入口不能阻塞通信；
- 若触发录像，通信线程仍需保持心跳与状态更新。

---

## 当日与 Day 1/Day 3 依赖关系

- 依赖 Day 1：采样字段已固定并可稳定输出；
- 输出 Day 3：为 RTOS 任务化提供稳定协议与通信状态机。

---

## Day 2 验收清单（必须全通过）

上位机：
- 可见下位机状态实时刷新；
- 至少 1 条控制命令下发成功并有回执；
- 进入录像/历史页面后，通信不失联。

下位机：
- 自动联网 + MQTT 会话建立；
- 连续 100 条消息发布无致命错误；
- 断线后可自动恢复并继续上报。

---

## 当日风险与回退

- 风险 1：UART 缓冲溢出导致 AT 响应截断；
  - 回退：降低发布频率，先保“心跳+单字段状态”。
- 风险 2：上位机页面切换影响通信处理；
  - 回退：先锁定单界面联调，再逐步放开多入口联调。
