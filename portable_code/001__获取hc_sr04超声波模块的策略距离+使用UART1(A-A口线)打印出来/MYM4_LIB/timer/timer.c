#include "timer.h"
#include "led.h"

static uint32_t tim13_cnt = 0;

/**
  * @brief  初始化定时器3片内外设
  * @note   None
  * @param  None
  * @retval None
  */
void TIMER3_Init(void)
{
	// 0、外设信息结构体
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;		// 基本定时器功能信息结构体
	NVIC_InitTypeDef 		 NVIC_InitStructure;		// 嵌套向量中断控制器信息结构体
	
	// 1、使能定时器3的硬件时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

	// 2、使能定时器3的全局中断(配置定时器3的中断优先级)
	NVIC_InitStructure.NVIC_IRQChannel 					 = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority 		 = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd 				 = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	// 3、基本定时器配置
	TIM_TimeBaseStructure.TIM_Prescaler 	= 8400-1;				// 预分频值(一次分频)：168MHZ(AHB/1) /4(APB1/4) *2(APB的预分频值不是1，所以要*2) / 8400(为了更好的计算) == 输出频率：10000HZ(168000000/4*2/8400)
	TIM_TimeBaseStructure.TIM_Period 		= 5000-1;				// 周期(计数值)(中断间隔)(加载值)：	使用10000HZ的时钟，去计数5000的计数值，花费的时间为：0.5s(500ms)
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;					// 时钟分频(二次分频)：STM32F407芯片没有二次分频，写0即可
	TIM_TimeBaseStructure.TIM_CounterMode 	= TIM_CounterMode_Up;	// 计数模式：从下往上计数
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);					// 使用该函数，将配置的信息写入到相应的寄存器中

	// 4、使能定时器3中断(更新中断)
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);

	// 5、使能定时器计数
	TIM_Cmd(TIM3, ENABLE);

}

/**
  * @brief  初始化定时器14外设
  * @note   使用定时器14通道1，实现PWM功能输出给->D1(丝印层)->LED0(网络标号) ->PF9 (引脚标号)
  * @param  None
  * @retval None
  */
void TIMER14_Ch1_Init(void)
{
	GPIO_InitTypeDef 		GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef 		TIM_OCInitStructure;
    
    // 1. 使能时钟			                                            
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);    			// 使能GPIOF时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM14, ENABLE);    			// 使能TIM14时钟
				                                                        
    // 2. 配置PF9为复用功能(TIM14_CH1)			                        
    GPIO_InitStructure.GPIO_Pin 	= GPIO_Pin_9;			 			// 引脚：第8根引脚
    GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_AF;          			// 复用功能
    GPIO_InitStructure.GPIO_OType 	= GPIO_OType_PP;         			// 推挽输出
    GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_100MHz;     			// 100MHz
    GPIO_InitStructure.GPIO_PuPd 	= GPIO_PuPd_NOPULL;      			// 无上拉下拉
    GPIO_Init(GPIOF, &GPIO_InitStructure);			
    
    // 3. 将PF9连接到TIM14_CH1
    GPIO_PinAFConfig(GPIOF, GPIO_PinSource9, GPIO_AF_TIM14);
    
    // 4. 配置TIM14时基单元
    // 定时器时钟源为APB1，在STM32F4中，APB1时钟最大为84MHz
    // 我们配置为84MHz/8400 = 10000Hz的计数频率
    // 自动重装载值设为1000，这样PWM频率为10000Hz/1000 = 100Hz(发出100个1000周期)
	TIM_TimeBaseStructure.TIM_Prescaler		= 8400 - 1;          	 	// 预分频器       10000HZ
    TIM_TimeBaseStructure.TIM_Period    	= 100  - 1;         		// 自动重装载值  100计数值    ==》 100HZ
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;	                    
    TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up;  	 	// 向上计数模式
    TIM_TimeBaseInit(TIM14, &TIM_TimeBaseStructure);	                
		                                                                
    // 5. 配置TIM14通道1为PWM模式1	                                    
    TIM_OCInitStructure.TIM_OCMode 			= TIM_OCMode_PWM1;       	 // PWM模式1
    TIM_OCInitStructure.TIM_OutputState 	= TIM_OutputState_Enable;	 // 输出使能
    TIM_OCInitStructure.TIM_OCPolarity 		= TIM_OCPolarity_High;   	 // 输出极性高(占空比：以高电平为基准)
    TIM_OCInitStructure.TIM_Pulse           = 0;                     	 // 初始比较值
    TIM_OC1Init(TIM14, &TIM_OCInitStructure);	
    
    // 6. 使能TIM14预装载寄存器
    TIM_OC1PreloadConfig(TIM14, TIM_OCPreload_Enable);
    
    // 7. 使能TIM14的自动重装载预装载寄存器
    TIM_ARRPreloadConfig(TIM14, ENABLE);
    
    // 8. 使能TIM14
    TIM_Cmd(TIM14, ENABLE);
}

/**
  * @brief  初始化定时器13外设
  * @note   使用定时器13通道1，实现PWM功能输出给->LS1(丝印层)->BEEP(网络标号) ->PF8 (引脚标号)
  * @param  None
  * @retval None
  */
void TIMER13_Ch1_Init(void)
{
	GPIO_InitTypeDef 		GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef 		TIM_OCInitStructure;
    
    // 1. 使能时钟			                                            
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);    			// 使能GPIOF时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM13, ENABLE);    			// 使能TIM13时钟
				                                                        
    // 2. 配置PF8为复用功能(TIM13_CH1)			                        
    GPIO_InitStructure.GPIO_Pin 	= GPIO_Pin_8;			 			// 引脚：第8根引脚
    GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_AF;          			// 复用功能
    GPIO_InitStructure.GPIO_OType 	= GPIO_OType_PP;         			// 推挽输出
    GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_100MHz;     			// 100MHz
    GPIO_InitStructure.GPIO_PuPd 	= GPIO_PuPd_NOPULL;      			// 无上拉下拉
    GPIO_Init(GPIOF, &GPIO_InitStructure);			
    
    // 3. 将PF8连接到TIM13_CH1
    GPIO_PinAFConfig(GPIOF, GPIO_PinSource8, GPIO_AF_TIM13);
    
    // 4. 配置TIM13时基单元
    // 定时器时钟源为APB1，在STM32F4中，APB1时钟最大为84MHz
    // 我们配置为84MHz/1400 = 60000Hz的计数频率
    // 自动重装载值设为60000
	TIM_TimeBaseStructure.TIM_Prescaler		= 1400  - 1;          	 	// 预分频器       60000HZ
    TIM_TimeBaseStructure.TIM_Period    	= 60000 - 1;         		// 这里只是初始化一下，不用理会(要不要都无所谓，重点在后面)
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;	                    
    TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up;  	 	// 向上计数模式
    TIM_TimeBaseInit(TIM13, &TIM_TimeBaseStructure);	                
		                                                                
    // 5. 配置TIM13通道1为PWM模式1	                                    
    TIM_OCInitStructure.TIM_OCMode 			= TIM_OCMode_PWM1;       	 // PWM模式1
    TIM_OCInitStructure.TIM_OutputState 	= TIM_OutputState_Enable;	 // 输出使能
    TIM_OCInitStructure.TIM_OCPolarity 		= TIM_OCPolarity_High;   	 // 输出极性高(占空比：以高电平为基准)
    TIM_OCInitStructure.TIM_Pulse           = 0;                     	 // 初始比较值
    TIM_OC1Init(TIM13, &TIM_OCInitStructure);	
    
    // 6. 使能TIM13预装载寄存器
    TIM_OC1PreloadConfig(TIM13, TIM_OCPreload_Enable);
    
    // 7. 使能TIM14的自动重装载预装载寄存器
    TIM_ARRPreloadConfig(TIM13, ENABLE);
    
    // 8. 使能TIM13
    TIM_Cmd(TIM13, ENABLE);
}

/**
  * @brief  设置定时器13的频率(蜂鸣器的音调)
  * @note   None
  * @param  None
  * @retval None
  */
void TIMER13_SetFreq(uint16_t freq)
{
	// 基本定时器信息结构体
	 TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;	
	
	// 配置TIM13时基单元
    // 定时器时钟源为APB1，在STM32F4中，APB1时钟最大为84MHz
    // 我们配置为84MHz/1400 = 60000Hz的计数频率
    // 自动重装载值设为60000，这样PWM频率为60000Hz/(60000/freq) == freqHZ(1s内发出freq个周期为(60000/freq)的波形)
	TIM_TimeBaseStructure.TIM_Prescaler		= 1400  - 1;          	 // 预分频器60000Hz
    TIM_TimeBaseStructure.TIM_Period    	= 60000/freq - 1;        // 这里的freq变量决定了蜂鸣器的频率(音调)
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;	                    
    TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up;  	 // 向上计数模式
    TIM_TimeBaseInit(TIM13, &TIM_TimeBaseStructure);	
	
	// 记住现在的计数值
	tim13_cnt = TIM_TimeBaseStructure.TIM_Period;  

	
}

/**
  * @brief  设置定时器13在PWM功能中的占空比(%0到%100)(设置蜂鸣器的音量)
  * @note   None
  * @param  None
  * @retval None
  */
void TIMER13_SetDuty(uint16_t duty)		// 30
{
	uint32_t cmp = 0;
	cmp = (tim13_cnt+1)*duty/100;		// TIM_Period(假设：800*30/100 == 240 )
	TIM_SetCompare1(TIM13, cmp);		// 240/800 == %30
}



/**
  * @brief  定时器3的中断服务函数
  * @note   None
  * @param  None
  * @retval None
  */
void TIM3_IRQHandler(void)
{
	static int8_t flag = 0;
	
	if(TIM_GetITStatus(TIM3, TIM_IT_Update) == SET)		// 判断定时器3更新中断标志位是否被触发了
	{

		flag = !flag;
		LED1(flag);
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);		// 清空定时器的中断挂起标志位
	}
}





