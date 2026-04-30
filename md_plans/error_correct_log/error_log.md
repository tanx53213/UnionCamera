# 裕行行车记录后视仪 — 深度 Bug 分析报告

> 分析时间：2026-04-30  
> 分析范围：`Sourse/*.c` + `Include/*.h` + `Makefile` + `weather.txt`
> 分析方式：静态代码审查 + 跨文件符号追踪
> **状态：全部 24 个 Bug 已于 2026-04-30 修复 ✅**

---

## Bug 分级标准

| 等级 | 含义 |
|------|------|
| **Critical** | 程序无法编译 / 启动即崩溃 / 核心功能完全失效 |
| **High** | 功能严重异常 / 数据错误 / 资源不可逆泄漏 |
| **Medium** | 潜在风险 / 边界条件失效 / 健壮性不足 |
| **Low** | 代码规范 / 可移植性 / 维护性 |

---

## Critical (7 个) — ✅ 全部已修复

### C1. 全局变量 `lcd_mp`、`lcd_fd` 多重定义 → 链接错误 ✅

| 项目 | 详情 |
|------|------|
| 文件 | `main.c:46-47`, `API_Camera2.c:20-21`, `VideoPlayer3.c:14-15`, `test.c:13` |
| 类型 | 符号冲突 |
| 描述 | `lcd_fd` 和 `lcd_mp` 在 4 个 .c 文件中各自定义为全局变量（未使用 `extern`）。GCC 10+ 默认 `-fno-common`，会报 `multiple definition` 链接错误。`-fcommon` 模式下虽可合并为 common symbol，但属于未定义行为。 |
| 复现 | `arm-linux-gcc ... -fno-common` |
| 修复 | 2026-04-30 — 仅 `main.c` 保留定义，其余文件移除并通过 `camera.h` 的 `extern` 引用 |

### C2. `camera_fd` 双重定义 → 录像/守护进程功能完全失效 ✅

| 项目 | 详情 |
|------|------|
| 文件 | `API_Camera2.c:26` vs `VideoPlayer3.c:17` |
| 类型 | 符号冲突 + 逻辑错误 |
| 描述 | `API_Camera2.c` 定义全局 `int camera_fd = -1;`（`open_device`/`init_device`/`start_capturing`/`mainloop` 均使用此变量）。`VideoPlayer3.c` 定义 `static int camera_fd = -1;`。`record_video()` 调用 `open_device()` 设置了全局 `camera_fd`，但后续 `select(camera_fd+1, ...)` 和 `ioctl(camera_fd, ...)` 使用的是 `VideoPlayer3.c` 的静态 `camera_fd`（值为 -1）。`select(0, ...)` 等待 fd=0(stdin)，`ioctl(-1, ...)` 直接失败。同理 `stop_camera()` 关闭的也是 -1，真正打开的 fd 从未关闭，造成资源泄漏。 |
| 影响 | 守护进程按钮（case 4）和 `record_video()` 完全不可用 |
| 修复 | 2026-04-30 — 移除 `VideoPlayer3.c` 中的静态副本，`camera.h` 添加 `extern` 声明统一引用 |

### C3. `test.c` 中 `lcd_mp` 未初始化 → 天气线程写空指针崩溃 ✅

| 项目 | 详情 |
|------|------|
| 文件 | `test.c:13`, `test.c:73` |
| 类型 | 未初始化指针 |
| 描述 | `test.c` 定义了独立的 `unsigned int *lcd_mp;`（值 = NULL）。`weather_time_thread` 在 `show_font_to_lcd(local_lcd_mp, ...)` 中使用这个空指针。即使 common symbol 合并，该变量也依赖于 main.c 的初始化顺序，极不可靠。 |
| 症状 | 段错误 (SIGSEGV) |
| 修复 | 2026-04-30 — 移除 `test.c` 的独立定义，通过 `camera.h` 的 `extern` 引用 `main.c` 中已初始化的指针 |

### C4. `program_running` 双重定义 → 死锁 ✅

| 项目 | 详情 |
|------|------|
| 文件 | `main.c:24` vs `test.c:18` |
| 类型 | 符号冲突 + 死锁 |
| 描述 | `main.c` 定义 `volatile bool program_running = true;`（全局）。`test.c` 定义 `static volatile bool program_running = true;`（文件局部）。`weather_time_thread` 的 `while(program_running)` 检查 `test.c` 的副本（永远为 true），而 `main.c` 中 `pthread_join(weather_tid, NULL)` 永远等不到线程退出。触摸线程同理使用 `main.c` 的 `program_running`（也永远为 true），双重死锁。 |
| 症状 | 程序无法正常退出，`pthread_join` 永久阻塞。电源按钮直接 `exit(0)` 暴力退出掩盖了此问题。 |
| 修复 | 2026-04-30 — 移除 `test.c` 中的 `static` 副本，`camera.h` 添加 `extern` 声明统一引用 |

### C5. YUV→RGB 整数除法截断 → 颜色完全错误 ✅

| 项目 | 详情 |
|------|------|
| 文件 | `API_Camera2.c:46-48` |
| 类型 | 整数运算错误 |
| 描述 | ```c R = (255/219)*(Y-16) + 1.402*(127/112)*(Cr-128); ``` C 语言中 `255/219 = 1`，`127/112 = 1`（整数除法截断）。实际计算公式退化为 `R = (Y-16) + 1.402*(Cr-128)`，丢失了关键的缩放因子 `255/219 ≈ 1.164` 和 `127/112 ≈ 1.134`。 |
| 修复方向 | 使用浮点字面量：`255.0/219.0` 或预计算整数查找表 |
| 修复 | 2026-04-30 — 替换为预计算的浮点系数：`1.164`, `1.596`, `0.392`, `0.813`, `2.017` |

### C6. `mainloop3()` 状态判断错误 → 录像循环从不执行 ✅

| 项目 | 详情 |
|------|------|
| 文件 | `API_Camera2.c:352` |
| 类型 | 逻辑错误 |
| 描述 | `mainloop3()` 中 `while (count-- > 0 && current_state == STATE_BACKUP)` 检查的是 `STATE_BACKUP`。但调用处（`VideoPlayer3.c:335`）先设置 `current_state = STATE_RECORD` 再调用 `mainloop3()`。条件立即为 false，循环体从不执行。对比 `mainloop()` 检查 `STATE_BACKUP` 是正确的（后视按钮设置的就是 `STATE_BACKUP`）。 |
| 影响 | 录像按钮（case 1）按下后摄像头打开但不显示/不录制任何数据 |
| 修复 | 2026-04-30 — 改为 `STATE_RECORD` |

### C7. 录像按钮从未调用 `record_video()` → 录像功能完全缺失 ✅

| 项目 | 详情 |
|------|------|
| 文件 | `VideoPlayer3.c:327-337` |
| 类型 | 功能缺失 |
| 描述 | `handleButtonPress` case 1（录像按钮）调用了 `open_device()` + `init_device()` + `start_capturing()` + `mainloop3()`，但**从未调用 `record_video()`**。`record_video()` 函数（280 行）是孤立代码，仅被 `circular_recording()` 间接调用（守护进程按钮 case 4）。 |
| 影响 | 录像功能按钮完全无效 |
| 修复 | 2026-04-30 — case 1 改为直接调用 `record_video()` |

---

## High (8 个) — ✅ 全部已修复

### H1. 天气字段 Key 与 `weather.txt` 不匹配 ✅

| 项目 | 详情 |
|------|------|
| 文件 | `get_weather.c:76-80` vs `weather.txt` |
| 类型 | 数据不匹配 |
| 描述 | 代码解析的 key → 文件实际 key 对照： |
| | `"Temperature:"` → `Temperature:` ✓ |
| | `"Humidity:"` → `Relative_Humidity:` ✗（多了 `Relative_` 前缀） |
| | `"Wind:"` → 文件中无此字段 ✗ |
| | `"Weather:"` → 文件中无此字段 ✗ |
| | `"Sky_conditions:"` → `Sky conditions:` ✗（下划线 vs 空格） |
| 影响 | 5 个字段中 4 个解析失败，均返回 "暂无数据" |
| 修复 | 2026-04-30 — 修正 key 字符串匹配 weather.txt 实际内容 |

### H2. 结构体类型不一致（`Weather` vs `weather1`） ✅

| 项目 | 详情 |
|------|------|
| 文件 | `camera.h:72-80` vs `get_weather.c:12-19` vs `test.c:21` |
| 类型 | 类型不匹配 / UB |
| 描述 | `camera.h` 定义 `weather1`（字段 `Weather[128]`），`get_weather.c` 自定义 `Weather`（字段 `weather_desc[128]`）。`test.c` 以 `extern int Get_weather(weather1 *wea_p)` 声明，但 `get_weather.c` 定义 `int Get_weather(Weather *weg_p)`。C 语言跨 TU 不检查参数类型，构成 UB。 |
| 影响 | 恰好内存布局相同（5×128 字节），当前侥幸可运行 |
| 修复 | 2026-04-30 — 统一使用 `camera.h` 中的 `weather1` 类型，移除 `get_weather.c` 的局部定义 |

### H3. `mainloop3()` 固定使用 `buffers[3]` ✅

| 项目 | 详情 |
|------|------|
| 文件 | `API_Camera2.c:393` |
| 类型 | 数组索引错误 |
| 描述 | `for(int i=0; i<n_buffers; ++i)` 循环中每次出队缓冲区 `i`，但始终调用 `yuyv_to_lcd(buffers[3].start)` 处理第 4 个缓冲区。对比 `mainloop()` 正确使用 `buffers[i].start`。 |
| 影响 | 即使状态检查修正后，画面显示也不正确 |
| 修复 | 2026-04-30 — 改为 `buffers[buf.index].start` |

### H4. `yuyv_to_bmp` YUYV 解码错误 ✅

| 项目 | 详情 |
|------|------|
| 文件 | `VideoPlayer3.c:83-92` |
| 类型 | 像素格式错误 |
| 描述 | YUYV 格式 4 字节表示 2 个像素（Y1 U Y2 V），UV 分量被相邻像素共享。代码中 `pixel_yuv2rgb(yuvbuf[i*2], yuvbuf[i*2+1], yuvbuf[i*2+3])` 对奇数像素使用了错误的 UV 值（取了下一个宏块的 U 而非共享的 U）。 |
| 对照 | `yuyv_to_lcd`（`API_Camera2.c:155-158`）正确处理了像素对（`i+=2, j+=4`）。 |
| 影响 | 保存的 BMP 图像奇数像素列颜色异常 |
| 修复 | 2026-04-30 — 重写为像素对循环，正确共享 UV 分量 |

### H5. `Bmp_Decode` 文件句柄泄漏 ✅

| 项目 | 详情 |
|------|------|
| 文件 | `op_Boot_anim.c:86-121` |
| 类型 | 资源泄漏 |
| 描述 | `fopen(bmp_path, "rb")` 后，函数正常返回 `true` 路径中缺少 `fclose(bmp_fp)`。每次调用泄漏一个 FILE 句柄。开机动画播放 104 帧即泄漏 104 个文件句柄。 |
| 修复 | 2026-04-30 — 添加 `fclose(bmp_fp)` |

### H6. `op_dve()` LCD 资源永久泄漏 ✅

| 项目 | 详情 |
|------|------|
| 文件 | `op_Boot_anim.c:46-76` |
| 类型 | 资源泄漏 |
| 描述 | 函数内定义了局部变量 `int lcd_fd;` 和 `unsigned int *lcd_mp;`，调用 `open(LCD_DEV)` 和 `mmap()` 后函数返回。局部变量离开作用域，fd 和 mmap 映射永久泄漏，且函数无返回值（声明为 `int` 但无 `return`）。 |
| 修复 | 2026-04-30 — 添加 `return 0`，mmap 失败时 `close(lcd_fd)`，标注为已弃用函数 |

### H7. `record_video()` 重复打开摄像头未先关闭 ✅

| 项目 | 详情 |
|------|------|
| 文件 | `VideoPlayer3.c:117` → `API_Camera2.c:185` |
| 类型 | 资源泄漏 |
| 描述 | `record_video()` 无条件调用 `open_device()` 打开 `/dev/video7`。如果守护进程循环调用或连续录像，每次都会打开新的 fd 而不关闭旧的。 |
| 修复 | 2026-04-30 — `open_device()` 前添加 `if (camera_fd >= 0) stop_camera();` |

### H8. `stop_camera()` 使用错误的 `camera_fd` ✅

| 项目 | 详情 |
|------|------|
| 文件 | `VideoPlayer3.c:26-35` |
| 类型 | 逻辑错误 |
| 描述 | `stop_camera()` 中的 `camera_fd` 引用 `VideoPlayer3.c` 的静态变量（值为 -1或从未被该文件设置），而实际被 `open_device()`（API_Camera2.c）打开的 fd 存储在 `API_Camera2.c` 的全局 `camera_fd` 中。`ioctl(-1, VIDIOC_STREAMOFF, ...)` 静默失败，`close(-1)` 返回 EBADF。 |
| 影响 | 摄像头流从不被停止，fd 从不关闭 |
| 修复 | 2026-04-30 — 由 C2 修复自动解决，`camera_fd` 现已统一引用全局定义 |

---

## Medium (5 个) — ✅ 全部已修复

### M1. LCD 互斥锁重复定义 → 无实际互斥保护 ✅

| 项目 | 详情 |
|------|------|
| 文件 | `main.c:23` vs `test.c:17` |
| 类型 | 并发安全 |
| 描述 | `main.c` 定义全局 `pthread_mutex_t lcd_mutex`，`test.c` 定义 `static pthread_mutex_t lcd_mutex`。main.c 的触摸线程锁的是全局锁，test.c 的天气线程锁的是文件局部锁——两个线程锁定的是**不同**的互斥锁，实际没有互斥效果。 |
| 修复 | 2026-04-30 — 移除 `test.c` 中的 `static` 副本，`camera.h` 添加 `extern` 声明 |

### M2. `mkdir()` 无法创建嵌套目录 ✅

| 项目 | 详情 |
|------|------|
| 文件 | `VideoPlayer3.c:122`, `VideoPlayer3.c:129` |
| 类型 | 健壮性 |
| 描述 | `RECORD_PATH` = `/tmp/Object_Yunx_Driving_Recorder2/Data/camera_records`，`mkdir(path, 0777)` 不会递归创建中间目录。如果 `/tmp/Object_Yunx_Driving_Recorder2/Data` 不存在，调用失败。 |
| 修复 | 2026-04-30 — 添加 `mkdir_p()` 递归创建函数，并检查返回值 |

### M3. `weather_time_thread` 忽略线程参数 ✅

| 项目 | 详情 |
|------|------|
| 文件 | `test.c:69-73` |
| 类型 | 冗余/混淆 |
| 描述 | `main.c:183` 调用 `pthread_create(&weather_tid, NULL, weather_time_thread, (void*)lcd_mp)` 传递 LCD 映射地址，但函数内 `unsigned int *local_lcd_mp = lcd_mp;` 直接使用全局变量，忽略了 `arg` 参数。 |
| 修复 | 2026-04-30 — 优先使用 `arg` 参数，NULL 时回退到全局变量 |

### M4. `yuyv_to_lcd2` 中 `"a+"` 模式 fseek 无效 ✅

| 项目 | 详情 |
|------|------|
| 文件 | `API_Camera2.c:119-123` |
| 类型 | API 误用 |
| 描述 | `fopen("./1.bmp", "a+")` 以追加模式打开，POSIX 规定追加模式下所有写入都发生在文件末尾，`fseek` 对写入位置无效。此处意图是随机位置写入 BMP 像素数据。 |
| 修复 | 2026-04-30 — 改为 `"rb"` 只读模式，添加错误检查和 `fclose` |

### M5. 天气信息仅显示温度，其他字段丢弃 ✅

| 项目 | 详情 |
|------|------|
| 文件 | `test.c:121` |
| 类型 | 功能不完整 |
| 描述 | `Get_weather()` 解析了 5 个字段到 `weather1` 结构体，但 `weather_time_thread` 只调用 `fontPrint(f, bw, 0, 0, wea.Temperature, ...)` 显示温度。湿度、风向、天气描述、天空状况均未显示。 |
| 修复 | 2026-04-30 — 新增湿度、风向、天气描述 3 个显示行 |

---

## Low (4 个) — ✅ 全部已修复

### L1. `timebuffer` 多重定义 ✅

| 项目 | 详情 |
|------|------|
| 文件 | `testtime.c:15` vs `test.c:19` |
| 类型 | 符号冲突 |
| 描述 | `testtime.c` 定义 `char timebuffer[80];`，`test.c` 也定义 `char timebuffer[80];`（非 extern）。 |
| 修复 | 2026-04-30 — 移除 `test.c` 中的重复定义，通过 `showtime.h` 的 `extern` 引用 |

### L2. 使用已弃用的 `bzero()` ✅

| 项目 | 详情 |
|------|------|
| 文件 | `main.c:152`, `VideoPlayer3.c:290` |
| 类型 | 可移植性 |
| 描述 | `bzero()` 是 POSIX-2001 弃用函数，应使用标准 C `memset(ptr, 0, size)`。 |
| 修复 | 2026-04-30 — 全部替换为 `memset()` |

### L3. `mkdir()` 返回值未检查 ✅

| 项目 | 详情 |
|------|------|
| 文件 | `VideoPlayer3.c:122, 129` |
| 类型 | 错误处理 |
| 描述 | `mkdir()` 调用后未检查返回值，创建失败时程序继续执行，后续 `sprintf`/`fopen` 可能使用无效路径。 |
| 修复 | 2026-04-30 — 由 M2 修复覆盖，所有目录创建均检查返回值 |

### L4. `system("rm -rf")` 存在安全隐患 ✅

| 项目 | 详情 |
|------|------|
| 文件 | `VideoPlayer3.c:289-292` |
| 类型 | 安全性 |
| 描述 | `circular_recording()` 使用 `sprintf(cmd, "rm -rf %s", oldest_file)` 拼接 shell 命令后调用 `system()`。若路径含特殊字符有命令注入风险。应使用 `remove()` 或 `unlink()` + `rmdir()`。 |
| 修复 | 2026-04-30 — 替换为 `opendir`/`readdir`/`remove`/`rmdir` 组合 |

---

## 跨文件符号所有权总览（修复后）

```
变量              定义位置                         其他文件引用方式
─────────────────────────────────────────────────────────────
lcd_fd            main.c (定义)                   camera.h extern
lcd_mp            main.c (定义, mmap 初始化)      camera.h extern
camera_fd         API_Camera2.c (定义)            camera.h extern
n_buffers         API_Camera2.c (定义)            camera.h extern
buffers           API_Camera2.c (定义)            camera.h extern
program_running   main.c (定义)                   camera.h extern
lcd_mutex         main.c (定义)                   camera.h extern
timebuffer        testtime.c (定义)               showtime.h extern
```

---

## 建议修复优先级（按顺序执行）

1. **C1** — 统一所有全局符号为 `extern` 声明 + 单一定义 ✅
2. **C2** — 合并 `camera_fd` 定义，移除 VideoPlayer3.c 的 static 副本 ✅
3. **C3+C4** — 移除 test.c 中重复的 `lcd_mp` 和 `program_running`，改用 extern ✅
4. **C5** — 修正 YUV→RGB 整数除法为浮点计算 ✅
5. **C6+C7** — 修正 `mainloop3()` 状态检查 + 在 case 1 中调用 `record_video()` ✅
6. **H1** — 修正 `get_weather.c` 中的 key 字符串匹配 weather.txt ✅
7. **H2** — 统一使用 `camera.h` 中的 `weather1` 类型 ✅
8. **H3~H8** — 修复 buffer 索引、YUYV 解码、资源泄漏等问题 ✅

---

> 共检出 **24 个 Bug**：Critical 7 / High 8 / Medium 5 / Low 4  
> **全部已修复 — 修复时间：2026-04-30**
