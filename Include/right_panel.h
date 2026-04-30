#ifndef __RIGHT_PANEL_H__
#define __RIGHT_PANEL_H__

#include "camera.h"

// 右侧面板几何参数 (列 640-799, 160px × 480px)
#define PANEL_X         640
#define PANEL_WIDTH     160
#define PANEL_Y          0
#define PANEL_HEIGHT   480

// 面板内分区
#define PANEL_TOP_H      95    // 顶端区: 时间+天气 (y: 0-94)
#define PANEL_MID_Y     100    // 中部区: 超声波数据
#define PANEL_MID_H     280    // 中部区高度
#define PANEL_BOT_Y     400    // 底部区: 电源键

// 超声波数据结构
typedef struct {
    float distance_mm;
    bool  data_valid;
    int   buzzer_state;        // 0=静音, 1=慢警告(150-300mm), 2=快警告(<=100mm)
    char  status_text[32];
} ultrasonic_data_t;

// 右侧面板全局数据 (天气线程写 time/weather, RS485线程写 ultrasonic)
typedef struct {
    char   time_str[80];
    char   temperature[128];
    char   humidity[128];
    char   wind[128];
    char   weather[128];
    ultrasonic_data_t ultrasonic;
} right_panel_data_t;

extern right_panel_data_t g_panel_data;
extern pthread_mutex_t    g_panel_mutex;

// 初始化右侧面板 (绘制边框、分隔线、固定标签) — 需持有 lcd_mutex
void right_panel_init(unsigned int *lcd_mp);

// 绘制右侧面板动态内容 (时间/天气/超声波/电源键) — 需持有 g_panel_mutex
void right_panel_draw_all(unsigned int *lcd_mp, font *f);

// 仅绘制超声波数据区域 (使用字库渲染大号数字+中文状态)
void right_panel_draw_ultrasonic(unsigned int *lcd_mp, font *f);

// RS485 超声波读取线程
void *rs485_ultrasonic_thread(void *arg);

#endif
