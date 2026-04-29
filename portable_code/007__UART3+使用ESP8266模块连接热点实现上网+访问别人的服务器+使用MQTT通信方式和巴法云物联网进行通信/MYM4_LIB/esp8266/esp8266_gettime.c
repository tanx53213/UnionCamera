#include "esp8266.h"
#include "esp8266_gettime.h"
#include "uart.h"
#include "delay.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
  * @brief  通过ESP8266模块获取网络时间
  * @note   时间API来自于(https://quan.suning.com/getSysTime.do)
  * @param  time_p：指向一个时间结构体变量的指针
  * @retval None
  */
int8_t ESP8266_GetTimeFunc(time_p p)
{
	int8_t ret    = 0;
	int16_t i     = 0;
	char *str     = NULL;
	char buf[128] = {0};
	
	
	// 1、与网站建立连接
	ret =  ESP8266_ConnectServer("TCP", "quan.suning.com", 80);
	if(ret < 0)
	{
		printf("与网站建立连接失败\r\n");
		return -4;
	}
	delay_ms(500);
	
	
	// 2、开启透传模式
	ret =  ESP8266_EnterTransparentTransmission();
	if(ret < 0)
	{
		printf("开启透传模式失败\r\n");
		return -6;
	}
	delay_ms(500);

	
	// 3、向网站提出请求
	ESP8266_SendATCmd("GET \"https://quan.suning.com/getSysTime.do\"");
	delay_ms(500);

	
	// 4、暴力解析抓取到的时间
	// a、截取到date:的片段
	// printf("u3_recvbuf == %s\r\n", u3_recvbuf);
	
	str = strstr((const char*)u3_recvbuf, (const char*)"Date:");
	str = str+6;
	// printf("str == %s\r\n", str);
	
	// b、将date: 后面到GMT之前的字符串放置到buf数组中
	while(*str++ != '\r')
	{
		buf[i++] = *(str-1);
	}
	buf[i] = '\0';
	// printf("buf == %s\r\n", buf);
	
	// c、将buf中有','和':'去除，改成' '，为了方便sscanf函数的处理
	i = 0;
	while(buf[i++] != '\0')
	{
		if( (buf[i] == ',') || (buf[i] == ':'))
		{
			buf[i] = ' ';
		}
	}
	// printf("buf == %s\r\n", buf);
	
	// d、使用sscanf函数将各个参数放置到时间结构体里面
	sscanf(buf, "%s %s %s %s %s %s %s %s", 
								p->weekday,
								p->date,
								p->month,
								p->year,
								p->hour,
								p->min,
								p->second,
								p->GMT
	       );
	
	
	// 清空接收缓冲区
	memset((u8*)u3_recvbuf, 0, sizeof(u3_recvbuf));
	
	// 清空接收计数值
	u3_count = 0;
	delay_ms(500);
		
	// 5、退出透传模式(断开连接)
	ret = ESP8266_ExitTransparentTransmission();
	if(ret < 0)
	{
		printf("退出透传模式(断开连接)失败\r\n");
		return -1;
	}
	delay_ms(500);

	
	return 0;
}

