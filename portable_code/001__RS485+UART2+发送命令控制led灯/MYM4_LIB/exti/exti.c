#include "exti.h"
#include "key.h"
#include "buzzer.h"
#include "led.h"
#include "delay.h"

/**
  * @brief  初始化按键触发的外部中断
  * @note   GEC-M4板子：
  *				S1(丝印层)->KEY0(网络标号)->PA0(引脚标号)->EXTI0线
  *				S2(丝印层)->KEY1(网络标号)->PE2(引脚标号)->EXTI2线
  *				S3(丝印层)->KEY2(网络标号)->PE3(引脚标号)->EXTI3线
  *				S4(丝印层)->KEY3(网络标号)->PE4(引脚标号)->EXTI4线
  * @param  None 
  * @retval None
  */
void EXTI0234_Init(void)
{
	EXTI_InitTypeDef  EXTI_InitStructure;
	NVIC_InitTypeDef  NVIC_InitStructure;
	
	// 1、配置引脚为输入浮动模式
	KEY_Init();
	
	// 2、将GPIO引脚连接到相应的EXTI中
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);							// 使用SYSCFG硬件使用
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource0);					// EXTI线被PA0使用了
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource2);					// EXTI线被PE2使用了
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource3);					// EXTI线被PE3使用了
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource4);					// EXTI线被PE4使用了
	
	
	// 3、配置外部中断
	EXTI_InitStructure.EXTI_Line 	= EXTI_Line0|EXTI_Line2|EXTI_Line3|EXTI_Line4;	// 外部中断：0线
	EXTI_InitStructure.EXTI_Mode	= EXTI_Mode_Interrupt;							// 模式：中断模式
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;							// 触发方式：下降沿触发
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;										// 使能外部中断
	EXTI_Init(&EXTI_InitStructure);													// 使用该函数，将配置的信息写入到相应的寄存器中
	
	// 4、配置时间的优先等级
	// EXTI0线
	NVIC_InitStructure.NVIC_IRQChannel					 = EXTI0_IRQn;				// 中断请求通道：外部中断0通道	
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;					// 抢占式优先级：0x00(0)	
	NVIC_InitStructure.NVIC_IRQChannelSubPriority		 = 0x02;					// 响应式优先级：0x00(0)	
	NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;					// 使能NVIC相应的中断请求通道	
	NVIC_Init(&NVIC_InitStructure);													// 使用该函数，将配置的信息写入到相应的寄存器中		

	// EXTI2线
	NVIC_InitStructure.NVIC_IRQChannel					 = EXTI2_IRQn;				
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;					
	NVIC_InitStructure.NVIC_IRQChannelSubPriority		 = 0x02;					
	NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;					
	NVIC_Init(&NVIC_InitStructure);													


	// EXTI3线
	NVIC_InitStructure.NVIC_IRQChannel					 = EXTI3_IRQn;				
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;					
	NVIC_InitStructure.NVIC_IRQChannelSubPriority		 = 0x00;					
	NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;					
	NVIC_Init(&NVIC_InitStructure);													


	// EXTI4线
	NVIC_InitStructure.NVIC_IRQChannel					 = EXTI4_IRQn;				
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;					
	NVIC_InitStructure.NVIC_IRQChannelSubPriority		 = 0x00;					
	NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;					
	NVIC_Init(&NVIC_InitStructure);													


}




/**
  * @brief  外部中断0的中断服务函数
  * @note   S1(丝印层)->KEY0(网络标号)->PA0(引脚标号)->EXTI0线
  * @param  None 
  * @retval None
  */
void EXTI0_IRQHandler(void)		
{

	if( EXTI_GetITStatus(EXTI_Line0) == SET)		// 判断是否触发了外部中断0线
	{
		DELAY_MyNms(10000);	
		LED1(ON);
		EXTI_ClearITPendingBit(EXTI_Line0);			// 清除外部中断0线的中断挂起标志位	
	}		
}


/**
  * @brief  外部中断2的中断服务函数
  * @note   S2(丝印层)->KEY1(网络标号)->PE2(引脚标号)->EXTI2线
  * @param  None 
  * @retval None
  */
void EXTI2_IRQHandler(void)		
{

	if( EXTI_GetITStatus(EXTI_Line2) == SET)		
	{
		DELAY_MyNms(20000);	
		LED2(ON);
		EXTI_ClearITPendingBit(EXTI_Line2);			
	}		
}



/**
  * @brief  外部中断4的中断服务函数
  * @note   S4(丝印层)->KEY3(网络标号)->PE4(引脚标号)->EXTI4线
  * @param  None 
  * @retval None
  */
void EXTI4_IRQHandler(void)		
{

	if( EXTI_GetITStatus(EXTI_Line4) == SET)		
	{
		DELAY_MyNms(20000);	
		LED4(ON);
		EXTI_ClearITPendingBit(EXTI_Line4);			
	}		
}















