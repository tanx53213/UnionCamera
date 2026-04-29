#include "hs0038.h"
#include "delay.h"


/**
  * @brief  初始化hs0038红外接收头模块
  * @note   GEC-M4板子         
  *			U6(丝印层)-> REMOTE_IN(网络标号) ->PA8(芯片引脚标号)
  * @param  None
  * @retval None
  */
void HS0038_Init(void)
{
	// 0、GPIO片内外设信息初始化结构体
	GPIO_InitTypeDef GPIO_InitStructure;
	
	// 1、使能GPIO片内外设的硬件时钟				
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	
	// 2、配置GPIO片内外设的引脚
	GPIO_InitStructure.GPIO_Pin		= GPIO_Pin_8;		// 引脚：第8根引脚
	GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_IN;		// 模式：输入模式
	GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_NOPULL;	// 上下拉：上拉或者不拉
	GPIO_Init(GPIOA, &GPIO_InitStructure);				// 使用该函数，将配置的信息写入到相应的寄存器中		
}

/**
  * @brief  获取hs0038红外接收头模块的数据
  * @note   None
  * @param  pbuf：存储32个红外数据位的缓冲区buf
				pbuf[0]：8位的地址码数据
				pbuf[1]：8位的地址反码数据
				pbuf[2]：8位的命令码数据
				pbuf[3]：8位的命令反码数据
  * @retval 成功：返回0
*			失败：返回非0(错误码：-1到-5为超时错误，-6为数据校验错误)
  */
int8_t HS0038_GetData(uint8_t pbuf[4])
{
	uint16_t t_count = 0;
	int8_t   i       = 0;
	int8_t   j       = 0;
	uint8_t data     = 0;
	
	// 一、解析起始位(引导码)
	// 1、等待一开始的高电平引脚过去(作用：防止调用这个函数的时候，在没有获取红外遥控的信号的时候，就直接运行了)
	while(HS0038 == 1);			// 获取到红外遥控信号的时候，红外接收头引脚将从高电平变为低电平
	
	// 2、等待9ms的红外脉冲过去(建议用微妙，更精确)(持续时间9000us，建议9500us)
	t_count = 0;
	while(HS0038 == 0)
	{
		t_count++;
		delay_us(1);
		if(t_count > 9500)
			return -1;
	}
	
	// 3、等待4.5ms的非红外脉冲过去(建议用微妙，更精确)(持续时间4500us，建议5000us)
	t_count = 0;
	while(HS0038 == 1)
	{
		t_count++;
		delay_us(1);
		if(t_count > 5000)
			return -2;
	}
	
	// 二、获取有效数据并赋值到数组pbuf中
	// 1、将红外数据存储到数组pbuf里面
	for(i=0; i<4; i++)			// 一共有4个字节的数据
	{
		data = 0;				// 将数据一开始赋值为0， 后面移位的时候，就只需要移位1了
		for(j=0; j<8; j++)		// LSB	
		{
			// a、等待560us的红外脉冲数据过去(持续时间560us，建议600us)
			t_count = 0;
			while(HS0038 == 0)
			{
				t_count++;
				delay_us(1);
				if(t_count > 600)
					return -3;
			}
			
			// b、延时一段时间(建议800us，以区分数据0还是数据1)
			delay_us(800);
			
			// c、判断引脚是否为高电平，是的话进行移位
			if(HS0038 == 1)
			{
				// 移位操作
				data = data | (1<<j);
				
				// 将剩余的高电平的时间略过
				t_count = 0;
				while(HS0038 == 1)
				{
					t_count++;
					delay_us(1);
					if(t_count > 1000)
						return -4;
				}
			}	
		}
		
		pbuf[i] = data;			// 将移位好的数据，赋值给数组pbuf
	}
	
	// 2、校验数据
	if( ((pbuf[0]&pbuf[1])!=0) ||  ((pbuf[2]&pbuf[3])!=0) )
		return -5;
	
	// 三、结束获取数据(延时600us度过红外脉冲信号)
	delay_us(600);
	return 0;
}




