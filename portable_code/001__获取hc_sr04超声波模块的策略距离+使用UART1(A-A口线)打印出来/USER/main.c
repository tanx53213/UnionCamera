/**
 ******************************************************************************
 * @file    main.c
 * @author  FZetc飞贼
 * @version V0.0.1
 * @date    2026.03.30
 * @brief   说明：
 *				获取hc_sr04超声波模块的策略距离+使用UART1(A-A口线)打印出来
  *         步骤1：
  *            1、保证stm32f4xx.h文件123行的HSE_VALUE时钟源值为8000000，保证system_stm32f4xx.c文件的316行的PLL_M系数为8
  *			   2、GEC-M4板子上丝印层UART1跳线帽处，需要短接1-3和2-4，A-A口线要连接电脑和单片机USB口，保证调试通信和5V供电
  *            3、串口助手的端口要打开连接开发板的那个串口端口，串口助手的波特率要和单片机串口程序保持一致
  *            4、保证keil5软件和串口助手都是一个编码格式(推荐：GB2312或utf-8格式)
  *			步骤2：连接超声波模块到开发板上
  *				CMAERA(13引脚，丝印层)-> DCMI_D5(网络标号) ->PB6(芯片引脚标号)  ===连接=== 超声波模块(TRIG引脚)
  * 			CMAERA(15引脚，丝印层)-> DCMI_D7(网络标号) ->PE6(芯片引脚标号)  ===连接=== 超声波模块(ECHO引脚)
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
#include "hc_sr04.h"



/**
  * @brief  主函数
  * @note   None
  * @param  None
  * @retval None
  */
int main(void)
{

	// 一、外设相关变量区域
	double hc_sr04_dis = 0;			 
							 
	
	// 二、外设初始化区域
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);		// 选择中断分组为第2组(抢占式优先级：4级， 响应式优先级：4级)
	DELAY_SysTickInit(168);								// 系统定时器的时钟源初始化(168MHZ或21MHZ)
	UART1_Init(115200);									// 串口1初始化为115200
	
	LED_Init();
	BUZZER_Init();//暂时关闭or开启
	KEY_Init();

	HC_SR04_Init();										// 超声波hc_sr04模块初始化
	printf("This is hc_sr04 test!\r\n");				// 下位机(单片机->上位机(电脑、手机端的串口助手))
	
// 二、外设功能具现化区域
while(1)										
{
    hc_sr04_dis = HC_SR04_GetDis(3.5);
    
    if((hc_sr04_dis > 20) && (hc_sr04_dis < 4000))
    {
        printf("hc_sr04 == %0.2fmm\r\n", hc_sr04_dis);
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














