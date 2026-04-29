#include "esp8266.h"           
#include "esp8266_bemfayun_mqtt.h"  
#include "delay.h"             
#include "uart.h"              
#include <stdio.h>             
#include <string.h>    

// 全局变量定义
const uint8_t g_packet_heart_reply[2] = {0xc0,0x00};  	// MQTT心跳回复报文
char g_mqtt_msg[526];                           	  	// MQTT消息缓冲区
uint32_t g_mqtt_tx_len;                         	  	// MQTT发送数据长度
volatile uint8_t g_is_sending_data = 0;         		// 数据发送状态标志

// 全局变量声明（外部定义）
extern int esp8266_transparent_transmission_sta; 		// ESP8266透传模式状态


/**
  * @brief  通过ESP8266模块连接到云物联网
  * @note   实现MQTT连接的完整初始化流程
  * @param  None
  * @retval 成功：返回0
  *         失败：返回小于0的数
  */
int8_t ESP8266_BemFaYun_Mqtt_Init(void)
{
    int8_t ret = 0;  

    // 1、与巴法云服务器建立TCP连接
    ret = ESP8266_ConnectServer("TCP", "bemfa.com", 9501);
    if(ret < 0)  
    {
        printf("与云物联网站建立连接失败\r\n");
        return -1;  
    }
    delay_ms(500);  
    printf("01：与云物联网站建立连接成功!\r\n");

    // 2、开启ESP8266透传模式
    ret = ESP8266_EnterTransparentTransmission();
    if(ret < 0)  
    {
        printf("开启透传模式失败\r\n");
        return -2;  
    }
    delay_ms(500);  
    printf("02：开启透传模式成功!\r\n");

    // 3、连接云物联网的MQTT服务器
    if(ESP8266_BemFaYun_Mqtt_Connect(BEMFAYUN_MQTT_SIYAO, "", ""))  // 无用户名和密码
    {
        printf("连接云物联网的MQTT失败\r\n");
        return -3;  
    }
    delay_ms(500);  
    printf("03：连接云物联网的MQTT成功!\r\n");

    // 4、订阅云物联网的MQTT主题
    if(ESP8266_MQTT_SubscribeTopic("buzzermqtt002", 0, 1))  			// 订阅主题，QoS=0
    {
        printf("MQTT订阅失败!\r\n");
        return -4;  
    }
    else
    {
        printf("04：MQTT订阅成功!\r\n");
    }

    return 0; 
}

/**
  * @brief  MQTT发送数据
  * @note   通过ESP8266模块发送MQTT报文
  * @param  buf：指向要发送给云物联网MQTT的数据的内存的指针
  *         len：要发送数据的长度
  * @retval None
  */
void ESP8266_MQTT_SendBytes(uint8_t *buf, uint32_t len)
{
    g_is_sending_data = 1;  	// 设置发送状态标志为忙碌
    UART3_SendBytes(buf, len);  // 通过UART3发送数据到ESP8266
    g_is_sending_data = 0;  	// 清除发送状态标志
}

/**
  * @brief  MQTT连接服务器的连接函数
  * @note   实现MQTT CONNECT报文的构建和发送
  * @param  client_id：设备名字(巴法云的私钥)
  *         user_name：产品密钥(不需要，填空)
  *         password：密码(不需要，填空)
  * @retval 成功：返回0
  *         失败：返回-1
  */
int8_t ESP8266_BemFaYun_Mqtt_Connect(char *client_id, char *user_name, char *password)
{
    uint32_t client_id_len = strlen(client_id);  // 客户端ID长度
    uint32_t user_name_len = strlen(user_name);  // 用户名长度
    uint32_t password_len = strlen(password);    // 密码长度
    uint32_t data_len;  // MQTT报文数据长度
    uint32_t cnt = 2;   // 尝试连接次数
    uint32_t wait = 0;  // 等待时间
    // uint32_t i = 0;  // 循环变量（注释）
    g_mqtt_tx_len = 0;  // 发送缓冲区长度清零


    // (1)、构建固定报头
    // 控制报文类型：CONNECT (0x10)
    u3_sendbuf[g_mqtt_tx_len++] = 0x10;   // MQTT消息类型：CONNECT

	 // (2)、计算可变报头+Payload长度（仅包含客户端ID）
    data_len = 10 + (client_id_len + 2);  // 10字节固定报头，客户端ID长度+2字节长度标识
	
    // 剩余长度编码（不包括固定头部）
    do
    {
        uint8_t encodedByte = data_len % 128;  
        data_len = data_len / 128;  
        if (data_len > 0)  
            encodedByte = encodedByte | 128;  
        u3_sendbuf[g_mqtt_tx_len++] = encodedByte;  
    } while (data_len > 0);

    // (3)、构建可变报头
    // 协议名：MQTT
    u3_sendbuf[g_mqtt_tx_len++] = 0;         // 协议名长度 MSB
    u3_sendbuf[g_mqtt_tx_len++] = 4;         // 协议名长度 LSB
    u3_sendbuf[g_mqtt_tx_len++] = 'M';       // ASCII码：M
    u3_sendbuf[g_mqtt_tx_len++] = 'Q';       // ASCII码：Q
    u3_sendbuf[g_mqtt_tx_len++] = 'T';       // ASCII码：T
    u3_sendbuf[g_mqtt_tx_len++] = 'T';       // ASCII码：T
    // 协议级别：MQTT 3.1.1
    u3_sendbuf[g_mqtt_tx_len++] = 4;         // MQTT协议版本 = 4
    // 连接标志：清理会话
    u3_sendbuf[g_mqtt_tx_len++] = 0x02;      // 连接标志 - 只设置清理会话
    // 心跳间隔：60秒
    u3_sendbuf[g_mqtt_tx_len++] = 0;         // 心跳时间长度 MSB
    u3_sendbuf[g_mqtt_tx_len++] = 60;        // 心跳时间长度 LSB

    // (4)、构建客户端标识
    u3_sendbuf[g_mqtt_tx_len++] = BYTE1(client_id_len);  						// 客户端ID长度 MSB
    u3_sendbuf[g_mqtt_tx_len++] = BYTE0(client_id_len);  						// 客户端ID长度 LSB
    memcpy((void*)&u3_sendbuf[g_mqtt_tx_len], client_id, client_id_len);  		// 复制客户端ID
    g_mqtt_tx_len += client_id_len;  											// 更新发送长度

    // 如果有用户名，添加用户名字段
    if(user_name_len > 0)
    {
        u3_sendbuf[g_mqtt_tx_len++] = BYTE1(user_name_len);  					// 用户名长度 MSB
        u3_sendbuf[g_mqtt_tx_len++] = BYTE0(user_name_len);  					// 用户名长度 LSB
        memcpy((void*)&u3_sendbuf[g_mqtt_tx_len], user_name, user_name_len);  	// 复制用户名
        g_mqtt_tx_len += user_name_len; 								 		// 更新发送长度
    }

    // 如果有密码，添加密码字段
    if(password_len > 0)
    {
        u3_sendbuf[g_mqtt_tx_len++] = BYTE1(password_len);  					// 密码长度 MSB
        u3_sendbuf[g_mqtt_tx_len++] = BYTE0(password_len);  					// 密码长度 LSB
        memcpy((void*)&u3_sendbuf[g_mqtt_tx_len], password, password_len);  	// 复制密码
        g_mqtt_tx_len += password_len;  										// 更新发送长度
    }

    // (5)、尝试发送连接请求（最多尝试2次）
    while(cnt--)
    {
        memset((void *)u3_recvbuf, 0, sizeof(u3_recvbuf));  					// 清空接收缓冲区
        u3_count = 0;  															// 接收计数器清零

        // 发送MQTT连接报文
        ESP8266_MQTT_SendBytes((u8*)u3_sendbuf, g_mqtt_tx_len);
        wait = 3000;  

		// (6)、对服务端的响应进行解析
        while(wait--)  
        {
            delay_ms(1);  

            // 检查连接确认固定报头
            if((u3_recvbuf[0] == 0x20) && (u3_recvbuf[1] == 0x02))
            {
                if(u3_recvbuf[3] == 0x00)  	// 连接确认成功
                {
                    printf("连接已被服务器端接受，连接确认成功\r\n");
                    return 0;  
                }
                else  						// 连接被拒绝
                {
                    switch(u3_recvbuf[3])  	// 根据返回码判断拒绝原因
                    {
                        case 1: printf("连接已拒绝，不支持的协议版本\r\n"); break;
                        case 2: printf("连接已拒绝，不合格的客户端标识符\r\n"); break;
                        case 3: printf("连接已拒绝，服务端不可用\r\n"); break;
                        case 4: printf("连接已拒绝，无效的用户或密码\r\n"); break;
                        case 5: printf("连接已拒绝，未授权\r\n"); break;
                        default: printf("未知响应\r\n"); break;
                    }
                    return -1;  			// 连接失败
                }
            }
        }
    }

    return -1;  // 连接超时失败
}


/**
  * @brief  MQTT订阅/取消订阅数据打包函数
  * @note   实现MQTT SUBSCRIBE/UNSUBSCRIBE报文的构建和发送
  * @param  topic：主题
  *         qos：消息等级
  *         whether：订阅/取消订阅请求包（1=订阅，0=取消订阅）
  * @retval 成功：返回0(订阅成功)
  *         失败：返回小于0的数据
  */
int8_t ESP8266_MQTT_SubscribeTopic(const char *topic, uint8_t qos, uint8_t whether)
{
    uint32_t cnt = 2;       			// 尝试订阅次数
    uint32_t wait = 0;      			// 等待时间
	uint32_t data_len = 0;
    uint32_t topiclen = strlen(topic);  // 主题长度
    g_mqtt_tx_len = 0;					// 发送缓冲区长度清零

    // (1)、构造固定报头
    if(whether)  
        u3_sendbuf[g_mqtt_tx_len++] = 0x82;  // 消息类型和标志：SUBSCRIBE
    else  
        u3_sendbuf[g_mqtt_tx_len++] = 0xA2;  // 消息类型和标志：UNSUBSCRIBE

	// (2)、计算可变报头+有效载荷长度
    data_len = 2 + (topiclen + 2) + (whether ? 1 : 0);  // 2字节报文标识符，主题长度+2，QoS(可选)
	
    // 剩余长度编码
    do
    {
        uint8_t encodedByte = data_len % 128; 
        data_len = data_len / 128;  
        if(data_len > 0) 
            encodedByte = encodedByte | 128;  
        u3_sendbuf[g_mqtt_tx_len++] = encodedByte; 
    } while(data_len > 0);

    // (3)、构造可变报头：报文标识符
    u3_sendbuf[g_mqtt_tx_len++] = 0;        		// 消息标识符 MSB
    u3_sendbuf[g_mqtt_tx_len++] = 0x01;     		// 消息标识符 LSB

    // (4)、有效载荷：主题
    u3_sendbuf[g_mqtt_tx_len++] = BYTE1(topiclen);  // 主题长度 MSB
    u3_sendbuf[g_mqtt_tx_len++] = BYTE0(topiclen);  // 主题长度 LSB
    memcpy((void*)&u3_sendbuf[g_mqtt_tx_len], topic, topiclen);  // 复制主题

    g_mqtt_tx_len += topiclen;  					// 更新发送长度

    // 如果是订阅，添加QoS级别
    if(whether)
    {
        u3_sendbuf[g_mqtt_tx_len++] = qos;  		// QoS级别
    }

    // (5)、尝试发送订阅请求（最多尝试2次）
    while(cnt--)
    {
        memset((void *)u3_recvbuf, 0, sizeof(u3_recvbuf));  
        u3_count = 0;  

        // 发送订阅报文
        ESP8266_MQTT_SendBytes((u8*)u3_sendbuf, g_mqtt_tx_len);
        // printf("发送订阅报文，尝试 %d\r\n", 5 - cnt);  

		// (6)、对服务端的响应进行解析
        wait = 3000;  
        while(wait--)
        {
            delay_ms(1);  

            // 检查是否有数据接收（同时检查u3_flag和u3_count）
            if(u3_flag || u3_count > 0)
            {
                // 获取实际接收长度
                uint16_t recv_len = u3_flag ? u3_len : u3_count;
                // printf("收到数据，长度: %d\r\n", recv_len);
                // printf("首字节: 0x%02X\r\n", u3_recvbuf[0]);

                // 检查订阅确认报头
                if(u3_recvbuf[0] == 0x90)  // SUBACK报文
                {
                    printf("订阅主题确认成功\r\n");

                    // 正确解析订阅确认报文
                    if(recv_len >= 4)  // 确保数据长度足够
                    {
                        uint8_t packet_id_msb = u3_recvbuf[2];  // 报文标识符MSB
                        uint8_t packet_id_lsb = u3_recvbuf[3];  // 报文标识符LSB
                        printf("报文标识符: 0x%02X%02X\r\n", packet_id_msb, packet_id_lsb);

                        // 解析订阅结果
                        if(recv_len > 4)  // 有订阅结果码
                        {
                            uint8_t return_code = u3_recvbuf[4];  // 订阅结果码
                            switch(return_code)
                            {
                                case 0: printf("订阅成功，QoS: 0\r\n"); break;
                                case 1: printf("订阅成功，QoS: 1\r\n"); break;
                                case 2: printf("订阅成功，QoS: 2\r\n"); break;
                                case 0x80: printf("订阅失败\r\n"); break;
                                default: printf("订阅返回未知代码: 0x%02X\r\n", return_code); break;
                            }
                        }
                    }

                    // 重置标志
                    u3_flag = 0;
                    u3_count = 0;
                    return 0;  // 订阅成功
                }

                // 重置标志
                u3_flag = 0;
                u3_count = 0;
            }
        }
    }

    printf("订阅超时，未收到服务器确认\r\n");
    return -1;  // 订阅失败
}


/**
  * @brief  MQTT发布数据
  * @note   实现MQTT PUBLISH报文的构建和发送
  * @param  topic：主题
  *         message：消息内容
  *         qos：消息质量等级
  * @retval 成功：返回消息长度
  *         失败：返回0
  */
uint32_t ESP8266_MQTT_PublishData(const char *topic, const char *message, uint8_t qos)
{
    uint32_t topiclen = strlen(topic);  // 主题长度
    uint32_t msglen = strlen(message);  // 消息长度
	uint32_t data_len = 0;
    g_mqtt_tx_len = 0;  				// 发送缓冲区长度清零

    // (1)、构造固定报头
    // 控制报文类型: PUBLISH (0x30)
    u3_sendbuf[g_mqtt_tx_len++] = 0x30;  // PUBLISH报文

    // 设置QoS标志
    if(qos > 0)
    {
        u3_sendbuf[0] |= (qos << 1);  // 设置QoS位
    }
		
    // (2)、计算数据长度：2字节主题长度 + 主题内容 + 消息内容
   data_len = 2 + (topiclen + 2) + (msglen + 0);
    // 剩余长度编码
    do
    {
        uint8_t encodedByte = data_len % 128;  
        data_len = data_len / 128;  
        if(data_len > 0) 
            encodedByte = encodedByte | 128; 
        u3_sendbuf[g_mqtt_tx_len++] = encodedByte;  
    } while(data_len > 0);

    // (3)、构造可变报头 - 主题名
    u3_sendbuf[g_mqtt_tx_len++] = BYTE1(topiclen);  			// 主题长度 MSB
    u3_sendbuf[g_mqtt_tx_len++] = BYTE0(topiclen);  			// 主题长度 LSB
    memcpy((void*)&u3_sendbuf[g_mqtt_tx_len], topic, topiclen); // 复制主题
    g_mqtt_tx_len += topiclen;  								// 更新发送长度

    // 报文标识符 (仅当QoS > 0时需要)
    if(qos > 0)
    {
        u3_sendbuf[g_mqtt_tx_len++] = 0;  // 报文标识符 MSB
        u3_sendbuf[g_mqtt_tx_len++] = 1;  // 报文标识符 LSB
    }
	
    // (4)、有效载荷 - 消息内容
    memcpy((void*)&u3_sendbuf[g_mqtt_tx_len], message, msglen);  // 复制消息内容
    g_mqtt_tx_len += msglen;  									 // 更新发送长度

    // (5)、发送发布报文
    ESP8266_MQTT_SendBytes((u8*)u3_sendbuf, g_mqtt_tx_len);
    // printf("g_mqtt_tx_len == %d\r\n", g_mqtt_tx_len);  			 // 打印发送长度

    delay_ms(500);  											 // 等待发送完成

    return g_mqtt_tx_len;  										 // 返回发送的数据长度
}

/**
  * @brief  MQTT发送心跳包
  * @note   定期发送PINGREQ报文，保持连接活跃
  * @param  None
  * @retval 成功：返回0
  *         失败：返回-1
  */
int8_t ESP8266_MQTT_SendPingReq(void)
{
    g_mqtt_tx_len = 0;  // 发送缓冲区长度清零

    // MQTT PINGREQ报文
    // 固定报头：0xC0 (PINGREQ), 0x00 (剩余长度)
    u3_sendbuf[g_mqtt_tx_len++] = 0xC0;
    u3_sendbuf[g_mqtt_tx_len++] = 0x00;

    printf("发送MQTT心跳包\r\n");

    // 发送心跳包
    ESP8266_MQTT_SendBytes((u8*)u3_sendbuf, g_mqtt_tx_len);

    delay_ms(100);  // 等待发送完成

    return 0;  		// 发送成功
}

/**
  * @brief  处理MQTT心跳包回复
  * @note   检查是否收到PINGRESP报文
  * @param  None
  * @retval 收到回复：返回0
  *         未收到回复：返回-1
  */
int8_t ESP8266_MQTT_ProcessPingResp(void)
{
    // 检查是否收到PINGRESP报文
    // PINGRESP报文：0xD0 (PINGRESP), 0x00 (剩余长度)
    if((u3_recvbuf[0] == 0xD0) && (u3_recvbuf[1] == 0x00))
    {
        printf("收到MQTT心跳包回复\r\n");
        // 清空接收缓冲区
        memset((void*)u3_recvbuf, 0, sizeof(u3_recvbuf));
        u3_count = 0;
        return 0;  	// 收到回复
    }

    return -1;  	// 未收到回复
}
