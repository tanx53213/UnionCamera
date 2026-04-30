#include "font.h"
#include"showtime.h"
#include "camera.h"
#include "right_panel.h"
#include <pthread.h>

// LCD_DEV, LCD_MAPSIZE 已在 camera.h 中定义
// 天气结构体

// lcd_mp/program_running/lcd_mutex 均已在 main.c 定义，通过 camera.h extern 引用

char *show_time();
extern int Get_weather(weather1 *wea_p);
char my_pic_pictr_path[256] ={0};
struct LcdDevice *lcd;


struct LcdDevice *init_lcd(const char *device)//打开与映射LCD
{
	//申请空间
	struct LcdDevice* lcd = malloc(sizeof(struct LcdDevice));//为字体申请空间
	if(lcd == NULL)
	{
		perror("malloc failed");
		return NULL;
	} 

	//1打开设备
	lcd->fd = open(device, O_RDWR);
	if(lcd->fd < 0)
	{
		perror("open lcd fail");
		free(lcd);
		return NULL;
	}

	//映射
	lcd->mp = mmap(NULL,800*480*4,PROT_READ|PROT_WRITE,MAP_SHARED,lcd->fd,0);

	return lcd;
}
// // void display_text();
// int main()
// {	

// 	op_dve();
	
// 	printf("------------1\n");
//     //初始化Lcd
// 	  lcd = init_lcd("/dev/fb0");  // 全局变量初始化
// 	if (lcd == NULL) {
// 		return -1;
// 	}
// 	printf("------------2\n");	
// 	display_text();
// return 0;
// }



void* weather_time_thread(void* arg) //天气与时间线程
{
    unsigned int *local_lcd_mp = (unsigned int *)arg;
    if (local_lcd_mp == NULL)
        local_lcd_mp = lcd_mp;  // 回退到全局变量

    // 加载字体 (hei.TTF 支持中文，优先尝试)
    font *f = NULL;
    char font_path[256] = {0};

    // 1. 项目自带中文字体 hei.TTF
    resolve_asset_path(font_path, sizeof(font_path), "hei.TTF");
    f = fontLoad(font_path);
    if (f == NULL)
    {
        // 2. Data 目录下的 DroidSansFallback
        resolve_asset_path(font_path, sizeof(font_path), "DroidSansFallback.ttf");
        f = fontLoad(font_path);
    }
    if (f == NULL)
    {
        // 3. 系统标准路径
        f = fontLoad("/usr/share/fonts/DroidSansFallback.ttf");
    }
    if (f == NULL)
    {
        // 4. 系统备选路径
        f = fontLoad("/usr/share/fonts/truetype/DroidSansFallback.ttf");
    }
    if (f == NULL)
    {
        fprintf(stderr, "weather_time_thread: 所有字体路径均加载失败\n");
        return NULL;
    }
    printf("font ptr: %p\n", f);
    fontSetSize(f, 20);

    // 构造天气结构体
    weather1 wea;
    memset(&wea, 0, sizeof(wea));

    while (program_running)
    {
        // 获取天气数据
        if (Get_weather(&wea) != 0)
        {
            fprintf(stderr, "weather_time_thread: 获取天气失败\n");
        }

        // 获取时间
        char *time_str = show_time();

        // 更新全局面板数据 (在 g_panel_mutex 保护下)
        pthread_mutex_lock(&g_panel_mutex);
        if (time_str)
            strncpy(g_panel_data.time_str, time_str, sizeof(g_panel_data.time_str) - 1);
        strncpy(g_panel_data.temperature, wea.Temperature,       sizeof(g_panel_data.temperature) - 1);
        strncpy(g_panel_data.humidity,    wea.Relative_Humidity, sizeof(g_panel_data.humidity) - 1);
        strncpy(g_panel_data.wind,        wea.Wind,              sizeof(g_panel_data.wind) - 1);
        strncpy(g_panel_data.weather,     wea.Weather,           sizeof(g_panel_data.weather) - 1);
        pthread_mutex_unlock(&g_panel_mutex);

        // 渲染右侧面板 (在 lcd_mutex 保护下)
        pthread_mutex_lock(&lcd_mutex);
        right_panel_draw_all(local_lcd_mp, f);
        pthread_mutex_unlock(&lcd_mutex);

        sleep(1);
    }

    fontUnload(f);
    return NULL;
}



// void* weather_time_thread(void* arg)// 天气和时间线程
// {	
// 	 // 将void指针转换为unsigned int指针
//     unsigned int *lcd_mp = (unsigned int*)arg;
// //打开字体	
// 	font *f = fontLoad("/usr/share/fonts/DroidSansFallback.ttf");

// 	printf("%p\n",f);

// 	//字体大小的设置
// 	fontSetSize(f,20);


// 	bitmap *bm;
// 	char my_pic_pictr_path[256] ={0}; //用于存储转换好的图像路径

// 	// 构造天气
// 	weather1 wea;
	

// 	 while(program_running)
// 	{
	
// 	 pthread_mutex_lock(&lcd_mutex);
// 	// sprintf(my_pic_pictr_path,"/tmp/Object_Yunx_Driving_Recorder2/Data/runV1.bmp");//图片路径
												
//     	// Bmp_Decode(my_pic_pictr_path,lcd->mp);									//转换图片像素,显示一张主界面图片	

// 		bitmap *bm = createBitmapWithInit(150,20,4,getColor(50,255,255,0)); //构造时间输入框,也可使用createBitmapWithInit函数，改变画板颜色
// 		bitmap *bw = createBitmapWithInit(30,20,4,getColor(50,255,255,0)); //构造天气输入框,

// 		// char *str = show_time();   											//获取时间
// 		// char * Temtur=weg_p->Temperature;
	
// 		// fontPrint(f,bw,0,0,Temtur,getColor(0,28,106,224),0);		//显示天气到输入框
// 		// fontPrint(f,bm,0,0,str,getColor(0,28,106,224),0);	
// 		//显示时间到输入框
// 		show_font_to_lcd(lcd_mp,150,80,bm);

// 		show_font_to_lcd(lcd_mp,150,100,bw);
	
// 		printf("-------------7\n");  
// 		destroyBitmap(bm);                                      //刷新缓存
// 		destroyBitmap(bw);
		
// 		// 释放LCD互斥锁
//         pthread_mutex_unlock(&lcd_mutex);
	
// 		sleep(1);
	
// 	}

// 	fontUnload(f);
// //编译test.c testtime.c op_Boot_anim.c get_weathe2.c  -o main_ok   -I ./ -L./Library -lfont  -lm 
// 	return NULL;
// }