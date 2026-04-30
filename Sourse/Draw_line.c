#include "camera.h"

// 倒车辅助线 — 摄像头左对齐于列 0-639，辅助线在其边缘绘制
void Draw_line(void)
 {
    int x, y, i = 0;
    for(y = 479; y >= 240; y--)
    {
        // 左侧窄标线 (39px wide, at camera left edge: column 0)
        for(x = i; x < 39 + i; x++)
        {
            if(y > 410)
                *(lcd_mp + 800 * y + x) = 0x0000FF; 
            if(410 >= y && y > 320)
                *(lcd_mp + 800 * y + x) = 0x00FF00; 
            if(320 >= y && y >= 240)
                *(lcd_mp + 800 * y + x) = 0xFF0000; 
        }
        // 右侧窄标线 (30px wide, at camera right edge: column 639)
        for(x = 639 - i; x > 609 - i; x--)
        {
            if(y > 410)
                *(lcd_mp + 800 * y + x) = 0x0000FF; 
            if(410 >= y && y > 320)
                *(lcd_mp + 800 * y + x) = 0x00FF00; 
            if(320 >= y && y >= 240)
                *(lcd_mp + 800 * y + x) = 0xFF0000; 
        }
        // 左侧宽标线 (80px wide)
        for(x = i; x < 80 + i; x++)
        {
            if(410 >= y && y >= 380)
                *(lcd_mp + 800 * y + x) = 0x0000FF; 
            if(350 >= y && y > 320)
                *(lcd_mp + 800 * y + x) = 0x00FF00; 
            if(260 >= y && y >= 240)
                *(lcd_mp + 800 * y + x) = 0xFF0000; 
        }    
        // 右侧宽标线 (80px wide)
        for(x = 639 - i; x > 559 - i; x--)    
        {
            if(410 >= y && y >= 380)
                *(lcd_mp + 800 * y + x) = 0x0000FF; 
            if(350 >= y && y > 320)
                *(lcd_mp + 800 * y + x) = 0x00FF00; 
            if(260 >= y && y >= 240)
                *(lcd_mp + 800 * y + x) = 0xFF0000; 
        }
        i++;
    }
 }
