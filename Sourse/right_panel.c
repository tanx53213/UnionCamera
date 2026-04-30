/******************************************************************************
 *  right_panel.c — 右侧信息栏模块
 *   布局: 列 640-799 (160px × 480px)
 *   顶端 (0-94):     时间 / 温度 / 湿度 / 天气 / 风向
 *   中部 (100-380):   超声波距离 + 柱状图 + 状态
 *   底部 (400-480):   电源键
 *
 *   RS485 协议: STM32 → 6818, 9600-8N1, ASCII "<distance>mm\r\n"
 ******************************************************************************/

#include "right_panel.h"
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

// 全局数据实例
right_panel_data_t g_panel_data = {0};
pthread_mutex_t    g_panel_mutex = PTHREAD_MUTEX_INITIALIZER;

// ─── 基础像素绘制辅助 ────────────────────────────────────────────────

// 填充矩形
static void fill_rect(unsigned int *lcd, int x0, int y0, int w, int h, unsigned int color)
{
    for (int y = y0; y < y0 + h && y < 480; y++)
        for (int x = x0; x < x0 + w && x < 800; x++)
            lcd[y * 800 + x] = color;
}

// 绘制水平线
static void draw_hline(unsigned int *lcd, int x0, int y, int w, unsigned int color)
{
    if (y < 0 || y >= 480) return;
    for (int x = x0; x < x0 + w && x < 800; x++)
        lcd[y * 800 + x] = color;
}

// 绘制垂直线
static void draw_vline(unsigned int *lcd, int x, int y0, int h, unsigned int color)
{
    if (x < 0 || x >= 800) return;
    for (int y = y0; y < y0 + h && y < 480; y++)
        lcd[y * 800 + x] = color;
}

// ─── 初始化 ──────────────────────────────────────────────────────────

void right_panel_init(unsigned int *lcd_mp)
{
    // 面板背景 (深灰色)
    fill_rect(lcd_mp, PANEL_X, 0, PANEL_WIDTH, PANEL_HEIGHT, 0x00303030);

    // 左边框线 (分隔摄像头和右侧栏)
    draw_vline(lcd_mp, PANEL_X, 0, PANEL_HEIGHT, 0x00FFFFFF);

    // 分隔线: 顶端区 / 中部区
    draw_hline(lcd_mp, PANEL_X + 2, PANEL_TOP_H, PANEL_WIDTH - 4, 0x00555555);

    // 分隔线: 中部区 / 底部区
    draw_hline(lcd_mp, PANEL_X + 2, PANEL_BOT_Y, PANEL_WIDTH - 4, 0x00555555);

    // 中部标题背景
    fill_rect(lcd_mp, PANEL_X + 2, PANEL_MID_Y, PANEL_WIDTH - 4, 22, 0x00004488);
}

// ─── 动态绘制 (全部) ──────────────────────────────────────────────────

void right_panel_draw_all(unsigned int *lcd_mp, font *f)
{
    if (lcd_mp == NULL || f == NULL) return;

    // 清除顶端区内容区域
    fill_rect(lcd_mp, PANEL_X + 2, 2, PANEL_WIDTH - 4, PANEL_TOP_H - 4, 0x00303030);

    // 使用字体库绘制时间/天气 (含中文标签)
    fontSetSize(f, 16);

    char label_buf[256];

    // 时间 (y=4)
    bitmap *bm_time = createBitmapWithInit(PANEL_WIDTH - 8, 16, 4, 0x00303030);
    if (bm_time && g_panel_data.time_str[0])
    {
        fontPrint(f, bm_time, 0, 0, g_panel_data.time_str, getColor(0, 255, 255, 255), 0);
        show_font_to_lcd(lcd_mp, PANEL_X + 4, 4, bm_time);
        destroyBitmap(bm_time);
    }

    // 温度 (y=24) — 中文标签
    bitmap *bm_t = createBitmapWithInit(PANEL_WIDTH - 8, 16, 4, 0x00303030);
    if (bm_t)
    {
        snprintf(label_buf, sizeof(label_buf), "温度 %s", g_panel_data.temperature);
        fontPrint(f, bm_t, 0, 0, label_buf, getColor(0, 0, 255, 0), 0);
        show_font_to_lcd(lcd_mp, PANEL_X + 4, 24, bm_t);
        destroyBitmap(bm_t);
    }

    // 湿度 (y=42) — 中文标签
    bitmap *bm_h = createBitmapWithInit(PANEL_WIDTH - 8, 16, 4, 0x00303030);
    if (bm_h && g_panel_data.humidity[0])
    {
        snprintf(label_buf, sizeof(label_buf), "湿度 %s", g_panel_data.humidity);
        fontPrint(f, bm_h, 0, 0, label_buf, getColor(0, 0, 255, 255), 0);
        show_font_to_lcd(lcd_mp, PANEL_X + 4, 42, bm_h);
        destroyBitmap(bm_h);
    }

    // 天气 (y=60) — 中文标签
    bitmap *bm_w = createBitmapWithInit(PANEL_WIDTH - 8, 16, 4, 0x00303030);
    if (bm_w)
    {
        snprintf(label_buf, sizeof(label_buf), "天气 %s", g_panel_data.weather);
        fontPrint(f, bm_w, 0, 0, label_buf, getColor(0, 255, 255, 0), 0);
        show_font_to_lcd(lcd_mp, PANEL_X + 4, 60, bm_w);
        destroyBitmap(bm_w);
    }

    // 风向 (y=78) — 中文标签
    bitmap *bm_wd = createBitmapWithInit(PANEL_WIDTH - 8, 16, 4, 0x00303030);
    if (bm_wd)
    {
        snprintf(label_buf, sizeof(label_buf), "风向 %s", g_panel_data.wind);
        fontPrint(f, bm_wd, 0, 0, label_buf, getColor(0, 200, 200, 200), 0);
        show_font_to_lcd(lcd_mp, PANEL_X + 4, 78, bm_wd);
        destroyBitmap(bm_wd);
    }

    // 绘制超声波区
    right_panel_draw_ultrasonic(lcd_mp, f);
}

// ─── 超声波数据显示 (字库渲染) ─────────────────────────────────────────

void right_panel_draw_ultrasonic(unsigned int *lcd_mp, font *f)
{
    int px = PANEL_X;
    int py = PANEL_MID_Y;

    // 中部区域背景
    fill_rect(lcd_mp, px + 2, py, PANEL_WIDTH - 4, PANEL_BOT_Y - py - 2, 0x00303030);

    // 标题栏
    fill_rect(lcd_mp, px + 2, py, PANEL_WIDTH - 4, 22, 0x00004488);

    fontSetSize(f, 14);
    bitmap *bm_title = createBitmapWithInit(PANEL_WIDTH - 8, 18, 4, 0x00004488);
    if (bm_title)
    {
        fontPrint(f, bm_title, 0, 0, "倒车测距", getColor(0, 255, 255, 255), 0);
        show_font_to_lcd(lcd_mp, px + 8, py + 2, bm_title);
        destroyBitmap(bm_title);
    }

    if (!g_panel_data.ultrasonic.data_valid)
    {
        // 无数据
        fontSetSize(f, 14);
        bitmap *bm_na = createBitmapWithInit(PANEL_WIDTH - 8, 18, 4, 0x00303030);
        if (bm_na)
        {
            fontPrint(f, bm_na, 0, 0, "等待数据...", getColor(0, 128, 128, 128), 0);
            show_font_to_lcd(lcd_mp, px + 8, py + 45, bm_na);
            destroyBitmap(bm_na);
        }
        fontSetSize(f, 16);
        return;
    }

    int dist = (int)g_panel_data.ultrasonic.distance_mm;
    char buf[64];

    // 距离大字号数字 (36px)
    fontSetSize(f, 36);
    snprintf(buf, sizeof(buf), "%d", dist);
    bitmap *bm_dist = createBitmapWithInit(PANEL_WIDTH - 8, 42, 4, 0x00303030);
    if (bm_dist)
    {
        unsigned int dist_color = 0x0000FF00;
        if (dist <= 100)
            dist_color = getColor(0, 255, 0, 0);
        else if (dist <= 300)
            dist_color = getColor(0, 255, 204, 0);

        fontPrint(f, bm_dist, 0, 0, buf, dist_color, 0);
        show_font_to_lcd(lcd_mp, px + 4, py + 30, bm_dist);
        destroyBitmap(bm_dist);
    }

    // 单位 "mm" (18px, 在大数字右侧)
    fontSetSize(f, 18);
    bitmap *bm_unit = createBitmapWithInit(60, 20, 4, 0x00303030);
    if (bm_unit)
    {
        fontPrint(f, bm_unit, 0, 0, "mm", getColor(0, 200, 200, 200), 0);
        show_font_to_lcd(lcd_mp, px + PANEL_WIDTH - 50, py + 50, bm_unit);
        destroyBitmap(bm_unit);
    }

    // 柱状图 (y=78, 高 16px, 满刻度 4000mm)
    int bar_y = py + 78;
    int bar_w = PANEL_WIDTH - 12;
    fill_rect(lcd_mp, px + 6, bar_y, bar_w, 16, 0x00111111);
    float ratio = (float)dist / 4000.0f;
    if (ratio > 1.0f) ratio = 1.0f;
    int fill_w = (int)(bar_w * ratio);

    unsigned int bar_color = 0x0000FF00;
    if (dist <= 100)
        bar_color = 0x00FF0000;
    else if (dist <= 300)
        bar_color = 0x00FFCC00;

    fill_rect(lcd_mp, px + 6, bar_y, fill_w, 16, bar_color);

    // 状态文本 — 中文 (20px, y=102)
    fontSetSize(f, 20);
    const char *status_str;
    unsigned int status_color;
    unsigned int status_bg = 0x00303030;
    if (dist <= 100)
    {
        status_str   = "危险";
        status_color = getColor(0, 255, 0, 0);
        status_bg    = 0x00330000;
    }
    else if (dist <= 300)
    {
        status_str   = "进入警戒值";
        status_color = getColor(0, 255, 204, 0);
        status_bg    = 0x00332200;
    }
    else
    {
        status_str   = "安全";
        status_color = getColor(0, 0, 255, 0);
        status_bg    = 0x00003300;
    }

    fill_rect(lcd_mp, px + 4, py + 102, PANEL_WIDTH - 8, 24, status_bg);
    bitmap *bm_st = createBitmapWithInit(PANEL_WIDTH - 8, 24, 4, status_bg);
    if (bm_st)
    {
        fontPrint(f, bm_st, 0, 0, (char *)status_str, status_color, 0);
        show_font_to_lcd(lcd_mp, px + 8, py + 104, bm_st);
        destroyBitmap(bm_st);
    }

    // 蜂鸣器指示器 (y=135)
    fill_rect(lcd_mp, px + 2, py + 135, PANEL_WIDTH - 4, 2, 0x00555555);
    unsigned int buzzer_color = 0x00666666;
    const char *buzzer_label = "静音";
    if (g_panel_data.ultrasonic.buzzer_state == 1)
    {
        buzzer_color = 0x00FFCC00;
        buzzer_label = "慢警告";
    }
    else if (g_panel_data.ultrasonic.buzzer_state == 2)
    {
        buzzer_color = 0x00FF0000;
        buzzer_label = "快警告";
    }

    fill_rect(lcd_mp, px + 8, py + 142, 12, 12, buzzer_color);

    fontSetSize(f, 14);
    bitmap *bm_buzz = createBitmapWithInit(PANEL_WIDTH - 30, 16, 4, 0x00303030);
    if (bm_buzz)
    {
        fontPrint(f, bm_buzz, 0, 0, (char *)buzzer_label, getColor(0, 200, 200, 200), 0);
        show_font_to_lcd(lcd_mp, px + 26, py + 140, bm_buzz);
        destroyBitmap(bm_buzz);
    }

    // 恢复默认字体大小
    fontSetSize(f, 16);
}

// ─── RS485 超声波读取线程 ─────────────────────────────────────────────

#define RS485_DEV   "/dev/ttyUSB0"
#define RS485_BAUD  B9600

void *rs485_ultrasonic_thread(void *arg)
{
    (void)arg;

    // 打开串口
    int fd = open(RS485_DEV, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0)
    {
        fprintf(stderr, "RS485: 无法打开 %s, errno=%d (%s)\n",
                RS485_DEV, errno, strerror(errno));
        return NULL;
    }

    // 配置串口参数
    struct termios opts;
    if (tcgetattr(fd, &opts) != 0)
    {
        fprintf(stderr, "RS485: tcgetattr 失败\n");
        close(fd);
        return NULL;
    }

    cfsetispeed(&opts, RS485_BAUD);
    cfsetospeed(&opts, RS485_BAUD);
    opts.c_cflag |= (CLOCAL | CREAD);
    opts.c_cflag &= ~PARENB;   // 无校验
    opts.c_cflag &= ~CSTOPB;   // 1 停止位
    opts.c_cflag &= ~CSIZE;
    opts.c_cflag |= CS8;       // 8 数据位
    opts.c_cflag &= ~CRTSCTS;  // 无硬件流控
    opts.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    opts.c_iflag &= ~(IXON | IXOFF | IXANY);
    opts.c_oflag &= ~OPOST;

    // 读取超时: 0.5 秒
    opts.c_cc[VMIN]  = 0;
    opts.c_cc[VTIME] = 5;  // 0.5s

    if (tcsetattr(fd, TCSANOW, &opts) != 0)
    {
        fprintf(stderr, "RS485: tcsetattr 失败\n");
        close(fd);
        return NULL;
    }

    printf("RS485: 超声波串口 %s 已打开 (9600-8N1)\n", RS485_DEV);

    char   rbuf[64] = {0};
    int    rpos = 0;

    while (program_running)
    {
        unsigned char c;
        int n = read(fd, &c, 1);
        if (n <= 0)
        {
            usleep(50000);  // 50ms 等待
            continue;
        }

        // 收集字符直到 \n
        if (c == '\n' && rpos > 0)
        {
            rbuf[rpos] = '\0';

            // 解析: "<distance>mm" 格式
            float dist = 0.0f;
            if (sscanf(rbuf, "%fmm", &dist) == 1)
            {
                pthread_mutex_lock(&g_panel_mutex);
                g_panel_data.ultrasonic.distance_mm = dist;
                g_panel_data.ultrasonic.data_valid   = true;

                // 蜂鸣器状态判断
                if (dist <= 100.0f)
                {
                    g_panel_data.ultrasonic.buzzer_state = 2;
                    snprintf(g_panel_data.ultrasonic.status_text, 
                             sizeof(g_panel_data.ultrasonic.status_text),
                             "危险! %.0fmm", dist);
                }
                else if (dist <= 300.0f)
                {
                    g_panel_data.ultrasonic.buzzer_state = 1;
                    snprintf(g_panel_data.ultrasonic.status_text,
                             sizeof(g_panel_data.ultrasonic.status_text),
                             "警告 %.0fmm", dist);
                }
                else
                {
                    g_panel_data.ultrasonic.buzzer_state = 0;
                    snprintf(g_panel_data.ultrasonic.status_text,
                             sizeof(g_panel_data.ultrasonic.status_text),
                             "安全 %.0fmm", dist);
                }
                pthread_mutex_unlock(&g_panel_mutex);
            }

            rpos = 0;
        }
        else if (c != '\r' && rpos < (int)sizeof(rbuf) - 1)
        {
            rbuf[rpos++] = (char)c;
        }
    }

    close(fd);
    return NULL;
}
