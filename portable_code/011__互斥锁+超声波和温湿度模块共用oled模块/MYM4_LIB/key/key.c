#include "key.h"
#include "buzzer.h"
#include "delay.h"
#include "iic_oled.h"
#include "iic_oled_bmp.h"
#include "iic_oled_font.h"
#include <stdio.h>


/**
  * @brief  初始化key按键
  * @note   GEC-M4板子：
  *				S1(丝印层)->KEY0(网络标号)->PA0(引脚标号)
  *				S2(丝印层)->KEY1(网络标号)->PE2(引脚标号)
  *				S3(丝印层)->KEY2(网络标号)->PE3(引脚标号)
  *				S4(丝印层)->KEY3(网络标号)->PE4(引脚标号)
  * @param  None 
  * @retval None
  */
void KEY_Init(void)
{
	// 0、GPIO片内外设信息初始化结构体
	GPIO_InitTypeDef GPIO_InitStructure;
	
	// 1、使能GPIO片内外设的硬件时钟					
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
	
	// 2、配置GPIO片内外设的引脚						
	GPIO_InitStructure.GPIO_Pin			= GPIO_Pin_0;					// 引脚：第0根引脚
	GPIO_InitStructure.GPIO_Mode		= GPIO_Mode_IN;					// 模式：输入模式
	// GPIO_InitStructure.GPIO_Speed  	= GPIO_High_Speed;				// 速度：高速(100MHz)
	GPIO_InitStructure.GPIO_PuPd		= GPIO_PuPd_NOPULL;				// 上下拉：不拉
	// GPIO_InitStructure.GPIO_OType   	= GPIO_OType_PP;				// 输出类型：推挽输出
	GPIO_Init(GPIOA, &GPIO_InitStructure);								// 使用该函数，将配置的信息写入到相应的寄存器中
	
	GPIO_InitStructure.GPIO_Pin		= GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4;	// 引脚：第2、3、4根引脚
	GPIO_Init(GPIOE, &GPIO_InitStructure);								// 使用该函数，将配置的信息写入到相应的寄存器中
}

