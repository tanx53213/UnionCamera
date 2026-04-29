/**
  ******************************************************************************
  * @file    main.c
  * @author  FZetc飞贼
  * @version V0.0.1
  * @date    2026.04.15
  * @brief   说明：
  *				互斥锁+超声波和温湿度模块共用oled模块
  *			操作1：
  *            	1、保证stm32f4xx.h文件123行的HSE_VALUE时钟源值为8000000，保证system_stm32f4xx.c文件的316行的PLL_M系数为8
  *			   	2、GEC-M4板子上丝印层UART1跳线帽处，需要短接1-3和2-4，A-A口线要连接电脑和单片机USB口，保证调试通信和5V供电
  *            	3、串口助手的端口要打开连接开发板的那个串口端口，串口助手的波特率要和单片机串口程序保持一致
  *            	4、保证keil5软件和串口助手都是一个编码格式(推荐：GB2312格式(串口助手：GBK))   
  *
  *			操作2：连接超声波模块到开发板上
  *				CMAERA(13引脚，丝印层)-> DCMI_D5(网络标号) ->PB6(芯片引脚标号)  ===连接=== 超声波模块(TRIG引脚)
  * 			CMAERA(15引脚，丝印层)-> DCMI_D7(网络标号) ->PE6(芯片引脚标号)  ===连接=== 超声波模块(ECHO引脚)
  *
  *         操作3：连接oled模块到开发板上
  *				单片机(TFTLCD处)：                                  		OLED模块(IIC接口)：
  *             TFTLCD(BL)  -> LCD_BL   -> PB15(纯输出)(SCL)	            SCL引脚
  *             TFTLCD(D15)-> FSMC_D15 -> PD10(既有输出，又有输入)(SDA)     SDA引脚
  *
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
#include "FreeRTOS.h"		// 选编译配置文件，用于汇总所有源文件的编译选择控制
#include "task.h"			// 任务相关函数头文件
#include "semphr.h"			// 信号量头文件

#include "hc_sr04.h"
#include "dht11.h"
#include "iic_oled.h"
#include "iic_oled_bmp.h"
#include "iic_oled_font.h"


// 任务控制块指针(相当于Linux的线程标识符)
static TaskHandle_t app_task1_Handle;
static TaskHandle_t app_task2_Handle;

// 任务函数声明
static void app_task1( void * pvParameters );
static void app_task2( void * pvParameters );

// 互斥信号量
static SemaphoreHandle_t oled_sem_mutex_handle;

// 全局变量
float hc_sr04_dis    = 0;
uint8_t dht11_buf[5] = {0};


// 主函数
int main(void)
{	
	// 一、外设相关变量区域 
	BaseType_t xReturned;

	// 二、外设初始化区域
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4); // 选择中断分组为第4组(抢占优先级：16级(0-15)， 响应优先级：0级)
	// DELAY_SysTickInit(168);						// 系统定时器的时钟源初始化(168MHz或21MHz)
	UART1_Init(115200);								// 初始化串口A-A口线

	LED_Init();
	BUZZER_Init();
	KEY_Init();
	
	IIC_OLED_Init();
	OLED_Clear();
	
	// 三、创建互斥信号量
	oled_sem_mutex_handle = xSemaphoreCreateMutex();
	
	
	// 三、RTOS任务的创建并开启调度
	// 1、创建任务，并存储其标识符
    xReturned = xTaskCreate(
								app_task1,       	// 任务接口函数
								"app_task1",     	// 任务名字
								512,      			// 任务堆栈的大小(注意：这个大小指字(4字节)，而非字节)
								NULL,    			// 传递给任务的参数
								3,					// 任务创建时的优先等级(注意：优先级数字越小表示任务优先级越低(跟stm32相反)、优先级默认上限为(configMAX_PRIORITIES - 1))
								&app_task1_Handle 	// 任务控制块指针
							);     

    if( xReturned == pdPASS )
    {
		printf("app_task1 create success!\r\n");
    }
	
	// 2、创建任务，并存储其标识符
    xReturned = xTaskCreate(app_task2,"app_task2", 512,NULL,   4, &app_task2_Handle);     
    if( xReturned == pdPASS )
    {
		printf("app_task2 create success!\r\n");
    }
	


	// 四、开启任务调度(堵塞于此，让任务可以运行起来)
    vTaskStartScheduler();
}


/**
  * @brief  任务1函数实现
  * @note   超声波
  * @param  None 
  * @retval None
  */
static void app_task1( void * pvParameters )
{
	char buf[16] = {0};
	HC_SR04_Init();											// 超声波模块初始化	
    DELAY_MyNms(2000);
	
	for( ;; )		
    { 	
		hc_sr04_dis = HC_SR04_GetDis(3.5);
		if((hc_sr04_dis>20) && (hc_sr04_dis<4000))			// 测量距离	
		{
			// 获取锁
			xSemaphoreTake(oled_sem_mutex_handle, portMAX_DELAY);   
			
			// 保护的共享资源(printf、oled)
			// 显示中文
			OLED_ShowCHinese(18*0, 6, 0);	// 超声波：
			OLED_ShowCHinese(18*1, 6, 1);		
			OLED_ShowCHinese(18*2, 6, 2);	
			OLED_ShowCHinese(18*3, 6, 3);
			
			// 显示字符串
			sprintf(buf, "%0.0fmm", hc_sr04_dis);
			OLED_ShowString(18*4 ,6, (u8*)buf, 16);
			
			// 释放锁
			xSemaphoreGive(oled_sem_mutex_handle);
		}
			
		vTaskDelay(500);		
    }

}

/**
  * @brief  任务2函数实现
  * @note   按键密码
  * @param  None 
  * @retval None
  */
static void app_task2( void * pvParameters )
{
	char buf[16] = {0};
    int8_t  dht11_ret    =  0;
	DHT11_Init();											// dht11温湿度模块初始化
    DELAY_MyNms(6000);
	
	for( ;; )		// 相当于while(1)死循环
    {
		dht11_ret = DHT11_GetData(dht11_buf);
		if(dht11_ret == 0)
		{
			// 获取锁
			xSemaphoreTake(oled_sem_mutex_handle, portMAX_DELAY);   
			
			// 保护的共享资源(printf、oled)
			
			// 显示中文
			OLED_ShowCHinese(18*0, 2, 4);	// 温湿度：
			OLED_ShowCHinese(18*1, 2, 5);		
			OLED_ShowCHinese(18*2, 2, 6);	
			OLED_ShowCHinese(18*3, 2, 7);
			
			// 显示字符串
			sprintf(buf, "%dC,%d%%", dht11_buf[2]-2, dht11_buf[0]);
			OLED_ShowString(18*4 ,2, (u8*)buf, 16);  
			
			// 释放锁
			xSemaphoreGive(oled_sem_mutex_handle);	
		}

		vTaskDelay(6000);								 	// 最低响应时间6S，最高是15S(湿度)/30S(温度)	    }
	}
}
