#include "rs485.h"
#include "delay.h"
#include "led.h"

volatile uint8_t u2_rs485_recvbuf[128];
volatile uint8_t u2_rs485_count;
volatile uint8_t u2_rs485_flag;


/**
  * @brief  RS485初始化函数：配置USART2(PA2=TX，PA3=RX)， 及方向控制引脚PG8
  * @note   None
  * @param  baudrate：波特率(9600(1200米)、115200等)
  * @retval None
  */
void RS485_Init(uint32_t baudrate)
{
	// 0、片内外设信息初始化结构体
	GPIO_InitTypeDef  GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef  NVIC_InitStructure;
	
	// 使能GPIOA外设时钟（AHB1总线）和USART2外设时钟（APB1总线，最高频率42MHz）
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE); 				
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);				
	
	// 串口2引脚复用映射：PA2->USART2_TX，PA3->USART2_RX
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource2,GPIO_AF_USART2); 			
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource3,GPIO_AF_USART2); 			
	
	// USART2端口配置（PA2、PA3）
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3; 				// 选择引脚2（TX）和3（RX）
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;						// 模式：复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;					// 输出速度：100MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 						// 输出类型：推挽复用输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; 						// 上拉电阻（空闲状态为高电平）
	GPIO_Init(GPIOA,&GPIO_InitStructure); 								// 将配置写入GPIOA寄存器
	
	
	// 使能GPIOG外设时钟（AHB1总线），用于RS485方向控制引脚PG8
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG,ENABLE); 				
	
	// PG8引脚初始化（RS485方向控制）
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8; 							// 选择引脚8
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;						// 模式：普通输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;					// 输出速度：100MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 						// 输出类型：推挽输出
	GPIO_InitStructure.GPIO_PuPd  =  GPIO_PuPd_UP; 						// 上拉电阻（默认高电平，但初始化后设为接收模式）
	GPIO_Init(GPIOG,&GPIO_InitStructure); 								// 将配置写入GPIOG寄存器
	

	// USART2初始化设置
	USART_InitStructure.USART_BaudRate = baudrate;							// 波特率：用户指定
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;			// 数据位：8位
	USART_InitStructure.USART_StopBits = USART_StopBits_1;				// 停止位：1位
	USART_InitStructure.USART_Parity = USART_Parity_No;					// 校验位：无
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;// 硬件流控：无
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;		// 模式：收发双向
	USART_Init(USART2, &USART_InitStructure); 							// 将配置写入USART2寄存器
	
	USART_Cmd(USART2, ENABLE);  										// 使能USART2外设
	

	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);						// 开启USART2接收中断（RXNE：接收数据寄存器非空）

	// USART2中断优先级配置（NVIC）
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn; 					// 中断通道：USART2_IRQn
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;				// 抢占优先级：3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;					// 子优先级：3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;						// 使能中断通道
	NVIC_Init(&NVIC_InitStructure);										// 将配置写入NVIC寄存器
	RS485_TX_EN=0;														// 默认设置为接收模式（PG8=0）	

}



/**
  * @brief  RS485发送数据函数(含方向控制)
  * @note   下位机(单片机)发送数据信息给上位机(电脑的串口助手、手机蓝牙的APP、手机WiFi的APP), 发送的是字符串数据
  * @param  pbuf：待发送数据的指针
  *         len： 待发送数据的长度(字节数)
  * @retval None
  */
void RS485_SendStr(uint8_t* pbuf, uint32_t len)			
{
	uint32_t i;  			// 循环计数器
	uint8_t *p = pbuf;  	// 数据指针副本
		
	RS485_TX_EN = 1;  		// 设置为发送模式（PG8=1）
	delay_ms(1);  			// 延时1ms，确保方向切换稳定
	
	for(i=0; i<len; i++)  	// 遍历发送所有字节
	{
		USART_SendData(USART2,p[i]);  								// 通过USART2发送当前字节
		
		while(USART_GetFlagStatus(USART2,USART_FLAG_TXE)==RESET);  // 等待发送完成（TXE：发送数据寄存器为空）
	}	 
	
	delay_ms(1); 			// 延时1ms，确保最后一字节发送完成
	RS485_TX_EN=0;  		// 恢复为接收模式（PG8=0）	
}



/**
  * @brief  UART2的接收数据中断服务函数(上位机->下位机)
  * @note   下位机(单片机)接收来自上位机(电脑的串口助手、手机蓝牙的APP、手机WiFi的APP)的信息，并进行处理
  * @param  None 
  * @retval None
  */
void USART2_IRQHandler(void)	// 串口助手通过rs485发送数据给单片机
{
	uint8_t r_data = 0;
	if( USART_GetITStatus(USART2, USART_IT_RXNE) == SET)		// 判断是否有数据发送到了uart2中了(串口接收中断)
	{
		// 接收来自上位机的数据并存放，设置接收完毕标志位
		r_data                           = USART_ReceiveData(USART2);

		if(r_data == 'a')
			LED1(ON);
		if(r_data == 'b')
			LED1(OFF);	

		USART_ClearITPendingBit(USART2, USART_IT_RXNE);			// 清除串口接收中断标志位
	}		
}
