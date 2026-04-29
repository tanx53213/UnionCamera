/**
  ******************************************************************************
  * @file    exti.h
  * @author  FZetc飞贼
  * @version V0.0.1
  * @date    2026.01.06
  * @brief   1、uart初始化
  *          2、uart中断接收函数
  *          3、uart发送数据
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

#ifndef __UART_H	
#define __UART_H

// 一、其它头文件
#include <stdio.h>
#include "stm32f4xx.h" 
#include "my_config.h"
#include "my_lib.h"
#include "my_reg.h"

// 二、宏定义(函数、变量、常量)
#define UART2_RUN 	0					// UART2接收的数据，回发到UART2(连接蓝牙的手机)上
#define UART2_DEBUG 1					// UART2接收的数据，回发到UART1(连接a-a口线的电脑)上
#define UART3_ESP8266_RECV_MSG_DEBUG 0	// UART3接收的数据，会发到UART1(连接a-a口线的电脑)上

#define RX1_BUF_SIZE 256
#define RX2_BUF_SIZE 256
#define RX3_BUF_SIZE 1024

#define TX1_BUF_SIZE 256
#define TX2_BUF_SIZE 256
#define TX3_BUF_SIZE 1024

// 三、自定义的数据类型(结构体、联合体、枚举等)

// 四、全局变量声明
extern volatile uint8_t u1_recvbuf[RX1_BUF_SIZE];
extern volatile uint8_t u1_count;
extern volatile uint8_t u1_flag;

extern volatile uint8_t u2_recvbuf[RX2_BUF_SIZE];
extern volatile uint8_t u2_count;
extern volatile uint8_t u2_flag;

extern volatile uint8_t  u3_recvbuf[RX3_BUF_SIZE];
extern volatile uint8_t  u3_sendbuf[TX3_BUF_SIZE];
extern volatile uint16_t u3_count;
extern volatile uint16_t u3_len;
extern volatile uint8_t  u3_flag;


// 五、函数声明
extern void UART1_Init(uint32_t baudrate);
extern void UART2_Init(uint32_t baudrate);
extern void UART3_Init(uint32_t baudrate);

extern void UART1_SendStr(const char* str);
extern void UART2_SendStr(const char* str);
extern void UART3_SendStr(const char* str);
extern void UART3_SendBytes(uint8_t *buf,uint32_t len);

extern void USART1_IRQHandler(void);
extern void USART2_IRQHandler(void);
extern void USART3_IRQHandler(void);

extern void UART_BlueTooth_GetMsg(void);
extern void UART_BlueTooth_SetMsg(const char* name, const char* pin, const char* baud);

// 六、静态变量、函数定义

#endif /* __UART_H */

