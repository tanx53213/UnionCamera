#include "esp8266.h"
#include "esp8266_bemfayun_tcp.h"
#include "delay.h"
#include "uart.h"
#include <stdio.h>
#include <string.h>


/**
  * @brief  通过ESP8266模块连接巴法云服务器
  * @note   None
  * @param  None
  * @retval 成功：返回0
  *         失败：返回小于0的数
  */
int32_t ESP8266_BemFaYun_Tcp_Connect(void)
{
	int8_t ret    = 0;
	// 1、与网站建立连接
	ret =  ESP8266_ConnectServer("TCP", "bemfa.com", 8344);
	if(ret < 0)
	{
		printf("与网站建立连接失败\r\n");
		return -1;
	}
	printf("01：与网站建立连接成功\r\n");
	delay_ms(1000);		

		
	// 2、开启透传模式
	ret =  ESP8266_EnterTransparentTransmission();
	if(ret < 0)
	{
		printf("开启透传模式失败\r\n");
		return 2;
	}
	printf("02：开启透传模式成功\r\n");
	delay_ms(1000);	
	
	return 0;
}



/**
  * @brief  订阅主题
  * @note   None
  * @param  key：  巴法云的私钥
  *         topic：巴法云的主题
  * @retval 成功：返回0
  */
int32_t ESP8266_BemFaYun_Tcp_Subscribe(const char *key, const char *topic)
{
	char sub_buf[512] = {0};

	// 3、向巴法云发送订阅指令
    sprintf(sub_buf,"cmd=1&uid=%s&topic=%s\r\n", key, topic);
	ESP8266_SendATCmd(sub_buf);
	delay_ms(500);	
	
    return 0;
}


/**
  * @brief  向主题发布信息
  * @note   None
  * @param  key：  巴法云的私钥
  *         topic：巴法云的主题
  * @retval 成功：返回0
  */
int32_t ESP8266_BemFaYun_Tcp_Publish(const char *key, const char *topic,  const char *msg)
{
	char pub_buf[512] = {0};
	
	// 3、向巴法云发送发布指令
    sprintf(pub_buf,"cmd=2&uid=%s&topic=%s&msg=%s\r\n", key, topic, msg);
	ESP8266_SendATCmd(pub_buf);
	delay_ms(500);	

	
    return 0;
}

