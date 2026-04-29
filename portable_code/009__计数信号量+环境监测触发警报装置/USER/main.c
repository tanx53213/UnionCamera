/**
  ******************************************************************************
  * @file    main.c
  * @author  FZetc飞贼
  * @version V0.0.1
  * @date    2026.04.14
  * @brief   说明：
  *				计数信号量+环境监测触发警报装置
  *
  *			 操作1：
  *            	1、保证stm32f4xx.h文件123行的HSE_VALUE时钟源值为8000000，保证system_stm32f4xx.c文件的316行的PLL_M系数为8
  *			   	2、GEC-M4板子上丝印层UART1跳线帽处，需要短接1-3和2-4，A-A口线要连接电脑和单片机USB口，保证调试通信和5V供电
  *            	3、串口助手的端口要打开连接开发板的那个串口端口，串口助手的波特率要和单片机串口程序保持一致
  *            	4、保证keil5软件和串口助手都是一个编码格式(推荐：GB2312格式、utf-8格式) 
  *
  *			 操作2：连接超声波模块到开发板上
  *				CMAERA(13引脚，丝印层)-> DCMI_D5(网络标号) ->PB6(芯片引脚标号)  ===连接=== 超声波模块(TRIG引脚)
  * 			CMAERA(15引脚，丝印层)-> DCMI_D7(网络标号) ->PE6(芯片引脚标号)  ===连接=== 超声波模块(ECHO引脚)
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

// 任务控制块指针(相当于Linux的线程标识符)
static TaskHandle_t app_task1_Handle;
static TaskHandle_t app_task2_Handle;
static TaskHandle_t app_task3_Handle;

// 任务函数声明
static void app_task1( void * pvParameters );
static void app_task2( void * pvParameters );
static void app_task3( void * pvParameters );

// 计数信号量
static SemaphoreHandle_t  em_count_sem_handle;

// 环境监测全局变量
double   hc_sr04_dis = 0;
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
	
	
	// 三、创建计数型信号量、计数值增到最大值100就不能再增加了，初始值：0(一共有100个资源，一开始放出0个资源)
	em_count_sem_handle =  xSemaphoreCreateCounting(100, 0);
	
	
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
	
	// 3、创建任务，并存储其标识符
    xReturned = xTaskCreate(app_task3,"app_task3", 512, NULL,   4, &app_task3_Handle);     
    if( xReturned == pdPASS )
    {
		printf("app_task3 create success!\r\n");
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
	HC_SR04_Init(); // 超声波hc_sr04模块初始化
	DELAY_MyNms(1000);
	
    for( ;; )		// 相当于while(1)死循环
    {
		hc_sr04_dis = HC_SR04_GetDis(3.5);
		if((hc_sr04_dis>20) && (hc_sr04_dis<4000))
		{
			printf("hc_sr04 == %0.2fmm\r\n", hc_sr04_dis);
			if(hc_sr04_dis < 100)							// 判断是否有人长时间站在你门口，将此异常数据发送到环境警报装置任务并进行处理
			{
				xSemaphoreGive(em_count_sem_handle);
				printf("chaosheng触发报警装置!\r\n");
			}
		}
		vTaskDelay(500);
    }
}

/**
  * @brief  任务2函数实现
  * @note   温湿度
  * @param  None 
  * @retval None
  */
static void app_task2( void * pvParameters )
{
	int8_t  dht11_ret    =  0;

	DHT11_Init();	// 温湿度dht11模块初始化
	DELAY_MyNms(1000);
	
	for( ;; )		// 相当于while(1)死循环
    {
		dht11_ret = DHT11_GetData(dht11_buf);
		if(dht11_ret == 0)
		{
			printf("温度：%d.%d℃ \r\n",   dht11_buf[2], dht11_buf[3]);
			printf("湿度：%d.%d%%RH \r\n", dht11_buf[0], dht11_buf[1]);
			
			if(dht11_buf[2] > 30 || dht11_buf[2] > 90)		// 判断温湿度是否有异常，有异常将这个情况报告给环境警报装置任务实现
			{
				xSemaphoreGive(em_count_sem_handle);
				printf("温湿度触发警报!\r\n");
			}
				
		}
		else
		{
			printf("错误码：%d\r\n", dht11_ret);
		}
		vTaskDelay(6000);	
    }
}

/**
  * @brief  任务3函数实现
  * @note   环境警报装置实现
  * @param  None 
  * @retval None
  */
static void app_task3( void * pvParameters )
{
	int16_t count = 0;
    for( ;; )		// 相当于while(1)死循环
    {
		// 等待信号量
		xSemaphoreTake( em_count_sem_handle, portMAX_DELAY);
		count++;
		printf("警报累计触发次数 == %d\r\n", count);
		
		if(count > 5)
		{
			// 打印异常信息
			printf("\r\n####################################\r\n");
			printf("异常值(温湿度)监控：\r\n");
			printf("温度：%d.%d℃\r\n",   dht11_buf[2], dht11_buf[3]);
			printf("湿度：%d.%d%%RH\r\n", dht11_buf[0], dht11_buf[1]);
			printf("\r\n");
			printf("异常值(超声波)监控：\r\n");
			printf("超声波距离 == %0.2fmm\r\n", hc_sr04_dis);
			printf("####################################\r\n");
			
			// 蜂鸣器响
			BUZZER_WarnVoice(500, 5);	
			count = 0;
		}
		
    }
}

