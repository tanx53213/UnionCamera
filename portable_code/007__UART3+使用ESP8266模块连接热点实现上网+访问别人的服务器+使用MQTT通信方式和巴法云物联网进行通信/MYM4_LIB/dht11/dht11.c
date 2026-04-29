#include "dht11.h"
#include "delay.h"
#include <stdio.h>

/**
  * @brief  初始化DHT11温湿度模块
  * @note   GEC-M4板子上      dht11温湿度模块
  *         U5-》DQ-》PG9	  data     
  * @param  None  
  * @retval None
  */
void DHT11_Init(void)
{
	// 0、GPIO片内外设信息初始化结构体
	GPIO_InitTypeDef  GPIO_InitStructure;

	// 1、使能GPIO片内外设的硬件时钟					
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);

	// 2、配置GPIO片内外设的引脚						
	GPIO_InitStructure.GPIO_Pin		= GPIO_Pin_9;				// 引脚：第9根引脚
	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_OUT;			// 模式：输出模式
	GPIO_InitStructure.GPIO_Speed   = GPIO_High_Speed;			// 速度：高速(100MHz)
	GPIO_InitStructure.GPIO_PuPd	= GPIO_PuPd_NOPULL;			// 上下拉：不拉
	GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;			// 输出类型：推挽输出
	GPIO_Init(GPIOG, &GPIO_InitStructure);						// 使用该函数，将配置的信息写入到相应的寄存器中
	
	// 3、一开始将引脚设置为高电平
	DHT11_OUT = 1;
}



/**
  * @brief  选择DHT11温湿度模块引脚的输入输出模式
  * @note   None
  * @param  GPIO_Mode: GPIO_Mode_OUT(输出模式)， GPIO_Mode_IN(输入模式)
  * @retval 成功：返回0
  *         失败：返回非0
  */
int8_t DHT11_SelectMode(GPIOMode_TypeDef GPIO_Mode)
{
	// 0、GPIO片内外设信息初始化结构体
	GPIO_InitTypeDef  GPIO_InitStructure;

	// 1、选择模式并进行配置
	GPIO_InitStructure.GPIO_Pin		= GPIO_Pin_9;				// 引脚：第9根引脚
	
	if(GPIO_Mode == GPIO_Mode_OUT)
	{
		GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_OUT;		// 模式：输出模式
		GPIO_InitStructure.GPIO_Speed   = GPIO_High_Speed;		// 速度：高速(100MHz)
		GPIO_InitStructure.GPIO_PuPd	= GPIO_PuPd_NOPULL;		// 上下拉：不拉
		GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;		// 输出类型：推挽输出
	}
	else if(GPIO_Mode == GPIO_Mode_IN)
	{
		GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_IN;			// 模式：输入模式
		GPIO_InitStructure.GPIO_PuPd	= GPIO_PuPd_NOPULL;		// 上下拉：不拉
	}
	else
	{
		return -1;
	}
	GPIO_Init(GPIOG, &GPIO_InitStructure);						// 使用该函数，将配置的信息写入到相应的寄存器中
	
	return 0;
}


/**
  * @brief  获取dht11温湿度模块测量的数据
  * @note   None
  * @param  pbuf：存储40个温湿度数据位的缓冲区
  *				pbuf[0]：8位的湿度整数数据
  *				pbuf[1]：8位的湿度小数数据
  *				pbuf[2]：8位的温度整数数据
  *				pbuf[3]：8位的温度小数数据
  *				pbuf[4]：8位的校验和数据
  *
  * @retval 成功：返回0
  *         失败：返回非0(错误码：-1，-2，-3，-4，-5为超时错误， -6为校验错误)
  */	
int8_t DHT11_GetData(uint8_t pbuf[5])
{
	// 功能涉及到的相关变量
	uint16_t t_count   = 0;		// 超时计数变量
	int8_t i,j		   = 0;		// 循环的变量
	uint8_t data       = 0;		// 接收到的一个字节数据
	uint16_t check_num = 0;		// 校验数据
	
	// 一、单片机发送信号给DHT11模块激活它						// 方老板：阿坤啊，起来搬砖啦
	// 1、将PG9引脚设置为输出模式
	DHT11_SelectMode(GPIO_Mode_OUT);
	
	// 2、将PG9引脚设置为低电平
	DHT11_OUT = 0;
	
	// 3、至少延时18ms以上
	delay_ms(20);
	
	// 4、将PG9引脚设置为高电平									
	DHT11_OUT = 1;
	
	// 5、延时20-40us
	delay_us(30);
		
	// 二、DHT11模块发送响应信号给单片机						// 坤坤：收到，方老板，我马上搬砖
	// 1、将PG9引脚设置为输入模式
	DHT11_SelectMode(GPIO_Mode_IN);
	
	// 2、等待PG9引脚从高电平变为低电平(看其从高电平变为低电平是否超时4ms)
	t_count = 0;
	while( DHT11_IN == 1)
	{
		t_count++;			// 通过计数，知道其是否超时(4ms)
		delay_us(1);
		if(t_count > 4500)
		{
			return -1;
		}
	}
	
	// 3、等待PG9引脚从低电平变为高电平(持续时间80us，建议100us)
	t_count = 0;
	while( DHT11_IN == 0)
	{
		t_count++;			
		delay_us(1);
		if(t_count > 100)
		{
			return -2;
		}
	}
	
	// 4、等待PG9引脚从高电平变为低电平(持续时间80us，建议100us)
	t_count = 0;
	while( DHT11_IN == 1)
	{
		t_count++;			
		delay_us(1);
		if(t_count > 100)
		{
			return -3;
		}
	}
	
	// 三、DHT11模块发送40个bit数据给单片机，单片机接收并验证		// 坤坤：努力地将轻重不一的砖搬给方老板，方老板进行结算(轻砖：10元， 重砖：20元)
	// 1、将40个bit数据一次存到5个字节的数组中
	for(i=0; i<5; i++)		// 5个字节数据
	{
		data = 0;			// 将数据一开始赋值为0，后面移位的时候，就只需要移位1了
		for(j=7; j>=0; j--)
		{
			// a、等待发送数据的高电平的到来(持续时间50us，建议60us)
			t_count = 0;
			while( DHT11_IN == 0)
			{
				t_count++;			
				delay_us(1);
				if(t_count > 60)
				{
					return -4;
				}
			}
			
			// b、开始延时一段时间(40us)
			delay_us(40);
			
			// c、判断引脚是否为高电平，是的话进行移位
			if(DHT11_IN == 1)
			{
				// 移位操作 
				data = data | (1<<j);
				
				// 将剩余的高电平的时间将其略过(持续时间30us，建议40us)
				t_count = 0;
				while( DHT11_IN == 1)
				{
					t_count++;			
					delay_us(1);
					if(t_count > 40)
					{
						return -5;
					}
				}
			}
		}
		// 将移位好的数据放置到pbuf数组里面去
		pbuf[i] = data;
		
	}
	
	// 2、进行数据校验
	check_num = pbuf[0]+pbuf[1]+pbuf[2]+pbuf[3];	// 湿整、湿小、温整、温小
	if( (check_num & 0xff)  != pbuf[4] )
			return -6;

	// 四、通信结束												// 坤坤没有事干了，暂且休息
	delay_us(100);

	return 0;
}



































