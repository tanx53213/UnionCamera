# 裕行行车记录后视仪 — Bug 修复日志

> 修复开始时间：2026-04-30  
> 修复依据：`error_log.md` 深度分析报告  
> 修复范围：24 个 Bug 全部修复

---

## 修复记录

### C1 — 全局变量 `lcd_fd`、`lcd_mp` 多重定义

| 字段 | 值 |
|------|-----|
| **时间戳** | 2026-04-30 |
| **类型** | Critical — 符号冲突 |
| **涉及文件** | `API_Camera2.c:20-21`, `VideoPlayer3.c:14-15`, `test.c:13` |
| **修复方式** | 移除 `API_Camera2.c`, `VideoPlayer3.c`, `test.c` 中重复的 `int lcd_fd;` 和 `unsigned int *lcd_mp;` 定义。仅在 `main.c` 保留定义，其余文件通过 `camera.h` 的 `extern` 声明引用。 |
| **状态** | ✅ 已修复 |

### C2 — `camera_fd` / `n_buffers` / `buffers` 双重定义

| 字段 | 值 |
|------|-----|
| **时间戳** | 2026-04-30 |
| **类型** | Critical — 符号冲突 + 逻辑错误 |
| **涉及文件** | `API_Camera2.c:26-32`, `VideoPlayer3.c:17-19`, `Include/camera.h` |
| **修复方式** | 移除 `VideoPlayer3.c` 中的 `static int camera_fd`, `static unsigned int n_buffers`, `static struct buffer *buffers`。在 `camera.h` 中添加对应的 `extern` 声明，统一从 `API_Camera2.c` 的定义引用。 |
| **状态** | ✅ 已修复 |

### C3 — `test.c` 中 `lcd_mp` 未初始化

| 字段 | 值 |
|------|-----|
| **时间戳** | 2026-04-30 |
| **类型** | Critical — 空指针解引用 |
| **涉及文件** | `test.c:13` |
| **修复方式** | 移除 `test.c` 中独立的 `unsigned int *lcd_mp;` 定义，改为通过 `camera.h` 的 `extern` 声明引用 `main.c` 中已通过 mmap 初始化的 `lcd_mp`。 |
| **状态** | ✅ 已修复 |

### C4 — `program_running` 双重定义导致死锁

| 字段 | 值 |
|------|-----|
| **时间戳** | 2026-04-30 |
| **类型** | Critical — 符号冲突 + 死锁 |
| **涉及文件** | `test.c:18`, `Include/camera.h` |
| **修复方式** | 移除 `test.c` 中的 `static volatile bool program_running`，在 `camera.h` 中添加 `extern volatile bool program_running;`。线程循环现在统一检查 `main.c` 中定义的全局 `program_running`。 |
| **状态** | ✅ 已修复 |

### C5 — YUV→RGB 整数除法截断

| 字段 | 值 |
|------|-----|
| **时间戳** | 2026-04-30 |
| **类型** | Critical — 颜色完全错误 |
| **涉及文件** | `API_Camera2.c:46-48` |
| **修复方式** | 将整数除法 `255/219`、`127/112` 替换为预计算的浮点系数：`1.164*(Y-16) + 1.596*(Cr-128)` / `1.164*(Y-16) - 0.392*(Cb-128) - 0.813*(Cr-128)` / `1.164*(Y-16) + 2.017*(Cb-128)`。浮点结果显式转换为 `(int)`。 |
| **状态** | ✅ 已修复 |

### C6 — `mainloop3()` 状态检查错误

| 字段 | 值 |
|------|-----|
| **时间戳** | 2026-04-30 |
| **类型** | Critical — 逻辑错误 |
| **涉及文件** | `API_Camera2.c:352, 399` |
| **修复方式** | 将 `while (count-- > 0 && current_state == STATE_BACKUP)` 改为 `STATE_RECORD`，同时内层 `if (current_state != STATE_BACKUP)` 也改为 `STATE_RECORD`。 |
| **状态** | ✅ 已修复 |

### C7 — 录像按钮未调用 `record_video()`

| 字段 | 值 |
|------|-----|
| **时间戳** | 2026-04-30 |
| **类型** | Critical — 功能缺失 |
| **涉及文件** | `VideoPlayer3.c:327-337` |
| **修复方式** | `handleButtonPress` case 1 中原有的 `open_device(); init_device(); start_capturing(); mainloop3();` 替换为直接调用 `record_video()`。`record_video()` 内部已包含完整的摄像头打开、初始化和录制逻辑。 |
| **状态** | ✅ 已修复 |

### H1 — 天气字段 Key 与 `weather.txt` 不匹配

| 字段 | 值 |
|------|-----|
| **时间戳** | 2026-04-30 |
| **类型** | High — 数据解析失败 |
| **涉及文件** | `get_weather.c:76-80` |
| **修复方式** | 修正 key 字符串：`"Humidity:"` → `"Relative_Humidity:"`, `"Sky_conditions:"` → `"Sky conditions:"`（下划线改空格，与 weather.txt 实际内容匹配）。 |
| **状态** | ✅ 已修复 |

### H2 — 结构体类型不一致 (`Weather` vs `weather1`)

| 字段 | 值 |
|------|-----|
| **时间戳** | 2026-04-30 |
| **类型** | High — 类型不匹配 / UB |
| **涉及文件** | `get_weather.c:12-19, 47, 79` |
| **修复方式** | 移除 `get_weather.c` 中的局部 `typedef struct { ... } Weather;`，添加 `#include "camera.h"`，统一使用 `camera.h` 中定义的 `weather1` 类型。字段名 `weather_desc` 改为 `Weather` 以匹配 `weather1` 结构体。 |
| **状态** | ✅ 已修复 |

### H3 — `mainloop3()` 固定使用 `buffers[3]`

| 字段 | 值 |
|------|-----|
| **时间戳** | 2026-04-30 |
| **类型** | High — 数组索引错误 |
| **涉及文件** | `API_Camera2.c:393` |
| **修复方式** | `yuyv_to_lcd(buffers[3].start)` → `yuyv_to_lcd(buffers[buf.index].start)`，使用 `VIDIOC_DQBUF` 返回的实际缓冲区索引，而非硬编码第 4 个缓冲区。与 `mainloop()` 保持一致。 |
| **状态** | ✅ 已修复 |

### H4 — `yuyv_to_bmp` YUYV 解码错误

| 字段 | 值 |
|------|-----|
| **时间戳** | 2026-04-30 |
| **类型** | High — 像素格式错误 |
| **涉及文件** | `VideoPlayer3.c:83-92` |
| **修复方式** | 重写像素转换循环，按 YUYV 标准的 4 字节/像素对格式处理：每次迭代处理 2 个像素（`x += 2`），共享 U 和 V 分量。与 `yuyv_to_lcd` 的处理方式保持一致。 |
| **状态** | ✅ 已修复 |

### H5 — `Bmp_Decode` 文件句柄泄漏

| 字段 | 值 |
|------|-----|
| **时间戳** | 2026-04-30 |
| **类型** | High — 资源泄漏 |
| **涉及文件** | `op_Boot_anim.c:86-121` |
| **修复方式** | 在 `Bmp_Decode` 函数中，所有像素写入完毕后添加 `fclose(bmp_fp);`。每次开机动画播放避免泄漏 104 个文件句柄。 |
| **状态** | ✅ 已修复 |

### H6 — `op_dve()` LCD 资源永久泄漏

| 字段 | 值 |
|------|-----|
| **时间戳** | 2026-04-30 |
| **类型** | High — 资源泄漏 |
| **涉及文件** | `op_Boot_anim.c:46-76` |
| **修复方式** | 重写 `op_dve()`：mmap 失败时先 `close(lcd_fd)` 再返回 -1；成功时返回 0 而非无返回值；将 `exit(-1)` 改为 `return -1` 使调用者可处理错误；添加注释标明此函数已弃用。 |
| **状态** | ✅ 已修复 |

### H7 — `record_video()` 重复打开摄像头未关闭

| 字段 | 值 |
|------|-----|
| **时间戳** | 2026-04-30 |
| **类型** | High — 资源泄漏 |
| **涉及文件** | `VideoPlayer3.c:109-116` |
| **修复方式** | 在 `record_video()` 中 `open_device()` 之前添加检查：`if (camera_fd >= 0) stop_camera();`，确保前次打开的摄像头被正确关闭后再重新打开。 |
| **状态** | ✅ 已修复 |

### H8 — `stop_camera()` 使用错误的 `camera_fd`

| 字段 | 值 |
|------|-----|
| **时间戳** | 2026-04-30 |
| **类型** | High — 逻辑错误 |
| **涉及文件** | `VideoPlayer3.c:26-35` |
| **修复方式** | 移除 `VideoPlayer3.c` 中的 `static int camera_fd` 后，`stop_camera()` 中的 `camera_fd` 通过 `camera.h` 的 `extern` 声明自动引用 `API_Camera2.c` 中正确的全局 `camera_fd`。无需修改函数体。 |
| **状态** | ✅ 已修复（由 C2 修复覆盖） |

### M1 — LCD 互斥锁重复定义

| 字段 | 值 |
|------|-----|
| **时间戳** | 2026-04-30 |
| **类型** | Medium — 并发安全 |
| **涉及文件** | `test.c:17` |
| **修复方式** | 移除 `test.c` 中的 `static pthread_mutex_t lcd_mutex`，在 `camera.h` 中添加 `extern pthread_mutex_t lcd_mutex;`。触摸线程和天气线程现在锁定同一个互斥锁。 |
| **状态** | ✅ 已修复 |

### M2 — `mkdir()` 无法创建嵌套目录

| 字段 | 值 |
|------|-----|
| **时间戳** | 2026-04-30 |
| **类型** | Medium — 健壮性 |
| **涉及文件** | `VideoPlayer3.c:122, 129` |
| **修复方式** | 添加 `mkdir_p()` 递归创建目录函数。所有 `mkdir()` 调用改为 `mkdir_p()`，并检查返回值，失败时打印错误信息并提前返回。 |
| **状态** | ✅ 已修复 |

### M3 — `weather_time_thread` 忽略线程参数

| 字段 | 值 |
|------|-----|
| **时间戳** | 2026-04-30 |
| **类型** | Medium — 冗余/混淆 |
| **涉及文件** | `test.c:69-73` |
| **修复方式** | 修改 `weather_time_thread`：优先使用 `(unsigned int *)arg` 参数，仅在参数为 NULL 时回退到全局 `lcd_mp`。 |
| **状态** | ✅ 已修复 |

### M4 — `yuyv_to_lcd2` 中 `"a+"` 模式 fseek 无效 + 丢弃返回值

| 字段 | 值 |
|------|-----|
| **时间戳** | 2026-04-30 |
| **类型** | Medium — API 误用 |
| **涉及文件** | `API_Camera2.c:119, 131-132` |
| **修复方式** | 将 `fopen("./1.bmp", "a+")` 改为 `"rb"` 只读模式；将 `pixel_yuv2rgb(...)` 的返回值赋值给 `rgbbuf[i]` 而非丢弃；添加 `fclose(bmp_fp)` 关闭文件句柄。 |
| **状态** | ✅ 已修复 |

### M5 — 天气信息仅显示温度

| 字段 | 值 |
|------|-----|
| **时间戳** | 2026-04-30 |
| **类型** | Medium — 功能不完整 |
| **涉及文件** | `test.c:100-131` |
| **修复方式** | 在 `weather_time_thread` 中新增 3 个 bitmap 框（湿度、风向、天气描述），显示 `Relative_Humidity`, `Wind`, `Weather` 字段。LCD 渲染新增对应行（y=120, 140, 160）。 |
| **状态** | ✅ 已修复 |

### L1 — `timebuffer` 多重定义

| 字段 | 值 |
|------|-----|
| **时间戳** | 2026-04-30 |
| **类型** | Low — 符号冲突 |
| **涉及文件** | `test.c:19` |
| **修复方式** | 移除 `test.c` 中的 `char timebuffer[80];`，仅保留 `testtime.c` 中的定义。`test.c` 通过 `showtime.h` 的 `extern char timebuffer[80];` 引用。 |
| **状态** | ✅ 已修复（由 C3 修复覆盖） |

### L2 — 使用已弃用的 `bzero()`

| 字段 | 值 |
|------|-----|
| **时间戳** | 2026-04-30 |
| **类型** | Low — 可移植性 |
| **涉及文件** | `main.c:152`, `VideoPlayer3.c:290` |
| **修复方式** | `bzero(OnTimets, sizeof(struct ts_pix))` → `memset(OnTimets, 0, sizeof(struct ts_pix))`; `bzero(cmd, sizeof(cmd))` → `memset(cmd, 0, sizeof(cmd))`。使用标准 C 库函数。 |
| **状态** | ✅ 已修复 |

### L3 — `mkdir()` 返回值未检查

| 字段 | 值 |
|------|-----|
| **时间戳** | 2026-04-30 |
| **类型** | Low — 错误处理 |
| **涉及文件** | `VideoPlayer3.c:122, 129` |
| **修复方式** | 所有 `mkdir()` / `mkdir_p()` 调用后检查返回值，失败时打印错误信息并执行 `stop_camera()` + `return` 提前退出。 |
| **状态** | ✅ 已修复（由 M2 修复覆盖） |

### L4 — `system("rm -rf")` 安全隐患

| 字段 | 值 |
|------|-----|
| **时间戳** | 2026-04-30 |
| **类型** | Low — 安全性 |
| **涉及文件** | `VideoPlayer3.c:289-292` |
| **修复方式** | 将 `sprintf(cmd, "rm -rf %s", oldest_file); system(cmd);` 替换为 `opendir`/`remove`/`rmdir` 的组合调用。对于目录使用 `readdir` 遍历删除内部文件后 `rmdir`；对于普通文件直接 `remove`。 |
| **状态** | ✅ 已修复 |

---

## 修改文件清单

| 文件 | 修改条数 | 涉及 Bug |
|------|----------|----------|
| `Include/camera.h` | 4 行新增 | C1, C2, C4, M1 |
| `Sourse/main.c` | 1 行修改 | L2 |
| `Sourse/API_Camera2.c` | 多处修改 | C1, C5, C6, H3, M4 |
| `Sourse/VideoPlayer3.c` | 多处修改 | C1, C2, C7, H4, H7, M2, L2, L3, L4 |
| `Sourse/test.c` | 多处修改 | C1, C3, C4, M1, M3, M5, L1 |
| `Sourse/get_weather.c` | 多处修改 | H1, H2 |
| `Sourse/op_Boot_anim.c` | 2 处修改 | H5, H6 |

---

## 修复统计

| 级别 | 总数 | 已修复 |
|------|------|--------|
| Critical | 7 | 7 ✅ |
| High | 8 | 8 ✅ |
| Medium | 5 | 5 ✅ |
| Low | 4 | 4 ✅ |
| **合计** | **24** | **24 ✅** |

---

> 修复完成时间：2026-04-30  
> 下一阶段：编译验证 → 功能测试

---

## Round 2 — 功能增强修复 (2026-04-30)

### FR1 — 后视摄像头画面缩小 + 右侧预留 100×480 数据栏

| 字段 | 值 |
|------|-----|
| **时间戳** | 2026-04-30 |
| **类型** | Feature — 布局调整 + 超声波数据预留 |
| **涉及文件** | `API_Camera2.c:171-178`, `Draw_line.c` |
| **修复方式** | `yuyv_to_lcd` 中 LCD 写入偏移从 `x+80` 改为 `x+30`，将 640px 宽摄像头画面居中于左侧 700px 区域（列 30-669），右侧留出列 700-799（100×480）供超声波传感器数据展示。`Draw_line` 的辅助标线坐标同步调整为以摄像头区域边缘（列 30 和 669）为基准。 |
| **状态** | ✅ 已修复 |

### FR2 — 电源键在任何状态下始终可用

| 字段 | 值 |
|------|-----|
| **时间戳** | 2026-04-30 |
| **类型** | Feature — 触摸响应 + 架构重构 |
| **涉及文件** | `API_Camera2.c`, `VideoPlayer3.c`, `Include/camera.h` |
| **描述** | 原先在"后视"等状态下，`mainloop()`/`record_video()`/`circular_recording()` 均以同步方式在触摸线程内执行，导致触摸线程阻塞，无法再响应电源键等触摸事件。 |
| **修复方式** | 全部改造为异步线程模型： |
| | 1. `mainloop()` → `backup_camera_thread()`：后视摄像头帧循环在独立线程运行，循环检查 `current_state == STATE_BACKUP` |
| | 2. `record_video()` → `recording_thread()`：录像功能在独立线程运行 |
| | 3. `circular_recording()` → `guard_thread()`：守护进程在独立线程运行，`while(1)` 改为 `while(current_state == STATE_GUARD)` |
| | 4. `handleButtonPress`：case 0/1/4 改为 `pthread_create` 启动线程，不阻塞触摸线程 |
| | 5. `return_to_main`：先设置 `current_state = STATE_MAIN` 通知各线程退出，再 `pthread_join` 等待线程结束，最后 `stop_camera()` |
| | 6. `record_video` 循环条件改为 `STATE_RECORD \|\| STATE_GUARD`，使守护进程模式下也能正常录制 |
| **状态** | ✅ 已修复 |

- **新增 Bug 检出与修复**

#### NB1 — 守护进程中 `record_video()` 状态检查遗漏

| 字段 | 值 |
|------|-----|
| **时间戳** | 2026-04-30 |
| **类型** | 新检出 — 逻辑错误 |
| **涉及文件** | `VideoPlayer3.c:185` |
| **描述** | `record_video()` 的 while 条件仅检查 `current_state == STATE_RECORD`，但被 `circular_recording()` 调用时 state 为 `STATE_GUARD`，导致守护进程模式下录制从不执行。 |
| **修复** | 改为 `current_state == STATE_RECORD \|\| current_state == STATE_GUARD` |
| **状态** | ✅ 已修复 |

#### NB2 — `circular_recording()` 无限循环无法中断

| 字段 | 值 |
|------|-----|
| **时间戳** | 2026-04-30 |
| **类型** | 新检出 — 逻辑错误 |
| **涉及文件** | `VideoPlayer3.c:287` |
| **描述** | `while(1)` 无限循环，即使 state 改变也无法退出。 |
| **修复** | 改为 `while(current_state == STATE_GUARD)` |
| **状态** | ✅ 已修复 |

---

## Round 3 — 全项目路径硬编码统一修复 (2026-04-30)

### 问题总览

| # | 文件 | 问题路径 | 类型 |
|---|------|----------|------|
| P1 | `VideoPlayer3.c:115` | `/tmp/.../Data/runV1.bmp` | 绝对硬编码 |
| P2 | `VideoPlayer3.c:36` vs `camera.h` | `RECORD_PATH` 双重定义 | 宏冲突 |
| P3 | `get_weather.c:10` vs `camera.h` | `WEATHER_TXTPATH` 双重定义 | 宏冲突 |
| P4 | `test.c:7-11` | `LCD_DEV/MAPSIZE` 重复 | 宏冲突 |
| P5 | `op_Boot_anim.c:26-28` | `LCD_DEV/MMAP_SIZE` 重复 | 宏冲突 |
| P6 | `VideoPlayer3.c:39` | `TS_DEV` 重复 | 宏冲突 |

### P1 — 修复 `return_to_main` 硬编码 BMP 路径 ✅

| 字段 | 值 |
|------|-----|
| **时间戳** | 2026-04-30 |
| **类型** | Critical — 运行时路径不可移植 |
| **涉及文件** | `main.c:5`, `camera.h:39`, `VideoPlayer3.c:115` |
| **修复方式** | `resolve_asset_path` 从 `static` 提升为全局函数，声明于 `camera.h`。`return_to_main` 改为调用此函数解析 `runV1.bmp` 路径（依次尝试 `./Object_Yunx_Driving_Recorder2/Data/` → `./Data/` → `/tmp/.../Data/`），替代原有硬编码绝对路径。 |
| **状态** | ✅ 已修复 |

### P2-P6 — 全项目路径宏统一至 camera.h ✅

| 字段 | 值 |
|------|-----|
| **时间戳** | 2026-04-30 |
| **类型** | Medium — 宏符号冲突及维护性 |
| **涉及文件** | `camera.h`, `VideoPlayer3.c`, `get_weather.c`, `test.c`, `op_Boot_anim.c` |
| **修复方式** | |
| | 1. `RECORD_PATH` → 统一定义为 `/tmp/camera_records`（仅 camera.h） |
| | 2. `WEATHER_TXTPATH` → 统一定义为 `./weather.txt`（仅 camera.h） |
| | 3. 移除 VideoPlayer3.c 中 `#define RECORD_PATH`、`#define TS_DEV` |
| | 4. 移除 get_weather.c 中 `#define WEATHER_TXTPATH`，改用 `resolve_asset_path` 查找 weather.txt |
| | 5. 移除 test.c 中 `#define LCD_DEV`、`#define LCD_MAPSIZE` |
| | 6. 移除 op_Boot_anim.c 中 `#define LCD_DEV`、`#define MMAP_SIZE` |
| **状态** | ✅ 已修复 |

### 修复后路径宏集中管理

```
路径/设备宏        唯一定义位置    用途
─────────────────────────────────────────────────
LCD_DEV            camera.h      LCD 帧缓冲设备
LCD_MAPSIZE        camera.h      LCD mmap 大小
TS_DEV             camera.h      触摸屏设备
CAMERA_PATH        camera.h      摄像头设备
WEATHER_TXTPATH    camera.h      天气数据文件
RECORD_PATH        camera.h      录像存储根目录
resolve_asset_path camera.h声明   路径三层回退解析
                   main.c 实现
```

> 修复完成时间：2026-04-30

