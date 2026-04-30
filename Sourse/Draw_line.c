#include "camera.h"

void Draw_line(void)
 {
    int x, y, i = 0;
    // 倒车辅助线绘制在摄像头区域 (列30~669, 640px宽)
    for(y = 479; y >= 240; y--)
    {
        // 左侧窄标线 (39px)
        for(x = 30 + i; x < 69 + i; x++)
        {
            if(y > 410)
            {
                *(lcd_mp + 800 * y + x) = 0x0000FF; 
            }
            if(410 >= y && y > 320)
            {
                *(lcd_mp + 800 * y + x) = 0x00FF00; 
            }
            if(320 >= y && y >= 240)
            {
                *(lcd_mp + 800 * y + x) = 0xFF0000; 
            }
        }
        // 右侧窄标线 (30px), 位于摄像头右边缘 669
        for(x = 669 - i; x > 639 - i; x--)
        {
            if(y > 410)
            {
                *(lcd_mp + 800 * y + x) = 0x0000FF; 
            }
            if(410 >= y && y > 320)
            {
                *(lcd_mp + 800 * y + x) = 0x00FF00; 
            }
            if(320 >= y && y > 240)
            {
                *(lcd_mp + 800 * y + x) = 0xFF0000; 
            }
        }
        // 左侧宽标线 (80px)
        for(x = 30 + i; x < 110 + i; x++)
        {
            if(410 >= y && y >= 380)
            {
                *(lcd_mp + 800 * y + x) = 0x0000FF; 
            }
            if(350 >= y && y > 320)
            {
                *(lcd_mp + 800 * y + x) = 0x00FF00; 
            }
            if(260 >= y && y >= 240)
            {
                *(lcd_mp + 800 * y + x) = 0xFF0000; 
            }
        }    
        // 右侧宽标线 (80px), 位于摄像头右边缘 669
        for(x = 669 - i; x > 589 - i; x--)    
        {
            if(410 >= y && y >= 380)
            {
                *(lcd_mp + 800 * y + x) = 0x0000FF; 
            }
            if(350 >= y && y > 320)
            {
                *(lcd_mp + 800 * y + x) = 0x00FF00; 
            }
            if(260 >= y && y >= 240)
            {
                *(lcd_mp + 800 * y + x) = 0xFF0000; 
            }
        }
        i++;
    }
 }
