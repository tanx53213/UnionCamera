#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h>
#include "camera.h"

// WEATHER_TXTPATH 已在 camera.h 中统一定义为 "./weather.txt"

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
int Get_weather(weather1 *weg_p)
{
    if (weg_p == NULL)
    {
        fprintf(stderr, "Get_weather: 传入空指针\n");
        return -1;
    }

    char buf[2048] = {0};
    char weather_path[256] = {0};

    // 优先使用 resolve_asset_path 查找 weather.txt，回退到 WEATHER_TXTPATH
    resolve_asset_path(weather_path, sizeof(weather_path), "weather.txt");
    FILE *fp = fopen(weather_path, "rb");
    if (fp == NULL)
    {
        // 回退到当前目录
        fp = fopen(WEATHER_TXTPATH, "rb");
    }
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
    parse_field(buf, "Temperature:",         weg_p->Temperature,         sizeof(weg_p->Temperature));
    parse_field(buf, "Relative_Humidity:",   weg_p->Relative_Humidity,   sizeof(weg_p->Relative_Humidity));
    parse_field(buf, "Wind:",                weg_p->Wind,                sizeof(weg_p->Wind));
    parse_field(buf, "Weather:",             weg_p->Weather,             sizeof(weg_p->Weather));
    parse_field(buf, "Sky conditions:",      weg_p->Sky_conditions,      sizeof(weg_p->Sky_conditions));

    return 0; // ← 修复：加上返回值，原来缺少这一行导致 segfault
}








