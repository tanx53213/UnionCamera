#include "camera.h"
#include <pthread.h>

// 条件选择读取当前文件夹下的数据读取
static void resolve_asset_path(char *out, size_t out_size, const char *relative_path)
{
    char local_obj_path[256] = {0};
    char local_data_path[256] = {0};
    char fallback_path[256] = {0};

    snprintf(local_obj_path, sizeof(local_obj_path), "./Object_Yunx_Driving_Recorder2/Data/%s", relative_path);
    snprintf(local_data_path, sizeof(local_data_path), "./Data/%s", relative_path);
    snprintf(fallback_path, sizeof(fallback_path), "/tmp/Object_Yunx_Driving_Recorder2/Data/%s", relative_path);

    if (access(local_obj_path, R_OK) == 0)
        snprintf(out, out_size, "%s", local_obj_path);
    else if (access(local_data_path, R_OK) == 0)
        snprintf(out, out_size, "%s", local_data_path);
    else
        snprintf(out, out_size, "%s", fallback_path);
}

pthread_mutex_t lcd_mutex = PTHREAD_MUTEX_INITIALIZER;
volatile bool program_running = true;
ProgramState current_state = STATE_MAIN;

struct ts_pix *OnTimets;

Button buttons[6] =
{
    {2,   480, 150, 50, "后视"},
    {220, 480, 150, 50, "录像"},
    {400, 480, 150, 50, "历史"},
    {600, 480, 150, 50, "返回"},
    {800, 480, 150, 50, "守护进程"},
    {950, 510,  72, 71, "电源"}
};
int buttonCount = sizeof(buttons) / sizeof(Button);

bool isPointInButton(int x, int y, Button *btn)
{
    return (x >= btn->x && x <= (btn->x + btn->width) &&
            y >= btn->y && y <= (btn->y + btn->height));
}

int lcd_fd;
unsigned int *lcd_mp;
int ts_x, ts_y;
struct input_event ts_event;
char gif_path[256]       = {0};
char main_pictr_path[256] = {0};
int ts_fd;

/****************************************************************************************/

void *touch_thread(void *arg)
{
    while (program_running)
    {
        read(ts_fd, &ts_event, sizeof(ts_event));

        switch (ts_event.type)
        {
            case EV_ABS:
                switch (ts_event.code)
                {
                    case ABS_X:
                        ts_x = ts_event.value;
                        break;  // ← 修复：加上 break，防止 fall-through 覆盖 ts_x
                    case ABS_Y:
                        ts_y = ts_event.value;
                        break;
                }
                break;

            case EV_KEY:
                if (ts_event.code == BTN_TOUCH)
                {
                    if (ts_event.value == 1)  // 触摸按下
                    {
                        printf("x = %d\t,y = %d\n", ts_x, ts_y);
                        printf("--------------1\n");

                        OnTimets->pix_x    = ts_x;
                        OnTimets->pix_y    = ts_y;
                        OnTimets->isPressed = true;

                        // ← 修复：加锁保护共享的 LCD 资源，处理完再解锁
                        pthread_mutex_lock(&lcd_mutex);
                        handleTouchEvent(OnTimets, buttons, buttonCount);
                        pthread_mutex_unlock(&lcd_mutex);

                        ts_x = 0;
                        ts_y = 0;
                    }
                    else if (ts_event.value == 0)  // 触摸释放
                    {
                        printf("松开\n");
                        OnTimets->isPressed = false;
                    }
                }
                break;
        }

        usleep(10000);
    }

    return NULL;
}

int main()
{
    // 0. 初始化 LCD
    lcd_fd = open(LCD_DEV, O_RDWR);
    if (-1 == lcd_fd)
    {
        fprintf(stderr, "open lcd error, errno = %d, %s\n", errno, strerror(errno));
        exit(-1);
    }

    lcd_mp = (unsigned int *)mmap(
        NULL,
        LCD_MAPSIZE,
        PROT_READ | PROT_WRITE,
        MAP_SHARED,
        lcd_fd,
        0
    );
    if (lcd_mp == MAP_FAILED)
    {
        fprintf(stderr, "mmap for lcd error, errno = %d, %s\n", errno, strerror(errno));
        exit(-1);
    }


    // 1. 打开触摸屏
    ts_fd = open(TS_DEV, O_RDWR);
    if (-1 == ts_fd)
    {
        fprintf(stderr, "open touch screen error, errno = %d, %s\n", errno, strerror(errno));
        exit(-1);
    }


    // 2. 为触摸像素结构分配内存
    OnTimets = malloc(sizeof(struct ts_pix));
    if (OnTimets == NULL)
    {
        fprintf(stderr, "内存分配失败\n");
        exit(-1);
    }
    memset(OnTimets, 0, sizeof(struct ts_pix));


    // 3. 播放开机动画
    for (size_t i = 1; i <= GIF_NUM; i++)
    {
        char rel_path[128] = {0};
        snprintf(rel_path, sizeof(rel_path), "RunV_OpBoot_Anim/runv%zu.bmp", i);
        resolve_asset_path(gif_path, sizeof(gif_path), rel_path);
        printf("boot anim path: %s\n", gif_path);
        Bmp_Decode(gif_path, lcd_mp);
        usleep(20 * 1000);
    }
    printf("--------------main1\n");

    int camera_fd  = -1;
    unsigned int n_buffers = 0;
    struct buffer *buffers  = NULL;


    // 4. 显示主界面
    resolve_asset_path(main_pictr_path, sizeof(main_pictr_path), "runV1.bmp");
    printf("main ui path: %s\n", main_pictr_path);
    Bmp_Decode(main_pictr_path, lcd_mp);
    printf("--------------main2\n");


   // 5. 创建线程
pthread_t weather_tid, touch_tid;
printf("--------------main3\n");

if (pthread_create(&weather_tid, NULL, weather_time_thread, (void*)lcd_mp) != 0)
{
    printf("创建天气线程失败\n");
    return -1;
}

if (pthread_create(&touch_tid, NULL, touch_thread, NULL) != 0)
{
    printf("创建触摸线程失败\n");
    return -1;
}
printf("--------------main4\n");

pthread_join(weather_tid, NULL);
printf("--------------main5\n");
pthread_join(touch_tid, NULL);
printf("--------------main6\n");



    // 6. 清理资源
    pthread_mutex_destroy(&lcd_mutex);
    free(OnTimets);
    munmap(lcd_mp, LCD_MAPSIZE);
    close(lcd_fd);
    close(ts_fd);

    return 0;
}
























// #include "camera.h"
// #include <pthread.h>

// //条件选择读取当前文件夹下的数据读取,  等下可以删了,这个我已经知道错在哪里了
// static void rsoleve_asset_path(char *out, size_t out_size, const char *relative_path)
// {
//     char local_obj_path[256] = {0};
//     char local_data_path[256] = {0};
//     char fallback_path[256] = {0};

//     snprintf(local_obj_path, sizeof(local_obj_path), "./Object_Yunx_Driving_Recorder2/Data/%s", relative_path);
//     snprintf(local_data_path, sizeof(local_data_path), "./Data/%s", relative_path);
//     snprintf(fallback_path, sizeof(fallback_path), "/tmp/Object_Yunx_Driving_Recorder2/Data/%s", relative_path);

//     if (access(local_obj_path, R_OK) == 0)
//     {
//         snprintf(out, out_size, "%s", local_obj_path);
//     }
//     else if (access(local_data_path, R_OK) == 0)
//     {
//         snprintf(out, out_size, "%s", local_data_path);
//     }
//     else
//     {
//         snprintf(out, out_size, "%s", fallback_path);
//     }
// }

// pthread_mutex_t lcd_mutex = PTHREAD_MUTEX_INITIALIZER;  // LCD显示互斥锁，用于防止多线程同时访问显示设备
// volatile bool program_running = true;  // 程序运行状态标志，volatile确保多线程能即时看到变量更新
// ProgramState current_state = STATE_MAIN;

//   // 定义按钮
// struct ts_pix *OnTimets;  // 只在全局声明

//    Button buttons[6] = 
//     {
//         {2, 480, 150, 50, "后视"},
//         {220, 480, 150, 50, "录像"},
//         {400, 480, 150, 50, "历史"},
//         {600, 480, 150, 50, "返回"},
//         {800, 480, 150, 50, "守护进程"},
//         {950, 510, 72, 71, "电源"}
//     };
// int buttonCount = sizeof(buttons) / sizeof(Button);//按键计数

// // 检测点是否在按钮区域内
// bool isPointInButton(int x, int y, Button* btn) {
//     return (x >= btn->x && x <= (btn->x + btn->width) &&
//             y >= btn->y && y <= (btn->y + btn->height));
// }

//     int lcd_fd;
//     unsigned int *lcd_mp;
//     int ts_x,ts_y;
//     struct input_event ts_event;
//     char gif_path[256] ={0}; //用于存储转换好的图像路径
//     char main_pictr_path[256] ={0}; //用于存储转换好的图像路径
//     int ts_fd;

// /****************************************************************************************/
 
//  void* touch_thread(void* arg)//事件触摸逻辑
//     {
                    
//                         while(program_running)
//                     {
                        
//                         // 从触摸屏的设备文件中读取输入设备的类的属性并存储到类相关的变量中
//                         read(ts_fd, &ts_event, sizeof(ts_event)); // read函数默认的属性是带有阻塞属性

//                         // 检查事件类型
//                         switch(ts_event.type) // 使用switch语句处理事件类型
//                         {
//                             case EV_ABS: // 处理绝对坐标事件
//                                 switch(ts_event.code) // 进一步判断坐标轴
//                                 {
//                                     case ABS_X: // 说明是触摸屏的X轴
//                                         ts_x = ts_event.value;
//                                         // break;
//                                     case ABS_Y: // 说明是触摸屏的Y轴
//                                         ts_y = ts_event.value;
//                                         // break;
//                                 }
//                                 break;

//                             case EV_KEY: // 处理键盘事件和触摸状态
//                                 if(ts_event.code == BTN_TOUCH) // 检查触摸状态
//                                 {
//                                     if(ts_event.value == 1) // 1表示触摸按下
//                                     {
//                                         // printf("按下\n", ts_event.code);// 表示按键按下
//                                         // 只有在坐标值被更新后才输出
//                                         printf("x = %d\t,y = %d\n", ts_x, ts_y);
//                                         printf("--------------1\n");
//                                         OnTimets->pix_x=ts_x;
//                                         OnTimets->pix_y=ts_y;
//                                         OnTimets->isPressed = true;
//                                         handleTouchEvent(OnTimets, buttons, buttonCount);

//                                         pthread_mutex_unlock(&lcd_mutex);  // 释放LCD互斥锁
//                                         // 处理完后清零，避免重复触发
//                                 ts_x = 0;
//                                 ts_y = 0;
//                                 break;
                                
//                                     }

//                                     else if(ts_event.value == 0) // 0表示按键释放
//                                     {
//                                         printf("松开\n", ts_event.code);
//                                         OnTimets->isPressed = false;
//                                     }
//                                 }
                            
//                                 break;//结束switch语句
//                         }

                        
//                         // 4.输出值处理
//                         usleep(10000);  // 短暂休眠，避免CPU占用过高
//                         //  printf("Event: type=%d, code=%d, value=%d\n",ts_event.type, ts_event.code, ts_event.value);
                
//                     }

//                 //编译TouchScreen4.c op_Boot_anim.c main.c
//                         return NULL;
                
//     }

// int main()
// {

   


//     // op_dve();//打开屏幕LCD_DEV文件
//    //0.初始化LCD(打开LCD+MMAP内存映射)
//     lcd_fd = open(LCD_DEV,O_RDWR);

//     //错误处理
//     if(-1 == lcd_fd)
//     {
//         fprintf(stderr,"open lcd error,errno = %d,%s\n",errno,strerror(errno));
//         exit(-1);
//     }

//     lcd_mp = (unsigned int *)mmap(
//                                     NULL        ,               //由系统内核选择合适的位置
//                                     LCD_MAPSIZE ,               //指的是要映射的内存的大小
//                                     PROT_READ | PROT_WRITE,     //以可读可写的方式访问
//                                     MAP_SHARED,                 //选择共享映射空间
//                                     lcd_fd,                     //指的是待映射内存的文件描述符
//                                     0                           //不偏移内存
//                                 );    
//     //错误处理
//     if(lcd_mp == MAP_FAILED)
//     {
//         fprintf(stderr,"mmap for lcd error,errno = %d,%s\n",errno,strerror(errno));
//         exit(-1);
//     }

    
  
//     ts_fd = open(TS_DEV,O_RDWR);//打开触摸屏TS_DEV文件
 
//   //错误处理
//         if(-1 == ts_fd)
//     {
//         fprintf(stderr,"open touch screen error,errno = %d,%s\n",errno,strerror(errno));
//         exit(-1);
//     }



//     /****************************************************************************************/
//     // 为实时读取像素分配内存分配
//     OnTimets = malloc(sizeof(struct ts_pix));
//     if (OnTimets == NULL)
//     {
//         fprintf(stderr, "内存分配失败\n");
//         exit(-1);
//     }
//     bzero(OnTimets,(sizeof(struct ts_pix)));


//   //在LCD显示开机动画,用户需要自己修改路径
//    for (size_t i = 1; i <= GIF_NUM; i++)
//     {
//         char rel_path[128] = {0};
//         snprintf(rel_path, sizeof(rel_path), "RunV_OpBoot_Anim/runv%zu.bmp", i);
//         resolve_asset_path(gif_path, sizeof(gif_path), rel_path);
//         printf("boot anim path: %s\n", gif_path);

//         Bmp_Decode(gif_path,lcd_mp);                    //开机动画图像显示
       
//         usleep(20*1000);                             //FPS=50HZ
//     }
// printf("--------------main1\n");


//     //用于记录摄像头的设备文件的文件描述符
//     int camera_fd = -1;

//     unsigned int n_buffers= 0; //用于记录缓冲区的编号


//     struct buffer *buffers = NULL; //用于记录申请的缓冲区信息

//     /****************************************************************************************/

//     //展示主界面
//     resolve_asset_path(main_pictr_path, sizeof(main_pictr_path), "runV1.bmp");
//     printf("main ui path: %s\n", main_pictr_path);

//     Bmp_Decode(main_pictr_path,lcd_mp);                      //主界面图像显示
//  printf("--------------main2\n");
//     // tch_Scrn();
//        // 创建线程
//     pthread_t weather_tid, touch_tid;
//    printf("--------------main3\n");
//   // 创建时间天气显示线程
//     if(pthread_create(&weather_tid, NULL, weather_time_thread, NULL) != 0) {
//         printf("创建天气线程失败\n");
//         return -1;

//     }

//     // 创建触摸检测线程
//     if(pthread_create(&touch_tid, NULL, touch_thread, NULL) != 0) {
//         printf("创建触摸线程失败\n");
//         return -1;
//     }
//        printf("--------------main4\n");
//     // 等待线程结束
//     pthread_join(weather_tid, NULL);
//     printf("--------------main5\n");
//     pthread_join(touch_tid, NULL);
//     printf("--------------main6\n");
//     // 清理资源
//     pthread_mutex_destroy(&lcd_mutex);

//     return 0;
// }
