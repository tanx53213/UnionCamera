/**
  ******************************************************************************
  * @file    iic_oled.h
  * @author  FZetc飞贼
  * @version V0.0.1
  * @date    2026.04.01
  * @brief   说明：0.69寸OLED 接口演示例程(STM32F407ZE系列IIC)
  *			 引脚：
  *             GND   电源地
  *             VCC   接5V或3.3v电源
  *             SCL   接PD6（SCL）
  *             SDA   接PD7（SDA） 
  *
  ******************************************************************************
  * @attention
  *
  * 本文档只供学习使用，不得商用，违者必究
  *
  * 微信公众号：    FZetc飞贼
  * 全视频平台：    FZetc飞贼
  * CSDN博客：      https://blog.csdn.net/qq_58629108?type=blog
  * 有疑问或者建议：FZetcSnitch@163.com
  *
  ******************************************************************************
  */
#ifndef __IIC_OLED_H
#define __IIC_OLED_H	


// 一、其它头文件
#include "stm32f4xx.h"                
#include "my_config.h"
#include "my_lib.h"
#include "stdlib.h"	 

// 二、宏定义(函数、变量、常量)
#define OLED_MODE   0
#define SIZE        8
#define XLevelL		0x00
#define XLevelH		0x10
#define Max_Column	128
#define Max_Row		64
#define	Brightness	0xFF 
#define X_WIDTH 	128
#define Y_WIDTH 	64	

// OLED IIC端口定义
#define IIC_OLED_SCL     PBout(15)
#define IIC_OLED_SDA_OUT PDout(10)
#define IIC_OLED_SDA_IN  PDin(10)

#define OLED_CMD  0											// 写命令
#define OLED_DATA 1											// 写数据

// 三、自定义的数据类型(结构体、联合体、枚举等)

// 四、全局变量声明

// 五、函数声明
// IIC协议(方工改的)
extern void    IIC_OLED_Start(void);
extern void    IIC_OLED_Stop(void);
extern void    IIC_OLED_SendByte(uint8_t byte);
extern uint8_t IIC_OLED_RecvAck(void);	

// OLED模块通信(方工改的)
extern void    IIC_OLED_Init(void);
extern int8_t  IIC_OLED_SelectMode(GPIOMode_TypeDef GPIO_Mode);

// OLED模块通信(别人的)
extern void OLED_WR_Byte(unsigned dat,unsigned cmd);  
extern void OLED_Display_On(void);
extern void OLED_Display_Off(void);	   							   		    
extern void OLED_Clear(void);
extern void OLED_DrawPoint(u8 x,u8 y,u8 t);
extern void OLED_Fill(u8 x1,u8 y1,u8 x2,u8 y2,u8 dot);
extern void OLED_ShowChar(u8 x,u8 y,u8 chr,u8 Char_Size);
extern void OLED_ShowNum(u8 x,u8 y,u32 num,u8 len,u8 size);
extern void OLED_ShowString(u8 x,u8 y, u8 *p,u8 Char_Size);	 
extern void OLED_Set_Pos(unsigned char x, unsigned char y);
extern void OLED_ShowCHinese(u8 x,u8 y,u8 no);
extern void OLED_DrawBMP(unsigned char x0, unsigned char y0,unsigned char x1, unsigned char y1,unsigned char BMP[]);
extern void Delay_50ms(unsigned int Del_50ms);
extern void Delay_1ms(unsigned int Del_1ms);
extern void fill_picture(unsigned char fill_Data);
extern void Picture(void);
extern void Write_IIC_Command(unsigned char IIC_Command);
extern void Write_IIC_Data(unsigned char IIC_Data);



// 六、静态变量、静态函数定义


#endif  /* __IIC_OLED_H */
	 



