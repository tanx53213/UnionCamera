#include "hc_sr04.h"
#include "delay.h"

/**
  * @brief  初始化HC_SR04超声波模块
  * @note   GEC-M4板子
  *			CMAERA(13引脚，丝印层)-> DCMI_D5(网络标号) ->PB6(芯片引脚标号)  ===连接=== 超声波模块(TRIG引脚)
  * 		CMAERA(15引脚，丝印层)-> DCMI_D7(网络标号) ->PE6(芯片引脚标号)  ===连接=== 超声波模块(ECHO引脚)
  * @param  None
  * @retval None
  */
void HC_SR04_Init(void)
{
	// 0、GPIO片内外设信息初始化结构体
	GPIO_InitTypeDef GPIO_InitStructure;
	
	// 1、使能GPIO片内外设的硬件时钟				
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
	
	// 2、配置GPIO片内外设的引脚
	// a、TRIG引脚(PB6)，输出
	GPIO_InitStructure.GPIO_Pin		= GPIO_Pin_6;		// 引脚：第6根引脚
	GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_OUT;	// 模式：输出模式
	GPIO_InitStructure.GPIO_Speed   = GPIO_High_Speed;	// 速度：高速(100MHz)
	GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_NOPULL;	// 上下拉：不拉(片外外设引脚有了)
	GPIO_InitStructure.GPIO_OType	= GPIO_OType_PP;	// 输出类型：推挽输出(增强引脚输出能力)
	GPIO_Init(GPIOB, &GPIO_InitStructure);				// 使用该函数，将配置的信息写入到相应的寄存器中
	
	// b、ECHO引脚(PE6)，输入
	GPIO_InitStructure.GPIO_Pin		= GPIO_Pin_6;		// 引脚：第6根引脚
	GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_IN;		// 模式：输入模式
	GPIO_Init(GPIOE, &GPIO_InitStructure);				// 使用该函数，将配置的信息写入到相应的寄存器中
	

	// 3、设置GPIO片内外设引脚的高低电平值			
	TRIG = 0;
	// ECHO = 0;										// 该引脚已经被设置成了输入模块，你设置不了电平的
	
}

/**
  * @brief  获取hc_sr04超声波模块的距离
  * @note   获取的距离单位是毫米级
  * @param  error_coefficient：超声波模块的误差系数
  * @retval 成功：
  *				返回超声波获取的距离(浮点型数据)
  *				如果使用的是声音在空气中的速度，返回为毫米级******
  *				如果使用的是公式，返回厘米级
  *			失败：
  8				返回非0
  */
double HC_SR04_GetDis(float error_coefficient)
{
	int32_t count = 0;
	double dis_val = 0;
	
	
	// 一、TRIG引脚输出至少10us以上的高电平					// STM32单片机老板：小超啊，赶紧起来干活啦，不要睡觉了	
	// 1、设置TRIG引脚为高电平
	TRIG = 1;
	
	// 2、让其高电平持续时间在10us以上
	delay_us(12);
	
	// 3、设置TRIG引脚为低电平
	TRIG = 0;
	
	// 二、检查ECH0引脚的状态，从而得知超声波现在的情况		// 超声波模块：STM32老板，我现在在干活啦，你看一下我的状态就知道了
	// 1、等待ECH引脚从低电平变为高电平(模块开始发出超声波信号)
	count = 0;
	while(ECHO == 0)
	{
		count++;				// 通过计数，知道其是否超时(60ms)
		delay_us(1);
		if(count > 65000)
			return -1;
	}
	// 2、等待ECHO引脚从高电平变为低电平(模块接收到返回的超声波i)，并计数ECHO引脚在高电平的持续时间(超声波来回障碍物的时间)
	count = 0;
	while(ECHO == 1)
	{
		count++;				// 计数有多少个8.8us，就是计数有多少个3mm，并测试其是否超时了	
		delay_us(8);			
		if(count > (65000/8))
			return -2;
	}
	
	// 3、通过超声波的来回障碍物的时间测量其距离
	dis_val = count*error_coefficient/2.0;	// 利用在空气中声音的速度(单位：毫米级)

	return dis_val;
	
}




