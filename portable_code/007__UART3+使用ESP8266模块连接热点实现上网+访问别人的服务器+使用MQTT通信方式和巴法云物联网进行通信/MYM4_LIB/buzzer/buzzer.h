/**
  ******************************************************************************
  * @file    buzzer.h
  * @author  FZetc飞贼
  * @version V0.0.1
  * @date    2025.12.26
  * @brief   1、初始化buzzer蜂鸣器模块
  *          2、指定buzzer蜂鸣器的响和不响
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

#ifndef __BUZZER_H	
#define __BUZZER_H

// 一、其它头文件
#include "stm32f4xx.h" 
#include "my_config.h"
#include "my_lib.h"
#include "my_reg.h"

// 二、宏定义(函数、变量、常量)
#define BUZZER(X) (X)?(PFout(8)=1): (PFout(8)=0)
 
// 三、自定义的数据类型(结构体、联合体、枚举等)

// 四、全局变量声明

// 五、函数声明
extern void BUZZER_Init(void);
extern void BUZZER_TipVoice(uint16_t speed);
extern void BUZZER_WarnVoice(uint16_t speed, uint16_t count);

// 六、静态变量、函数定义

#endif /* __BUZZER_H */

