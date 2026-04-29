/**
  ******************************************************************************
  * @file    esp8266_bemfayun_mqtt.h
  * @author  FZetc飞贼
  * @version V0.0.1
  * @date    2026.01.16
  * @brief   1、通过esp8266模块、mqtt协议和巴法云相互通信
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
  *	
  */
  
#ifndef __ESP8266_BEMFAYUN_MQTT_H
#define __ESP8266_BEMFAYUN_MQTT_H


// 一、其它头文件
#include "stm32f4xx.h"   		// Device header
#include "my_config.h"
#include "my_lib.h"
#include "my_reg.h"

// 二、自定义的数据类型(结构体、联合体、枚举等)

// 三、宏定义(函数、变量、常量)
#define BEMFAYUN_MQTT_SIYAO ""

#define BYTE0(dwTemp)       (*( char *)(&dwTemp))
#define BYTE1(dwTemp)       (*((char *)(&dwTemp) + 1))
#define BYTE2(dwTemp)       (*((char *)(&dwTemp) + 2))
#define BYTE3(dwTemp)       (*((char *)(&dwTemp) + 3))

// 四、变量声明

// 五、函数声明
extern int8_t   ESP8266_BemFaYun_Mqtt_Init(void);
extern int8_t   ESP8266_BemFaYun_Mqtt_Connect(char *client_id, char *user_name,char *password);

extern int8_t   ESP8266_MQTT_SubscribeTopic(const char *topic,uint8_t qos,uint8_t whether);
extern uint32_t ESP8266_MQTT_PublishData(const char *topic, const char *message, uint8_t qos);
extern void     ESP8266_MQTT_ListenForMessages(void);
extern int8_t   ESP8266_MQTT_SendPingReq(void);
extern int8_t   ESP8266_MQTT_ProcessPingResp(void);
extern int8_t   ESP8266_MQTT_ProcessPingResp(void);


#endif /* __ESP8266_BEMFAYUN_MQTT_H */
