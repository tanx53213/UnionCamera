#include "systick_it.h"
#include "buzzer.h"

/**
  * @brief  systick中断服务函数
  * @note   None
  * @param  None 
  * @retval None
  */
void SysTick_Handler(void)		
{
	static int16_t systick_count = 0;
	static int8_t  flag          = 0;
	
	systick_count++;
	if(systick_count == 500)
	{
		flag = !flag;	
		BUZZER(flag);
		
		systick_count = 0;	
	}
	
}

