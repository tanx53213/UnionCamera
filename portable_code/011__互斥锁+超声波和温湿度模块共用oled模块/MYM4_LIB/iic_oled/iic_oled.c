#include "iic_oled.h"
#include "iic_oled_bmp.h"
#include "iic_oled_font.h" 
#include "stdlib.h"
#include "delay.h"


// =======================================================一、IIC协议======================================================= // 
/**
  * @brief  起始信号
  * @note   通知从机做好通信的准备
  * @param  None
  * @retval None
  */
void IIC_OLED_Start(void)
{
	// 1、设置SDA引脚为输出模式
	IIC_OLED_SelectMode(GPIO_Mode_OUT);
	
	// 2、设置SCL为高电平，SDA高电平、并延时4.7us以上
	IIC_OLED_SCL     = 1;
	IIC_OLED_SDA_OUT = 1;
	delay_us(5);
	
	// 3、设置SDA为低电平，并延时4us以上
	IIC_OLED_SDA_OUT = 0;
	delay_us(5);
	
	// 4、设置SCL为低电平
	IIC_OLED_SCL     = 0;
}


/**
  * @brief  停止信号
  * @note   告诉从机通信已结束
  * @param  None
  * @retval None
  */
void IIC_OLED_Stop(void)
{
	// 1、设置SDA引脚为输出模式
	IIC_OLED_SelectMode(GPIO_Mode_OUT);
	
	// 2、设置SCL为高电平、SDA为低电平、并延时4us以上
	IIC_OLED_SCL     = 1;
	IIC_OLED_SDA_OUT = 0;
	delay_us(5);
	
	// 3、设置SDA为高电平，并延时4.7us以上
	IIC_OLED_SDA_OUT = 1;
	delay_us(5);
	
	// 4、设置SDA为低电平
	IIC_OLED_SDA_OUT = 0;
	
}

/**
  * @brief  发送一个字节(8位)数据
  * @note   主机->从机
  * @param  byte：要发送的一个字节(8位)数据
  * @retval None
  */
void IIC_OLED_SendByte(uint8_t byte)	// 1010 1100
{
	int8_t i = 0;
	
	// 1、设置SDA引脚为输出模式
	IIC_OLED_SelectMode(GPIO_Mode_OUT);
	
	// 2、设置SCL为低电平，SDA为低电平，并延时4us以上
	IIC_OLED_SCL     = 0;
	IIC_OLED_SDA_OUT = 0;
	delay_us(5);
	
	// 3、开始发送8位数据(MSB)
	for(i=7; i>=0; i--)					// 1010 1100&1000 0000 == 1000 0000(非0为真)
	{
		// a、设置SCL为低电平，并发送数据
		if(byte&(1<<i))
			IIC_OLED_SDA_OUT = 1;
		else
			IIC_OLED_SDA_OUT = 0;
		
		delay_us(2);					// 让其有时间将数据发送出去

		// b、设置SCL为高电平，并延时4us以上
		IIC_OLED_SCL     = 1;
		delay_us(5);					
		
		// c、设置SCL为低电平，并延时4us以上
		IIC_OLED_SCL     = 0;
		delay_us(5);
	}
	
}



/**
  * @brief  接收应答信号
  * @note   主机<-从机
  * @param  None
* @retval 返回应答信号(应答：0， 不应答1)
  */
uint8_t IIC_OLED_RecvAck(void)	
{
	uint8_t ack = 0;
	
	// 1、设置SDA引脚为输入模式
	IIC_OLED_SelectMode(GPIO_Mode_IN);
	
	// 2、设置SCL为高电平，并延时4us以上
	IIC_OLED_SCL     = 1;
	delay_us(5);
	
	// 3、判断SDA引脚的电平值
	if(IIC_OLED_SDA_IN == 1)
		ack = 1;
	else
		ack = 0;
	
	delay_us(5);			// 让其有时间读取引脚的电平值
	
	// 4、设置SCL为低电平
	IIC_OLED_SCL     = 0;
	delay_us(2);
	
	// 5、返回ack值
	return ack;
}


// =======================================================二、OLED模块通信======================================================= // 

/**
  * @brief  初始化oled模块(ssd1306芯片)
  * @note   GEC-M4板子
  *				TFTLCD(丝印层)->LCD_BL(网络标号)   ->PB15(芯片引脚标号) -> SCL引脚
  *				TFTLCD(丝印层)->FSMC_D15(网络标号) ->PD10(芯片引脚标号) -> SDA引脚
  *
  * @param  None
  * @retval None
  */				    
void IIC_OLED_Init(void)
{ 	
 	 
 	// 0、GPIO片内外设信息初始化结构体
	GPIO_InitTypeDef GPIO_InitStructure;
	
	// 1、使能GPIO片内外设的硬件时钟				
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	
	// 2、配置GPIO片内外设的引脚	
	// PB15(SCL)
	GPIO_InitStructure.GPIO_Pin		= GPIO_Pin_15;				// 引脚：第15根引脚
	GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_OUT;			// 模式：输出模式
	GPIO_InitStructure.GPIO_Speed   = GPIO_High_Speed;			// 速度：高速(100MHz)
	GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_UP;				// 上下拉：上拉或不拉
	GPIO_InitStructure.GPIO_OType	= GPIO_OType_OD;			// 输出类型：开漏输出
	GPIO_Init(GPIOB, &GPIO_InitStructure);						// 使用该函数，将配置的信息写入到相应的寄存器中
	
	// PD10(SCL)
	GPIO_InitStructure.GPIO_Pin		= GPIO_Pin_10;				// 引脚：第10根引脚、
	GPIO_Init(GPIOD, &GPIO_InitStructure);						// 使用该函数，将配置的信息写入到相应的寄存器中
	
	delay_ms(200);

	OLED_WR_Byte(0xAE,OLED_CMD);//--display off
	OLED_WR_Byte(0x00,OLED_CMD);//---set low column address
	OLED_WR_Byte(0x10,OLED_CMD);//---set high column address
	OLED_WR_Byte(0x40,OLED_CMD);//--set start line address  
	OLED_WR_Byte(0xB0,OLED_CMD);//--set page address
	OLED_WR_Byte(0x81,OLED_CMD); // contract control
	OLED_WR_Byte(0xFF,OLED_CMD);//--128   
	OLED_WR_Byte(0xA1,OLED_CMD);//set segment remap 
	OLED_WR_Byte(0xA6,OLED_CMD);//--normal / reverse
	OLED_WR_Byte(0xA8,OLED_CMD);//--set multiplex ratio(1 to 64)
	OLED_WR_Byte(0x3F,OLED_CMD);//--1/32 duty
	OLED_WR_Byte(0xC8,OLED_CMD);//Com scan direction
	OLED_WR_Byte(0xD3,OLED_CMD);//-set display offset
	OLED_WR_Byte(0x00,OLED_CMD);//
	
	OLED_WR_Byte(0xD5,OLED_CMD);//set osc division
	OLED_WR_Byte(0x80,OLED_CMD);//
	
	OLED_WR_Byte(0xD8,OLED_CMD);//set area color mode off
	OLED_WR_Byte(0x05,OLED_CMD);//
	
	OLED_WR_Byte(0xD9,OLED_CMD);//Set Pre-Charge Period
	OLED_WR_Byte(0xF1,OLED_CMD);//
	
	OLED_WR_Byte(0xDA,OLED_CMD);//set com pin configuartion
	OLED_WR_Byte(0x12,OLED_CMD);//
	
	OLED_WR_Byte(0xDB,OLED_CMD);//set Vcomh
	OLED_WR_Byte(0x30,OLED_CMD);//
	
	OLED_WR_Byte(0x8D,OLED_CMD);//set charge pump enable
	OLED_WR_Byte(0x14,OLED_CMD);//
	
	OLED_WR_Byte(0xAF,OLED_CMD);//--turn on oled panel
}  

/**
  * @brief  选择oled模块引脚的输入输出模式
  * @note   None
  * @param  GPIO_Mode：GPIO_Mode_OUT(输出)、GPIO_Mode_IN(输入)
  * @retval None
  */
int8_t IIC_OLED_SelectMode(GPIOMode_TypeDef GPIO_Mode)
{
	// 0、GPIO片内外设信息初始化结构体
	GPIO_InitTypeDef GPIO_InitStructure;
	
	// 1、选择模式并进行配置
	GPIO_InitStructure.GPIO_Pin		= GPIO_Pin_10;		// 引脚：第10根引脚
	
	if(GPIO_Mode == GPIO_Mode_OUT)
	{
		GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_OUT;	// 模式：输出模式
		GPIO_InitStructure.GPIO_Speed   = GPIO_High_Speed;	// 速度：高速(100MHz)
		GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_UP;		// 上下拉：上拉或不拉(因为硬件有上拉)
		GPIO_InitStructure.GPIO_OType	= GPIO_OType_OD;	// 输出类型：开漏输出
		
	}
	else if(GPIO_Mode == GPIO_Mode_IN)
	{
		GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_IN;		// 模式：输入模式
		GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_UP;		// 上下拉：上拉或不拉(因为硬件有上拉)
	}
	else
	{
		return -1;
	}
	GPIO_Init(GPIOD, &GPIO_InitStructure);					// 使用该函数，将配置的信息写入到相应的寄存器中
	
	return 0;
	
}


/**********************************************
// IIC Write Command
**********************************************/
void Write_IIC_Command(unsigned char IIC_Command)
{
	IIC_OLED_Start();
	
	IIC_OLED_SendByte(0x78);            
	IIC_OLED_RecvAck();	
	
	IIC_OLED_SendByte(0x00);			
	IIC_OLED_RecvAck();	
	
	IIC_OLED_SendByte(IIC_Command); 
	IIC_OLED_RecvAck();	
	
	IIC_OLED_Stop();
}

/**********************************************
// IIC Write Data
**********************************************/
void Write_IIC_Data(unsigned char IIC_Data)
{
	IIC_OLED_Start();
	
	IIC_OLED_SendByte(0x78);			//D/C#=0; R/W#=0
	IIC_OLED_RecvAck();	
	
	IIC_OLED_SendByte(0x40);			//write data
	IIC_OLED_RecvAck();	
	
	IIC_OLED_SendByte(IIC_Data);
	IIC_OLED_RecvAck();	
	
	IIC_OLED_Stop();
}

void OLED_WR_Byte(unsigned dat,unsigned cmd)
{
	if(cmd)
	{
		Write_IIC_Data(dat);
	}
	else 
	{
		Write_IIC_Command(dat);	
	}
}


/********************************************
// fill_Picture
********************************************/
void fill_picture(unsigned char fill_Data)
{
	unsigned char m,n;
	for(m=0;m<8;m++)
	{
		OLED_WR_Byte(0xb0+m,0);		//page0-page1
		OLED_WR_Byte(0x00,0);		//low column start address
		OLED_WR_Byte(0x10,0);		//high column start address
		for(n=0;n<128;n++)
			{
				OLED_WR_Byte(fill_Data,1);
			}
	}
}


/***********************Delay****************************************/
void Delay_50ms(unsigned int Del_50ms)
{
	unsigned int m;
	for(;Del_50ms>0;Del_50ms--)
		for(m=6245;m>0;m--);
}

void Delay_1ms(unsigned int Del_1ms)
{
	unsigned char j;
	while(Del_1ms--)
	{	
		for(j=0;j<123;j++);
	}
}

//坐标设置

	void OLED_Set_Pos(unsigned char x, unsigned char y) 
{ 	OLED_WR_Byte(0xb0+y,OLED_CMD);
	OLED_WR_Byte(((x&0xf0)>>4)|0x10,OLED_CMD);
	OLED_WR_Byte((x&0x0f),OLED_CMD); 
}   	  
//开启OLED显示    
void OLED_Display_On(void)
{
	OLED_WR_Byte(0X8D,OLED_CMD);  //SET DCDC命令
	OLED_WR_Byte(0X14,OLED_CMD);  //DCDC ON
	OLED_WR_Byte(0XAF,OLED_CMD);  //DISPLAY ON
}
//关闭OLED显示     
void OLED_Display_Off(void)
{
	OLED_WR_Byte(0X8D,OLED_CMD);  //SET DCDC命令
	OLED_WR_Byte(0X10,OLED_CMD);  //DCDC OFF
	OLED_WR_Byte(0XAE,OLED_CMD);  //DISPLAY OFF
}		   			 
//清屏函数,清完屏,整个屏幕是黑色的!和没点亮一样!!!	  
void OLED_Clear(void)  
{  
	u8 i,n;		    
	for(i=0;i<8;i++)  
	{  
		OLED_WR_Byte (0xb0+i,OLED_CMD);    //设置页地址（0~7）
		OLED_WR_Byte (0x00,OLED_CMD);      //设置显示位置—列低地址
		OLED_WR_Byte (0x10,OLED_CMD);      //设置显示位置—列高地址   
		for(n=0;n<128;n++)OLED_WR_Byte(0,OLED_DATA); 
	} //更新显示
}
void OLED_On(void)  
{  
	u8 i,n;		    
	for(i=0;i<8;i++)  
	{  
		OLED_WR_Byte (0xb0+i,OLED_CMD);    //设置页地址（0~7）
		OLED_WR_Byte (0x00,OLED_CMD);      //设置显示位置—列低地址
		OLED_WR_Byte (0x10,OLED_CMD);      //设置显示位置—列高地址   
		for(n=0;n<128;n++)OLED_WR_Byte(1,OLED_DATA); 
	} //更新显示
}
//在指定位置显示一个字符,包括部分字符
//x:0~127
//y:0~63
//mode:0,反白显示;1,正常显示				 
//size:选择字体 16/12 
void OLED_ShowChar(u8 x,u8 y,u8 chr,u8 Char_Size)
{      	
	unsigned char c=0,i=0;	
		c=chr-' ';//得到偏移后的值			
		if(x>Max_Column-1){x=0;y=y+2;}
		if(Char_Size ==16)
			{
			OLED_Set_Pos(x,y);	
			for(i=0;i<8;i++)
			OLED_WR_Byte(F8X16[c*16+i],OLED_DATA);
			OLED_Set_Pos(x,y+1);
			for(i=0;i<8;i++)
			OLED_WR_Byte(F8X16[c*16+i+8],OLED_DATA);
			}
			else {	
				OLED_Set_Pos(x,y);
				for(i=0;i<6;i++)
				OLED_WR_Byte(F6x8[c][i],OLED_DATA);
				
			}
}
//m^n函数
u32 oled_pow(u8 m,u8 n)
{
	u32 result=1;	 
	while(n--)result*=m;    
	return result;
}				  
//显示2个数字
//x,y :起点坐标	 
//len :数字的位数
//size:字体大小
//mode:模式	0,填充模式;1,叠加模式
//num:数值(0~4294967295);	 		  
void OLED_ShowNum(u8 x,u8 y,u32 num,u8 len,u8 size2)
{         	
	u8 t,temp;
	u8 enshow=0;						   
	for(t=0;t<len;t++)
	{
		temp=(num/oled_pow(10,len-t-1))%10;
		if(enshow==0&&t<(len-1))
		{
			if(temp==0)
			{
				OLED_ShowChar(x+(size2/2)*t,y,' ',size2);
				continue;
			}else enshow=1; 
		 	 
		}
	 	OLED_ShowChar(x+(size2/2)*t,y,temp+'0',size2); 
	}
} 
//显示一个字符号串
void OLED_ShowString(u8 x,u8 y,u8 *chr,u8 Char_Size)
{
	unsigned char j=0;
	while (chr[j]!='\0')
	{		OLED_ShowChar(x,y,chr[j],Char_Size);
			x+=8;
		if(x>120){x=0;y+=2;}
			j++;
	}
}
//显示汉字
void OLED_ShowCHinese(u8 x,u8 y,u8 no)
{      			    
	u8 t,adder=0;
	OLED_Set_Pos(x,y);	
    for(t=0;t<16;t++)
		{
				OLED_WR_Byte(Hzk[2*no][t],OLED_DATA);
				adder+=1;
     }	
		OLED_Set_Pos(x,y+1);	
    for(t=0;t<16;t++)
			{	
				OLED_WR_Byte(Hzk[2*no+1][t],OLED_DATA);
				adder+=1;
      }					
}
/***********功能描述：显示显示BMP图片128×64起始点坐标(x,y),x的范围0～127，y为页的范围0～7*****************/
void OLED_DrawBMP(unsigned char x0, unsigned char y0,unsigned char x1, unsigned char y1,unsigned char BMP[])
{ 	
 unsigned int j=0;
 unsigned char x,y;
  
  if(y1%8==0) y=y1/8;      
  else y=y1/8+1;
	for(y=y0;y<y1;y++)
	{
		OLED_Set_Pos(x0,y);
    for(x=x0;x<x1;x++)
	    {      
	    	OLED_WR_Byte(BMP[j++],OLED_DATA);	    	
	    }
	}
} 































