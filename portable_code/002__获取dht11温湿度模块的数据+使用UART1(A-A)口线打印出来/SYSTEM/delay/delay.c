#include "delay.h"


static uint8_t  sysclk_nus      = 0;
static uint32_t sysclk_nms      = 0;
static uint8_t  sysclk_clk_flag = 0;


/**
  * @brief  自写延时
  * @note   利用执行语句时产生的时间，进行多个语句的调用，即可实现粗延时
  * @param  None
  * @retval None
  */
void DELAY_MyNms(uint16_t nms)
{
	uint32_t i = 0;
	while(nms--)
	{
		i = 12000;
		while(i--);
	}
}



/**
  * @brief  系统定时器初始化
  * @note   选择时钟源(168MHZ(AHB或HCLK)、21MHZ(8分频的AHB或HCLK))
  * @param  sysclk：此处填写168或21,选择时钟源，取决于你的需要
  * @retval 成功：返回0
  *         失败：返回-1
  */
int8_t DELAY_SysTickInit(uint8_t sysclk)
{
	if(sysclk == 168)
	{
		sysclk_nus      = 168;	
		sysclk_nms      = 168000;
		sysclk_clk_flag = 5;			// 选择HCLK时钟(内核时钟)：168MHZ
		// SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);
	}
	else if(sysclk == 21)
	{
		sysclk_nus      = 21;	
		sysclk_nms      = 21000;
		sysclk_clk_flag = 1;			// 选择8分频HCLK时钟(外部参考时钟)：21MHZ
		// SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);
	}
	else
	{
		return -1;
	}
	
	return 0;
}

/**
  * @brief  精准延时nus
  * @note   None
  * @param  nus：微秒级延时
  * @retval 成功：返回0
  *         失败：返回非0
  */
int8_t delay_us(uint32_t nus)
{
	uint32_t t    = nus;
	uint32_t temp = 0;
	
	while(t--)
	{
		SysTick->CTRL = 0;
		SysTick->LOAD = sysclk_nus-1;   
		SysTick->VAL  = 0;
		SysTick->CTRL = sysclk_clk_flag; 
		
		while(1)
		{
			temp = SysTick->CTRL;
			
			// 检测COUNTFLAG标志位
			if(temp & 0x00010000)
				break;
			
			// 检查系统定时器是否意外关闭
			if( (temp & 0x1) == 0)
				return -1;	
		}
		
		SysTick->CTRL = 0;
	}
	
	return 0;	
	
}

/**
  * @brief  精准延时nms
  * @note   None
  * @param  nus：毫秒级延时
  * @retval 成功：返回0
  *         失败：返回非0
  */
int8_t delay_ms(uint32_t nms)
{
	uint32_t t    = nms;
	uint32_t temp = 0;
	
	while(t--)
	{
		SysTick->CTRL = 0;
		SysTick->LOAD = sysclk_nms-1;
		SysTick->VAL  = 0;
		SysTick->CTRL = sysclk_clk_flag; 
		
		while(1)
		{
			temp = SysTick->CTRL;
			
			// 检测COUNTFLAG标志位
			if(temp & 0x00010000)
				break;
			
			// 检查系统定时器是否意外关闭
			if( (temp & 0x1) == 0)
				return -1;	
		}
		
		SysTick->CTRL = 0;
	}
	
	return 0;	
	
}
