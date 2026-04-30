/**
  ******************************************************************************
  * @file    rs485.h
  * @author  FZetc飞贼
  * @version V0.0.1
  * @date    2026.01.06
  * @brief   1、rs485初始化
  *          2、rs485中断接收函数
  *          3、rs485发送数据
  * 
  ******************************************************************************
  * @attention
  *
  * 本文档只供学习使用，不得商用，违者必究
  *
  * 微信公众号：    FZetc飞贼
  * 全视频平台：    FZetc飞贼
  * CSDN博客：      https://blog.csdn.net/qq_58629108?type=blog
  * 有疑问或者建议：FZetcSnitch@163.com
  *
  ******************************************************************************
  */

#ifndef __RS485_H	
#define __RS485_H

// 一、其它头文件
#include <stdio.h>
#include "stm32f4xx.h" 
#include "my_config.h"
#include "my_lib.h"


// 二、宏定义(函数、变量、常量)
// RS485的模式控制
#define RS485_TX_EN PGout(8)	// 485模式控制引脚宏定义：PG8输出，0=接收模式， 1=发送模式

// 三、自定义的数据类型(结构体、联合体、枚举等)

// 四、全局变量声明


// 五、函数声明
extern void RS485_Init(uint32_t baudrate);
extern void RS485_SendStr(uint8_t* pbuf, uint32_t len);		
extern void USART2_IRQHandler(void);

// 六、静态变量、函数定义

#endif /* __RS485_H */

