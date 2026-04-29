/**
  ******************************************************************************
  * @file    main.c
  * @author  FZetc飞贼
  * @version V0.0.1
  * @date    2026.01.16
  * @brief   说明：
  *				UART3+使用ESP8266模块连接热点实现上网+访问别人的服务器+使用MQTT通信方式和巴法云物联网进行通信
  *
  *          在单片机上的操作：
  *          	操作1：
  *            		1、保证stm32f4xx.h文件123行的HSE_VALUE时钟源值为8000000，保证system_stm32f4xx.c文件的316行的PLL_M系数为8
  *			   		2、GEC-M4板子上丝印层UART1跳线帽处，需要短接1-3和2-4，A-A口线要连接电脑和单片机USB口，保证调试通信和5V供电
  *            		3、串口助手的端口要打开连接开发板的那个串口端口，串口助手的波特率要和单片机串口程序保持一致
  *            		4、保证keil5软件和串口助手都是一个编码格式(推荐：GB2312格式或utf格式)
  *          
  *          	操作2：
  *            		1、wifi模块连接开发板板左上角ESP8266接口处
  *            		2、UART3跳线帽连接3-5，4-6
  *            		3、如果想要ESP8266模块回发数据，需要将uart.h里面的宏定义ESP8266_DEBUG置位1即可
  *               #define UART3_ESP8266_RECV_MSG_DEBUG 1
  *
  *          	操作3：
  *            		1、在esp8266.h文件中修改自己SSID和PSWD，否则用不了网络(手机热点设置成2.4Ghz频段)
  *					#define SSID "xxxxx"
  *					#define PSWD "xxxxx" 
  *  
  * 			操作4：
  *            		1、在esp8266_bemfayun_mqtt.h文件中修改自己的私钥BEMFAYUN_MQTT_SIYAO
  *                 #define BEMFAYUN_MQTT_SIYAO "xxxxx"
  *
  *          	操作5：
  *            		点击魔法棒->target->Use microLIB
  *				
  *
  *			在巴法云上的操作：
  *				1、获取巴法云上的私钥
  *				2、在控制台上创建好mqtt通信设备(一个002设备(设置)，一个004设备(获取))
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
  
#include "main.h"
#include "esp8266.h"
#include "esp8266_gettime.h"
#include "esp8266_getweather.h"
#include "esp8266_bemfayun_tcp.h"
#include "esp8266_bemfayun_mqtt.h"
#include "dht11.h"


// 全局变量声明
extern int esp8266_transparent_transmission_sta;

// 主函数
int main(void)
{
	// 零、外设相关变量区域 
	uint16_t count 		  =  0;
	int8_t   dht11_ret    =  0;
	uint8_t  dht11_buf[5] = {0};
	char     buf[32] 	  = {0};
	
	// 一、外设初始化区域
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); 		// 选择中断分组为第2组(抢占优先级：4级， 响应优先级：4级)
	DELAY_SysTickInit(168);									// 系统定时器的时钟源初始化(168MHz或21MHz)
	UART1_Init(115200);										// 串口a-a口线初始化
	
	LED_Init();
	KEY_Init();
	BUZZER_Init();
	
	DHT11_Init();											// dht11温湿度模块初始化
	TIMER3_Init();											// 定时器定时发送心跳包给巴法云
	ESP8266_Init(115200);									// ESP8266模块初始化
	delay_ms(1000);
	printf("This is ESP8266模块 test\r\n");			
	
	// 二、外设功能具现化区域
	// 1、使得你的ESP8266模块连接热点，能够上网
	printf("##### ESP8266模块连接网络中 #####\r\n");
	while(ESP8266_OnLineInit() != 0)
	{
		printf("ESP8266模块连接网络中.......\r\n\r\n");
		delay_ms(1000);
	}
	printf("##### ESP8266模块连接网络成功 #####\r\n");
	
	
	// 2、连接巴法云服务器，并向其订阅主题
	printf("##### 连接巴法云物联网服务器中...... #####\r\n");
	ESP8266_BemFaYun_Mqtt_Init();
	printf("##### 连接巴法云物联网服务器成功 #####\r\n");
	

	while(1)												
	{
		
		// 3、接收来自mqtt服务器的信息(比如：控制蜂鸣器)
		if(u3_flag == 1 && esp8266_transparent_transmission_sta == 1)
		{
			/*	打印信息
				for(i=0; i<u3_len; i++)
				{
					 if(u3_recvbuf[i] >= 32 && u3_recvbuf[i] <= 126)
								printf("%c", u3_recvbuf[i]);  // 打印可显示字符
							else
								printf(".");  // 不可显示字符用点代替
				}
			*/
		
			if( strstr((const char*)u3_recvbuf+5, "on"))
			{
				BUZZER(ON);
			}
			
			if( strstr((const char*)u3_recvbuf+5, "off"))
			{
				BUZZER(OFF);
			}

			// 清空相关的变量，方便下次运行
			MY_LIB_ClearArray((int8_t *)u3_recvbuf, 512);
			u3_count = 0;
			u3_flag  = 0;
		}
		
		
		// 4、上传信息到mqtt服务器(比如：发送温湿度数据)
		count++;
		delay_ms(1);
		if(count == 2000)
		{
			// 获取温湿度数据
			dht11_ret = DHT11_GetData(dht11_buf);
			if(dht11_ret == 0)
			{
				// 打印数据
				printf("温度：%d.%d℃\r\n",   dht11_buf[2], dht11_buf[3]);
				printf("湿度：%d.%d%%RH\r\n", dht11_buf[0], dht11_buf[1]);
				
				// 拼接数据(小程序获取温湿度数据的时候是用#号隔开的)
				sprintf(buf, "#%d.%d#%d.%d%", dht11_buf[2], dht11_buf[3], dht11_buf[0], dht11_buf[1]);
				
				// 上传数据
				ESP8266_MQTT_PublishData("dht11mqtt004", buf, 1);
			}	
			
			count = 0;
		}
		
		
	}

}






