/**
  ******************************************************************************
  * @file    main.c
  * @author  FZetc飞贼
  * @version V0.0.1
  * @date    2026.04.22
  * @brief   说明：
  *				RS485+UART2+发送命令控制led灯
  *
  *          操作：连接RS485模块
  *          	GEC-M4板子：                       RS485模块：
  *         	RS485-A     						A口
  *         	RS485-B     						B口
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
  
#include "main.h"
#include "rs485.h"
#include "hc_sr04.h"

// 全局变量声明
extern volatile uint8_t u2_rs485_recvbuf[128];
extern volatile uint8_t u2_rs485_count;
extern volatile uint8_t u2_rs485_flag;


// 主函数
int main(void)
{
	
	// 零、外设相关变量区域 
		
	double hc_sr04_dis = 0;		
	uint8_t d = 'A';
	char buf[32];
	// 一、外设初始化区域
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); 		// 选择中断分组为第2组(抢占优先级：4级， 响应优先级：4级)
	DELAY_SysTickInit(168);									// 系统定时器的时钟源初始化(168MHz或21MHz)
	UART1_Init(115200);										// 串口a-a口线初始化
	
	LED_Init();
	KEY_Init();
	BUZZER_Init();
		HC_SR04_Init();										// 超声波hc_sr04模块初始化
	printf("This is hc_sr04 test!\r\n");	

	RS485_Init(9600);										// rs486初始化
	
	// 二、外设功能具现化区域
	while(1)												
	{
		
		//RS485_SendStr(&d, 1);								// 单片机通过rs485发送数据给串口助手
		//delay_ms(1000);
		   hc_sr04_dis = HC_SR04_GetDis(3.5);

// 过滤掉无效值（HC-SR04有效范围20mm~4000mm）
if((hc_sr04_dis >= 20) && (hc_sr04_dis <= 4000))
{
    printf("hc_sr04 == %0.2fmm\r\n", hc_sr04_dis);
    
    // 发送格式：D278\r\n  (单位mm，整数)
    sprintf(buf, "%.0fmm\r\n", hc_sr04_dis);
    RS485_SendStr((uint8_t*)buf, strlen(buf));
}
else
{
    printf("out of range: %0.2f\r\n", hc_sr04_dis);  // 调试用，不发RS485
}
    
    // 距离报警逻辑（单位：mm）
    if(hc_sr04_dis <= 100)  // ≤ 10cm = 100mm，急促报警
    {
        BUZZER_WarnVoice(50, 5);   // 50ms间隔，响5次，急促
    }
    else if((hc_sr04_dis >= 150) && (hc_sr04_dis <= 300))  // 15~30cm，一般警告
    {
        BUZZER_WarnVoice(200, 2);  // 200ms间隔，响2次，缓慢
    }
    
    delay_ms(100);
		
	}
}






