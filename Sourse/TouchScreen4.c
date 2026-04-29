// /*******************************************************************************************
//  *      @name   TouchScreen.c
//  *      @brief  这是系统IO的关于文件访问接口实现对linux系统下的触摸屏设备的访问，用于获取坐标值
//  *      @date   2025/01/10 
//  *      @author cecelmx@126.com 
//  *      @note
//  *          1. ARM开发板搭载的是linux系统，而linux系统下有关于输入设备的子系统(输入子系统)
//  *          2. 输入子系统提供了关于触摸屏的设备文件 设备文件的路径"/dev/input/event0" 
//  *          3. ARM开发板搭载的触摸屏属于电容式触摸屏，触摸屏的坐标系分为两类
//  *          4. 蓝色边框的触摸屏，坐标范围是(800,480 ),所以和LCD屏的分辨率一致，所以不需要转换
//  *          5. 黑色边框的触摸屏，坐标范围是(1024,600),所以和LCD屏的分辨率不同，所以需要转换！！
//  *          6. linux系统的输入设备采用的类一般都是定义在input.h头文件中  路径 <linux/input.h>
//  * 
//  *      @version  xx.xx.xx   主版本号.次版本号.修正版本号
//  * 
//  *      CopyRight (c)   2024-2025   Your Name     All Right Reserved
//  * 
//  * ****************************************************************************************/
// #include <stdio.h>
// #include <errno.h>
// #include <string.h>
// #include <stdlib.h>
// #include <sys/types.h>
// #include <sys/stat.h>
// #include <fcntl.h>
// #include <unistd.h>
// #include <sys/mman.h>
// #include <linux/input.h>
// #include"camera.h"
// //为了提高程序的可移植性，触摸屏设备文件的路径通过宏定义实现
// #define  TS_DEV   "/dev/input/event0"






// extern int buttonCount;


// // 修改handleButtonPress函数，添加新功能
// void handleButtonPress(Button* button, int buttonIndex) {
//     if (button == NULL || buttonIndex < 0 || buttonIndex >= buttonCount) 
//     {
//         printf("无效的按钮索引\n");
//         return;
//     }
//    // 电源键特殊处理 (buttonIndex == 5)

//      switch(buttonIndex)
//     {
//         case 0:  // 后视
//             if (current_state == STATE_MAIN) 
//             {
//                 printf("进入后视功能\n");
//                 current_state = STATE_BACKUP;
//                 open_device();
//                 init_device();
//                 start_capturing();
//                 mainloop();
//             }
//             break;
            
//         case 1:  // 录像
//             if (current_state == STATE_MAIN) 
//             {
//                 printf("进入录像功能\n");
//                 current_state = STATE_RECORD;
//                  open_device();
//                 init_device();
//                 start_capturing();
//                 mainloop3();
//             }
//             break;
            
//         case 2:  // 历史
//             if (current_state == STATE_MAIN)
//              {
//                 printf("进入历史记录\n");
//                 current_state = STATE_HISTORY;
//                 show_history();
//             }
//             break;
            
//         case 3:  // 返回
//             if (current_state != STATE_MAIN)
//              {
//                 return_to_main();
//             }
//             break;
            
//         case 4:  // 守护进程
//             if (current_state == STATE_MAIN)
//              {
//                 printf("进入守护进程\n");
//                 current_state = STATE_GUARD;
//                 circular_recording();
//             }
//             break;
//             case5: 
//              {
//                     if (current_state == STATE_MAIN) 
//                     {
//                         // 在主界面按电源键退出程序
//                         printf("程序退出\n");
//                         stop_camera();  // 确保清理摄像头
//                         exit(0);
//                     } 
//                     else 
//                     {
//                         // 在其他界面按电源键返回主界面
//                         return_to_main();
//                         return;
//                     }   
                   
//              }
//     }
// }
// // 处理触摸事件
// void handleTouchEvent(ts_pix* event, Button* buttons, int buttonCount)
//  {
//     if (!event->isPressed) {
//         return;  // 未检测到按压，直接返回
//     }
    
//     // // 遍历所有按钮检查是否被点击
//     for (int i = 0; i < buttonCount; i++) {
//         if (isPointInButton(event->pix_x, event->pix_y, &buttons[i]))
//          {
          
//             // 按钮的具体处理逻辑
//             handleButtonPress(&buttons[i],i);

//             break;
//         }
    

//      }
// }


// // 处理触摸事件
// void handleTouchEvent(ts_pix* event, Button* buttons, int buttonCount)
//  {
//     if (!event->isPressed) {
//         return;  // 未检测到按压，直接返回
//     }
    
//     // // 遍历所有按钮检查是否被点击
//     for (int i = 0; i < buttonCount; i++) {
//         if (isPointInButton(event->pix_x, event->pix_y, &buttons[i]))
//          {
          
//             // 按钮的具体处理逻辑
//             handleButtonPress(&buttons[i],i);

//             break;
//         }
    

//      }
// }





// // 按钮点击处理函数
// void handleButtonPress(Button* button, int buttonIndex) //将i作为常量索引
// {
//     if (button == NULL || buttonIndex < 0 || buttonIndex >= buttonCount) {
//         printf("无效的按钮索引\n");
//         return;
//     }

//     // printf("按下%s\n", button->text);
    
//     // 根据按钮索引执行不同操作
//     switch(buttonIndex) {
//         case 0:  // 后视
    
//                                             //1.打开摄像头
//                                             open_device();

//                                             //2.初始化设备
//                                             init_device();

//                                             //3.开始捕获
//                                             start_capturing();

//                                             //4.开始转换
//                                             mainloop();
//             printf("按下后视按钮\n");
//             break;
//         case 1:  // 录像
//             printf("按下录像按钮\n");
//             break;
//         case 2:  // 历史
//             printf("按下历史按钮\n");
//             break;
//         case 3:  // 返回
//             printf("按下返回按钮\n");
//             break;
//         case 4:  // 守护进程
//             printf("按下守护进程按钮\n");
//             break;
//         case 5:  // 电源
//             printf("按下电源按钮\n");
//             break;
//         default:
//             printf("未知按钮\n");
//             break;
//     }
// }






// struct ts_pix *OnTimets = NULL;  // 只在全局声明

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