#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h>

#define WEATHER_TXTPATH "./weather.txt"

// 天气结构体
typedef struct 
{
    char Temperature[128];       // 体感温度
    char Relative_Humidity[128]; // 相对湿度
    char Wind[128];              // 风向
    char weather_desc[128];      // 天气：小雨、大雨、大小雪等
    char Sky_conditions[128];    // 天气环境：阴、晴、阴转晴，晴转阴等
} Weather;

// 通用字段解析辅助函数
static int parse_field(const char *buf, const char *key, char *out, size_t out_size)
{
    char *p = strstr(buf, key);
    if (p == NULL)
    {
        strncpy(out, "暂无数据", out_size - 1);
        out[out_size - 1] = '\0';
        return -1;
    }

    p += strlen(key); // 跳过 key 本身，不要硬编码偏移量

    int i = 0;
    while (i < (int)(out_size - 1))
    {
        if (p[i] == '\r' || p[i] == '\n' || p[i] == '\0')
            break;
        out[i] = p[i];
        i++;
    }
    out[i] = '\0';
    return 0;
}

// 获取天气信息并解析
int Get_weather(Weather *weg_p)
{
    if (weg_p == NULL)
    {
        fprintf(stderr, "Get_weather: 传入空指针\n");
        return -1;
    }

    char buf[2048] = {0};

    // 打开文件
    FILE *fp = fopen(WEATHER_TXTPATH, "rb");
    if (fp == NULL)
    {
        perror("Get_weather: fopen 失败");
        return -1;
    }

    // 读取文件
    size_t ret = fread(buf, 1, sizeof(buf) - 1, fp);
    fclose(fp); // ← 无论成功与否都要关闭，放在 fread 后立即关闭
    if (ret == 0)
    {
        perror("Get_weather: fread 失败");
        return -2;
    }
    buf[ret] = '\0';

    // 解析各字段（key 要与 weather.txt 里的格式完全一致）
    parse_field(buf, "Temperature:",      weg_p->Temperature,      sizeof(weg_p->Temperature));
    parse_field(buf, "Humidity:",         weg_p->Relative_Humidity,sizeof(weg_p->Relative_Humidity));
    parse_field(buf, "Wind:",             weg_p->Wind,             sizeof(weg_p->Wind));
    parse_field(buf, "Weather:",          weg_p->weather_desc,     sizeof(weg_p->weather_desc));
    parse_field(buf, "Sky_conditions:",   weg_p->Sky_conditions,   sizeof(weg_p->Sky_conditions));

    return 0; // ← 修复：加上返回值，原来缺少这一行导致 segfault
}








