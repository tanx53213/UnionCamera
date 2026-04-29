/**
  ******************************************************************************
  * @file    esp8266_gettime.h
  * @author  FZetc飞贼
  * @version V0.0.1
  * @date    2026.01.14
  * @brief   1、通过esp8266模块获取网络时间
  *          
  ******************************************************************************
  * @attention
  *
  * 本文档只供学习使用，不得商用，违者必究
  *
  * 微信公众号：    FZetc飞贼
  * 全视频平台：    FZetc飞贼
  * CSDN博客：      https://blog.csdn.net/qq_58629108?type=blog
  * 有疑问或者建议： FZetcSnitch@163.com
  *
  ******************************************************************************
  */

#ifndef __ESP8266_GETTIME_H		// 定义以防止递归包含
#define __ESP8266_GETTIME_H


// 一、其它头文件
#include "stm32f4xx.h"   		// Device header
#include "my_config.h"
#include "my_lib.h"
#include "my_reg.h"

// 二、自定义的数据类型(结构体、联合体、枚举等)
typedef struct 
{
	// 年月日、星期
	char year[16];
	char month[16];
	char date[16];
	char weekday[16];
	
	// 时分秒
	char hour[16];
	char min[16];
	char second[16];
	
	// 世界时间
	char GMT[16];

}time_t, *time_p;


// 三、宏定义(函数、变量、常量)

// 四、变量声明

// 五、函数声明
extern int8_t ESP8266_GetTimeFunc(time_p p);

// 六、静态变量、函数定义

#endif /* __ESP8266_GETTIME_H */
