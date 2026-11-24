#include "stm32f4xx.h"
#include "board1.h"
#include "board.h"
#include "Key.h"
#include "OLED.h"
#include "k230.h"
#include "function.h"
#include "Emm_V5.h"
#include "Emm_V5_2.h"
#include "Timer1.h"
#include "led.h"
#include "Buzzer.h"
extern uint8_t pid_flag;

int main(void)
{
    uint8_t key;

    board_init();
    Key_Init();
    OLED_Init();
    K230_Init();
    LED_Init();
    Timer1_Init();
    Buzzer_Init();
	
    Func_Reset_Init();
    Func_LeftTop_Init();
    Func_Edge_Init(); 
    Func_Track_Init();	
	Func_Circle_Init(); 
	Func_LaserCircle_Init();
	
    Emm_V5_En_Control(1, true, false);
    Emm_V5_En_Control_2(1, true, false);
    delay_ms(500);

  while (1)
{
    K230_Process();

	
    key = Key_GetNum();

    if (key == 1)        Func_Reset();       /* PE3 复位 */
    else if (key == 2)   Func_Stop();        /* PE4 停止 */
    else if (key == 3)   Func_SelectNext();  /* PE7 选功能 */
    else if (key == 4)   Func_Execute();     /* PE8 确认执行 */

    if (pid_flag)
    {
        pid_flag = 0;

        if (reset_running == 1)
            Func_Reset_Task();
        if (left_top_running == 1)
            Func_LeftTop_Task();
		if (edge_running==1)    
		    Func_Edge_Task();    // 边框任务 
		if (track_running==1)   
			Func_Track_Task(); //追踪系统
		if (circle_running==1)
			 Func_Circle_Task();//中心画圆 
        if (laser_circle_running) 
			Func_LaserCircle_Task();//激光画圆
    }
}
}