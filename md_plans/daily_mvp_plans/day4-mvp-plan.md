# Day 4 独立规划：全链路整合演示 + 扩展占位 MVP

## 当日成功标准（MVP）

当天结束时，必须完成“可交付演示版本”：

上位机：
1. 开机动画、触控、后视、录像、历史、守护六条入口稳定；
2. 历史记录可展示首帧并回放关联录像；
3. 循环录像守护达到上限自动删除最旧记录并继续录制；
4. 天气时间在主界面稳定显示（天气允许降级）。

下位机：
1. 采集、告警、通信并发稳定；
2. OLED（若接入）与上位机状态一致；
3. MQTT/串口状态可被上位机验证。

扩展层：
1. BSD 模块完成占位接口与联调接入点定义；
2. 温湿度火警路径完成参数与验收口径固化。

---

## Day 4 必调模块（下位机 + 上位机）

### 下位机（整合与显示）

1. `/mnt/d/My R&D/Yunx_Driving_Recorder V2 --union_machine/portable_code/011__互斥锁+超声波和温湿度模块共用oled模块/USER/main.c`
2. `/mnt/d/My R&D/Yunx_Driving_Recorder V2 --union_machine/portable_code/011__互斥锁+超声波和温湿度模块共用oled模块/MYM4_LIB/iic_oled/iic_oled.c`
3. `/mnt/d/My R&D/Yunx_Driving_Recorder V2 --union_machine/portable_code/011__互斥锁+超声波和温湿度模块共用oled模块/MYM4_LIB/iic_oled/iic_oled_font.c`
4. `/mnt/d/My R&D/Yunx_Driving_Recorder V2 --union_machine/portable_code/011__互斥锁+超声波和温湿度模块共用oled模块/MYM4_LIB/iic_oled/iic_oled_bmp.c`
5. `/mnt/d/My R&D/Yunx_Driving_Recorder V2 --union_machine/portable_code/011__互斥锁+超声波和温湿度模块共用oled模块/MYM4_LIB/hc_sr04/hc_sr04.c`
6. `/mnt/d/My R&D/Yunx_Driving_Recorder V2 --union_machine/portable_code/011__互斥锁+超声波和温湿度模块共用oled模块/MYM4_LIB/dht11/dht11.c`
7. `/mnt/d/My R&D/Yunx_Driving_Recorder V2 --union_machine/portable_code/011__互斥锁+超声波和温湿度模块共用oled模块/FreeRTOS/src/tasks.c`
8. `/mnt/d/My R&D/Yunx_Driving_Recorder V2 --union_machine/portable_code/007__UART3+使用ESP8266模块连接热点实现上网+访问别人的服务器+使用MQTT通信方式和巴法云物联网进行通信/MYM4_LIB/esp8266/esp8266_bemfayun_mqtt.c`

### 上位机（全功能收口）

1. `/mnt/d/My R&D/Yunx_Driving_Recorder V2 --union_machine/Sourse/main.c`
2. `/mnt/d/My R&D/Yunx_Driving_Recorder V2 --union_machine/Sourse/main_2.c`（回退入口保留）
3. `/mnt/d/My R&D/Yunx_Driving_Recorder V2 --union_machine/Sourse/TouchScreen4.c`
4. `/mnt/d/My R&D/Yunx_Driving_Recorder V2 --union_machine/Sourse/VideoPlayer3.c`
5. `/mnt/d/My R&D/Yunx_Driving_Recorder V2 --union_machine/Sourse/test.c`
6. `/mnt/d/My R&D/Yunx_Driving_Recorder V2 --union_machine/Sourse/get_weathe2.c`
7. `/mnt/d/My R&D/Yunx_Driving_Recorder V2 --union_machine/Sourse/testtime.c`

---

## 精细开发步骤（今天执行顺序）

### 1) 先做全链路状态一致性

- 下位机本地状态、MQTT 上报状态、上位机显示状态三方字段统一；
- 统一字段至少包括：
  - `distance_cm` `temp_c` `humidity` `alarm_state` `device_online` `record_state`

### 2) 完成历史与守护最终闭环

- 录像生成：`YUV 主文件 + 首帧 BMP 缩略图 + 时间戳索引`；
- 历史页面：点击缩略图进入对应录像回放；
- 守护进程：达到 `MAX_RECORDS` 自动删除最旧记录并继续录制。

### 3) 完成天气时间收口

- 时间必须稳定显示；
- 天气获取失败时降级显示“缓存/暂无数据”，不影响主界面。

### 4) BSD 占位收口

- 明确 BSD 输入输出结构与状态码；
- 增加模拟数据接入口；
- 形成“后续替换真实 BSD 驱动”的最小改造路径。

---

## 当日与前 3 天依赖关系

- 依赖 Day1：上位机基础功能已通；
- 依赖 Day2：联调通信已通；
- 依赖 Day3：并发稳定基线已建立；
- Day4 输出：可演示版本 + 下周期可执行扩展接口。

---

## Day 4 验收清单（必须全通过）

上位机：
- 六大入口（主界面、后视、录像、历史、守护、电源逻辑）可演示；
- 历史“首帧缩略图 -> 回放”链路可跑通；
- 守护模式达到上限后可自动清理并连续录制。

下位机：
- 采集、告警、通信并发稳定；
- 与上位机展示字段一致；
- 联网异常时可恢复。

扩展：
- BSD 占位接口文档化完成；
- 温湿度火警阈值策略与测试点完成固化。

---

## 当日风险与回退

- 风险 1：OLED/显示刷新占用过高影响任务实时性；
  - 回退：降刷新率并只保留关键字段展示。
- 风险 2：历史回放与守护并发冲突；
  - 回退：回放时暂停删除动作，退出回放后恢复守护。
- 风险 3：天气接口不稳定影响 UI；
  - 回退：固定显示时间与缓存天气，不阻塞主流程。

---

## 四天闭环结论（每天可跑 MVP）

- Day 1：上位机基础调试 + 下位机本地感知 MVP；
- Day 2：双端通信联调 MVP；
- Day 3：并发稳定与交互闭环 MVP；
- Day 4：全链路整合演示与扩展占位 MVP。

每天都可独立演示，并可逐日递进，不需要等待最后一天才可运行。
