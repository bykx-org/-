#include "function.h"
#include "board.h"
#include "OLED.h"
#include "k230.h"
#include "PID.h"
#include "Emm_V5.h"
#include "Emm_V5_2.h"
#include "math.h"
#include "Timer1.h"
#include "Buzzer.h"
#include "led.h"
#include "math.h"
#include <stdlib.h>
static uint8_t func_index = 0;

//第一点中心像素
#define CENTER_PX   336.0f
#define CENTER_PY   246.0f

static uint8_t x_aligned = 0;   /* X 是否已对齐 */

static PID_t pid_x;
static PID_t pid_y;

uint8_t reset_running = 0;

//左上角任务
/* ==================== 左上角像素 ==================== */
#define LEFT_TOP_PX   140.0f
#define LEFT_TOP_PY   52.0f

/* ==================== 左上角独立 PID ==================== */
static PID_t pid_x_lt;
static PID_t pid_y_lt;

uint8_t left_top_running = 0;

//第二点边线要求
#define ARRIVE_PX       6.0f    /* 到点误差，约 1cm */
#define SEG_STEPS       8       

/* ==================== 边框 PID ==================== */
static PID_t pid_x_edge;
static PID_t pid_y_edge;

uint8_t edge_running = 0;

static uint8_t edge_seg_from = 0;
static uint8_t edge_seg_step = 0;
static float   edge_targetX  = 0;
static float   edge_targetY  = 0;

//第三点追踪系统
/* ==================== 追踪参数 ==================== */
#define TRACK_TOLERANCE_PX   3.0f     /* 追上误差，像素 */
#define TRACK_SPEED_MAX      800      /* 速度上限 RPM */
#define TRACK_STABLE_MS      1000    /* 持续稳定 1 秒 */

/* ==================== 追踪 PID ==================== */
static PID_t pid_x_track;
static PID_t pid_y_track;

uint8_t track_running = 0;

static uint8_t track_alarm_done = 0;   /* 声光提醒已触发 */

//中心画圆
/* ==================== 画圆参数 ==================== */
#define CIRCLE_CX      336.0f    /* 圆心 X */
#define CIRCLE_CY      246.0f    /* 圆心 Y */
#define CIRCLE_RX      50.0f     /* X 半径，10cm */
#define CIRCLE_RY      66.0f     /* Y 半径，10cm */
#define CIRCLE_POINTS  36       /* 圆上点个数，加密 */
#define CIRCLE_ARRIVE  4.0f      /* 到点误差，像素 */

/* ==================== 画圆 PID ==================== */
static PID_t pid_x_circle;
static PID_t pid_y_circle;

uint8_t circle_running = 0;

static uint16_t circle_index   = 0;
static float    circle_targetX = 0;
static float    circle_targetY = 0;

//激光笔为中心画圆
/* ==================== 激光笔画圆参数 ==================== */
#define LASER_CIRCLE_RX      50.0f
#define LASER_CIRCLE_RY      66.0f
#define LASER_CIRCLE_POINTS  36
#define LASER_CIRCLE_ARRIVE  8.0f
#define LASER_HOLD_MS        1500
#define LASER_RANGE_PX       5

/* ==================== 激光笔画圆变量 ==================== */
static PID_t pid_x_lc;
static PID_t pid_y_lc;

uint8_t  laser_circle_running = 0;
static uint8_t  laser_circle_phase  = 0;
static uint16_t laser_circle_index  = 0;
static float    laser_circle_targetX = 0;
static float    laser_circle_targetY = 0;
static float    laser_cx = 0;
static float    laser_cy = 0;

//第一点
void Func_Reset_Init(void)
{
    PID_Init(&pid_x, 0.65f, 0.0f,1.0f, 500.0f);
    PID_Init(&pid_y, 0.6f, 0.0f, 0.5f, 500.0f);

    PID_SetTarget(&pid_x, CENTER_PX);
    PID_SetTarget(&pid_y, CENTER_PY);
}

void Func_Reset(void)
{
    x_aligned = 0;              /* 复位时清标志 */
    reset_running = 1;
    OLED_ShowString(2, 1, "Reseting...");
}


void Func_Reset_Task(void)
{
    static uint32_t stable_start = 0;
    static uint8_t  stable_flag  = 0;

    if (!reset_running) return;

    uint16_t bx, by;

    if (!K230_GetBlackPos(&bx, &by)) return;

    float errorX = CENTER_PX - (float)bx;
    float errorY = CENTER_PY - (float)by;

    float vx = 0;
    float vy = 0;

    if (x_aligned && fabs(errorX) > 10.0f)
    {
        x_aligned = 0;
    }

    if (!x_aligned)
    {
        if (fabs(errorX) <= 3.0f)
        {
            x_aligned = 1;
        }
        else
        {
            vx = PID_Calc(&pid_x, (float)bx);
            vy = 0;
        }
    }
    else
    {
        if (fabs(errorX) <= 5.0f && fabs(errorY) <= 5.0f)
        {
            if (!stable_flag)
            {
                stable_flag  = 1;
                stable_start = GetTick();
            }
            else
            {
                if (GetTick() - stable_start >= 2000)
                {
                    reset_running = 0;
                    x_aligned = 0;
                    stable_flag = 0;
                    OLED_ShowString(2, 1, "Reset Done  ");
                    return;
                }
            }
        }
        else
        {
            stable_flag = 0;
        }

        vx = 0;
        vy = PID_Calc(&pid_y, (float)by);
    }

    /* ===== 方向已确认 ===== */
    int32_t clk1 = (int32_t)( vx + vy);
    int32_t clk2 = (int32_t)( vx - vy);

    uint8_t dir1, dir2;

    if (clk1 >= 0) { dir1 = 0; }
    else           { dir1 = 1; clk1 = -clk1; }

    if (clk2 >= 0) { dir2 = 0; }
    else           { dir2 = 1; clk2 = -clk2; }

    if (clk1 == 0) clk1 = 1;
    if (clk2 == 0) clk2 = 1;

    if (clk1 > 500) clk1 = 500;
    if (clk2 > 500) clk2 = 500;

    OLED_ShowSignedNum(3, 1, (int32_t)errorX, 4);
    OLED_ShowSignedNum(3, 8, (int32_t)errorY, 4);

    Emm_V5_Pos_Control(1, dir1, 50, 50, (uint32_t)clk1, 0, 0);
    Emm_V5_Pos_Control_2(1, dir2, 50, 50, (uint32_t)clk2, 0, 0);
}

//左上角

/* ==================== 初始化 ==================== */
void Func_LeftTop_Init(void)
{
    PID_Init(&pid_x_lt, 0.6f, 0.0f, 0.05f, 500.0f);
    PID_Init(&pid_y_lt, 0.6f, 0.0f, 0.12f, 500.0f);

    PID_SetTarget(&pid_x_lt, LEFT_TOP_PX);
    PID_SetTarget(&pid_y_lt, LEFT_TOP_PY);
}

/* ==================== 触发 ==================== */
void Func_LeftTop(void)
{
    x_aligned = 0;
    left_top_running = 1;
    OLED_ShowString(2, 1, "Go LeftTop...");
}


/* ==================== 任务 ==================== */
void Func_LeftTop_Task(void)
{
    static uint32_t stable_start = 0;
    static uint8_t  stable_flag  = 0;

    if (!left_top_running) return;

    uint16_t bx, by;

    if (!K230_GetBlackPos(&bx, &by)) return;

    float errorX = LEFT_TOP_PX - (float)bx;
    float errorY = LEFT_TOP_PY - (float)by;

    float vx = 0;
    float vy = 0;

    if (x_aligned && fabs(errorX) > 10.0f)
    {
        x_aligned = 0;
    }

    if (!x_aligned)
    {
        if (fabs(errorX) <= 5.0f)
        {
            x_aligned = 1;
        }
        else
        {
            vx = PID_Calc(&pid_x_lt, (float)bx);
            vy = 0;
        }
    }
    else
    {
        if (fabs(errorX) <= 5.0f && fabs(errorY) <= 5.0f)
        {
            if (!stable_flag)
            {
                stable_flag  = 1;
                stable_start = GetTick();
            }
            else
            {
                if (GetTick() - stable_start >= 2000)
                {
                    left_top_running = 0;
                    x_aligned = 0;
                    stable_flag = 0;
                    OLED_ShowString(2, 1, "LeftTop Done");
                    return;
                }
            }
        }
        else
        {
            stable_flag = 0;
        }

        vx = 0;
        vy = PID_Calc(&pid_y_lt, (float)by);
    }

    /* ===== 方向已确认 ===== */
    int32_t clk1 = (int32_t)( vx + vy);
    int32_t clk2 = (int32_t)( vx - vy);

    uint8_t dir1, dir2;

    if (clk1 >= 0) { dir1 = 0; }
    else           { dir1 = 1; clk1 = -clk1; }

    if (clk2 >= 0) { dir2 = 0; }
    else           { dir2 = 1; clk2 = -clk2; }

    if (clk1 == 0) clk1 = 1;
    if (clk2 == 0) clk2 = 1;

    if (clk1 > 500) clk1 = 500;
    if (clk2 > 500) clk2 = 500;

    OLED_ShowSignedNum(4, 1, (int32_t)errorX, 4);
    OLED_ShowSignedNum(4, 8, (int32_t)errorY, 4);

    Emm_V5_Pos_Control(1, dir1, 50, 50, (uint32_t)clk1, 0, 0);
    Emm_V5_Pos_Control_2(1, dir2, 50, 50, (uint32_t)clk2, 0, 0);
}
//第二点顺时针

/* ==================== 边框 17 个大点 ==================== */
float edge_path[17][2];


/* ==================== 生成边框路径 ==================== */
void Generate_Edge_Path(void)
{
    edge_path[0][0]  = 140.0f;  edge_path[0][1]  = 52.0f;    /* 左上角 */
    edge_path[1][0]  = 223.0f;  edge_path[1][1]  = 52.0f;
    edge_path[2][0]  = 336.0f;  edge_path[2][1]  = 52.0f;
    edge_path[3][0]  = 456.0f;  edge_path[3][1]  = 52.0f;
    edge_path[4][0]  = 556.0f;  edge_path[4][1]  = 52.0f;
    edge_path[5][0]  = 556.0f;  edge_path[5][1]  = 166.0f;
    edge_path[6][0]  = 556.0f;  edge_path[6][1]  = 243.0f;
    edge_path[7][0]  = 556.0f;  edge_path[7][1]  = 309.0f;   /* 右上角 */
    edge_path[8][0]  = 556.0f;  edge_path[8][1]  = 405.0f;
    edge_path[9][0]  = 456.0f;  edge_path[9][1]  = 405.0f;
    edge_path[10][0] = 336.0f;  edge_path[10][1] = 410.0f;
    edge_path[11][0] = 223.0f;  edge_path[11][1] = 415.0f;   /* 右下角 */
    edge_path[12][0] = 140.0f;  edge_path[12][1] = 415.0f;
    edge_path[13][0] = 140.0f;  edge_path[13][1] = 309.0f;
    edge_path[14][0] = 140.0f;  edge_path[14][1] = 243.0f;
    edge_path[15][0] = 140.0f;  edge_path[15][1] = 166.0f;   /* 左下角 */
    edge_path[16][0] = 140.0f;  edge_path[16][1] = 52.0f;    /* 回到左上角 */
}

/* ==================== 初始化 ==================== */


void Func_Edge_Init(void)
{
    PID_Init(&pid_x_edge, 6.0f, 0.0f, 0.5f, 3000.0f);
    PID_Init(&pid_y_edge, 3.0f, 0.0f, 0.5f, 3000.0f);
}

/* ==================== 段内插值 ==================== */
static void Edge_CalcSegTarget(uint8_t from, uint8_t to, uint8_t step)
{
    float x1 = edge_path[from][0];
    float y1 = edge_path[from][1];
    float x2 = edge_path[to][0];
    float y2 = edge_path[to][1];

    float t = (float)(step + 1) / (float)(SEG_STEPS + 1);


    edge_targetX = x1 + t * (x2 - x1);
    edge_targetY = y1 + t * (y2 - y1);

    PID_SetTarget(&pid_x_edge, edge_targetX);
    PID_SetTarget(&pid_y_edge, edge_targetY);
}

/* ==================== 触发 ==================== */
void Func_Edge(void)
{
    edge_seg_from = 0;
    edge_seg_step = 0;
    edge_running  = 1;

    Generate_Edge_Path();

    edge_targetX = edge_path[0][0];
    edge_targetY = edge_path[0][1];

    PID_SetTarget(&pid_x_edge, edge_targetX);
    PID_SetTarget(&pid_y_edge, edge_targetY);

    OLED_ShowString(2, 1, "Edge Start");
}

/* ==================== 任务（速度模式） ==================== */
void Func_Edge_Task(void)
{
    if (!edge_running) return;

    uint16_t bx, by;

    if (!K230_GetBlackPos(&bx, &by)) return;

    float errorX = edge_targetX - (float)bx;
    float errorY = edge_targetY - (float)by;
    float distance = sqrt(errorX*errorX + errorY*errorY);

    /* ==================== 到点，切下一个 ==================== */
    float arrive_x = ARRIVE_PX;

    if (edge_seg_from >= 12&&edge_seg_from >= 15)
    {
        arrive_x = 4.0f;
    }

    if (fabs(errorX) <= arrive_x && fabs(errorY) <= ARRIVE_PX)
    {
		
		    if (edge_seg_from == 4 || edge_seg_from == 8|| edge_seg_from == 12)
    {
        Emm_V5_Stop_Now(1, false);
        Emm_V5_Stop_Now_2(1, false);
    }
		
        edge_seg_step++;

        if (edge_seg_step > SEG_STEPS)
        {
            edge_seg_step = 0;
            edge_seg_from++;

            if (edge_seg_from >= 17)
            {
                edge_running = 0;
                Emm_V5_Stop_Now(1, false);
                Emm_V5_Stop_Now_2(1, false);
                OLED_ShowString(2, 1, "Edge Done   ");
                return;
            }

            edge_targetX = edge_path[edge_seg_from][0];
            edge_targetY = edge_path[edge_seg_from][1];

            PID_SetTarget(&pid_x_edge, edge_targetX);
            PID_SetTarget(&pid_y_edge, edge_targetY);
        }
        else
        {
            Edge_CalcSegTarget(edge_seg_from,
                               (edge_seg_from + 1) % 17,
                               edge_seg_step);
        }
        return;
    }
    /* ==================== PID ==================== */
    float vx = PID_Calc(&pid_x_edge, (float)bx);
    float vy = PID_Calc(&pid_y_edge, (float)by);

	if (edge_seg_from >= 8 && edge_seg_from <= 12)
  {
    vx *= 0.6f;
  }
  

	
        if (edge_seg_from < 3)
         {
            vx *= 0.75f;
            vy *= 0.3f;  
          }


    /* ==================== 合成速度 ==================== */
    int16_t vel1 = (int16_t)( vx + vy);
    int16_t vel2 = (int16_t)( vx - vy);


    uint8_t  dir1, dir2;
    uint16_t spd1, spd2;

    if (vel1 >= 0) { dir1 = 0; spd1 =  vel1; }
    else           { dir1 = 1; spd1 = -vel1; }

    if (vel2 >= 0) { dir2 = 0; spd2 =  vel2; }
    else           { dir2 = 1; spd2 = -vel2; }
	
	if (spd1 > 0 && spd1 <10) spd1 = 10;
    if (spd2 > 0 && spd2 < 10) spd2 = 10;

	if (spd1 > 0) Emm_V5_Vel_Control(1, dir1, spd1, 10, false);
    if (spd2 > 0) Emm_V5_Vel_Control_2(1, dir2, spd2, 10, false);
	
	OLED_ShowNum(4, 1, spd1, 4);
    OLED_ShowNum(4, 10, spd2, 4);
	
}



//第三点追踪系统
/* ==================== 初始化 ==================== */
void Func_Track_Init(void)
{
    PID_Init(&pid_x_track, 2.5f, 0.0f, 2.0f, 800.0f);
    PID_Init(&pid_y_track, 1.5f, 0.0f, 2.0f, 800.0f);
}

/* ==================== 触发：立刻开始追 ==================== */

void Func_Track(void)
{
    track_running    = 1;
    track_alarm_done = 0;

    OLED_Clear();          /* 清屏幕 */
    OLED_Clear_trace();    /* 清轨迹缓冲 */
    OLED_Refresh();        /* 刷屏，全黑 */
}

void Func_Track_Task(void)
{
    static uint32_t stable_start = 0;
    static uint8_t  stable_flag  = 0;

    static uint16_t last_lx = 336;
    static uint16_t last_ly = 246;
    static uint8_t  laser_was_valid = 0;
    static uint8_t  refresh_cnt     = 0;   /* 刷屏计数 */

    if (!track_running) return;

    uint16_t lx, ly;
    uint16_t bx, by;

    if (!K230_GetBlackPos(&bx, &by)) return;

    /* ==================== 激光笔坐标 ==================== */
    if (K230_GetLaserPos(&lx, &ly))
    {
        last_lx = lx;
        last_ly = ly;
        laser_was_valid = 1;
    }
    else
    {
        if (!laser_was_valid) return;

        lx = last_lx;
        ly = last_ly;

        float dx = (float)lx - (float)bx;
        float dy = (float)ly - (float)by;
        float dist = sqrt(dx*dx + dy*dy);

        if (dist < 10.0f)
        {
            /* 盖住了，算追上 */
            if (!track_alarm_done)
            {
                Buzzer_On();
                LED_On();
                track_alarm_done = 1;
            }

            Emm_V5_Stop_Now(1, false);
            Emm_V5_Stop_Now_2(1, false);
            return;
        }
    }

    /* ==================== 误差 ==================== */
    float errorX = (float)lx - (float)bx;
    float errorY = (float)ly - (float)by;
    float distance = sqrt(errorX*errorX + errorY*errorY);

    if (fabs(errorX) < 5.0f) errorX = 0;
    if (fabs(errorY) < 5.0f) errorY = 0;

    /* ==================== 追上判断 ==================== */
    uint8_t is_done = 0;

    if (distance <= TRACK_TOLERANCE_PX)
    {
        if (!stable_flag)
        {
            stable_flag  = 1;
            stable_start = GetTick();
        }

        if (GetTick() - stable_start >= TRACK_STABLE_MS)
        {
            is_done = 1;
        }
    }
    else
    {
        stable_flag = 0;
    }

    /* ==================== 声光提醒：追上亮，偏移灭 ==================== */
    if (is_done)
    {
        if (!track_alarm_done)
        {
            Buzzer_On();
            LED_On();
            track_alarm_done = 1;
        }
    }
    else
    {
        if (track_alarm_done)
        {
            Buzzer_Off();
            LED_Off();
            track_alarm_done = 0;
        }
    }

    /* ==================== PID ==================== */
    PID_SetTarget(&pid_x_track, (float)bx + errorX);
    PID_SetTarget(&pid_y_track, (float)by + errorY);

    float vx = PID_Calc(&pid_x_track, (float)bx);
    float vy = PID_Calc(&pid_y_track, (float)by);

	if (distance > 200.0f)
    {
    vx *= 0.5f;
    vy *= 0.5f;
    }
    else if (distance > 100.0f)
    {
    vx *= 0.7f;
    vy *= 0.7f;
    }
	
	if (distance < 10.0f)
    { 
       vx *= 0.05f; 
       vy *= 0.05f; 
    }
       if (distance < 20.0f)
    {
        vx *= 0.1f;
        vy *= 0.1f;
    }
    else if (distance < 40.0f)
    {
        vx *= 0.3f;
        vy *= 0.3f;
    }

    int16_t vel1 = (int16_t)( vx + vy);
    int16_t vel2 = (int16_t)( vx - vy);

    if (vel1 >  TRACK_SPEED_MAX) vel1 =  TRACK_SPEED_MAX;
    if (vel1 < -TRACK_SPEED_MAX) vel1 = -TRACK_SPEED_MAX;
    if (vel2 >  TRACK_SPEED_MAX) vel2 =  TRACK_SPEED_MAX;
    if (vel2 < -TRACK_SPEED_MAX) vel2 = -TRACK_SPEED_MAX;

    uint8_t  dir1, dir2;
    uint16_t spd1, spd2;

    if (vel1 >= 0) { dir1 = 0; spd1 =  vel1; }
    else           { dir1 = 1; spd1 = -vel1; }

    if (vel2 >= 0) { dir2 = 0; spd2 =  vel2; }
    else           { dir2 = 1; spd2 = -vel2; }

    /* 速度下限保底 15，防止低速走不动 */
    if (spd1 > 0 && spd1 < 15) spd1 = 15;
    if (spd2 > 0 && spd2 < 15) spd2 = 15;

    if (spd1 > 0) Emm_V5_Vel_Control(1, dir1, spd1, 50, false);
    if (spd2 > 0) Emm_V5_Vel_Control_2(1, dir2, spd2, 50, false);
    /* 画轨迹，每 5 轮刷一次 */
    OLED_DrawPoint(bx / 5, by / 8, 1);

    refresh_cnt++;
    if (refresh_cnt >= 5)
    {
        refresh_cnt = 0;
        OLED_Refresh();
    }
}
//	//中心画圆

/* ==================== 初始化 ==================== */
void Func_Circle_Init(void)
{
    PID_Init(&pid_x_circle, 1.5f, 0.0f, 0.15f, 500.0f);
    PID_Init(&pid_y_circle, 2.0f, 0.0f, 0.3f, 1000.0f);   /* Y Kd 0.12 → 0.3 */
}

/* ==================== 触发 ==================== */
void Func_Circle(void)
{
    x_aligned      = 0;
    circle_index   = 0;
    circle_running = 1;

    /* 第一个点：θ = 0，即 (cx+RX, cy) */
    circle_targetX = CIRCLE_CX + CIRCLE_RX;
    circle_targetY = CIRCLE_CY;

    PID_SetTarget(&pid_x_circle, circle_targetX);
    PID_SetTarget(&pid_y_circle, circle_targetY);

    OLED_ShowString(2, 1, "Circle Start");
}


/* ==================== 任务 ==================== */
void Func_Circle_Task(void)
{
    if (!circle_running) return;

    uint16_t bx, by;

    if (!K230_GetBlackPos(&bx, &by)) return;

    float errorX = circle_targetX - (float)bx;
    float errorY = circle_targetY - (float)by;

    float distance = sqrt(errorX*errorX + errorY*errorY);

    /* ==================== 到点判断 ==================== */
    if (distance <= CIRCLE_ARRIVE)
    {
        circle_index++;

        if (circle_index >= CIRCLE_POINTS)
        {
            circle_running = 0;
            OLED_ShowString(2, 1, "Circle Done ");
            return;
        }

        float theta = 2.0f * 3.1415926f * circle_index / CIRCLE_POINTS;

        circle_targetX = CIRCLE_CX + CIRCLE_RX * cosf(theta);
        circle_targetY = CIRCLE_CY + CIRCLE_RY * sinf(theta);

        PID_SetTarget(&pid_x_circle, circle_targetX);
        PID_SetTarget(&pid_y_circle, circle_targetY);

        OLED_ShowNum(4, 1, circle_index, 3);
        return;
    }

    /* ==================== X、Y 一起算 ==================== */
    float vx = PID_Calc(&pid_x_circle, (float)bx);
    float vy = PID_Calc(&pid_y_circle, (float)by);

    /* ==================== 方向已确认 ==================== */
    int32_t clk1 = (int32_t)( vx + vy);
    int32_t clk2 = (int32_t)( vx - vy);

    uint8_t dir1, dir2;

    if (clk1 >= 0) { dir1 = 0; }
    else           { dir1 = 1; clk1 = -clk1; }

    if (clk2 >= 0) { dir2 = 0; }
    else           { dir2 = 1; clk2 = -clk2; }

    if (clk1 == 0) clk1 = 1;
    if (clk1 > 500) clk1 = 500;

    if (clk2 < 5) clk2 = 5;
    if (clk2 > 1000) clk2 = 1000;

    OLED_ShowNum(3, 1, (int32_t)circle_targetX, 4);
    OLED_ShowNum(3, 8, (int32_t)bx, 4);

    Emm_V5_Pos_Control(1, dir1, 200, 100, (uint32_t)clk1, 0, 0);
    Emm_V5_Pos_Control_2(1, dir2, 200, 100, (uint32_t)clk2, 0, 0);
	
	
}

//激光笔为圆心画圆
void Func_LaserCircle_Init(void)
{
    PID_Init(&pid_x_lc, 1.5f, 0.0f, 0.15f, 500.0f);
    PID_Init(&pid_y_lc, 2.0f, 0.0f, 0.3f, 1000.0f);
}

void Func_LaserCircle(void)
{
    laser_circle_phase   = 0;
    laser_circle_index   = 0;
    laser_circle_running = 1;

    OLED_Clear();
    OLED_Clear_trace();
    OLED_Refresh();
}

void Func_LaserCircle_Task(void)
{
    static uint16_t last_lx = 0;
    static uint16_t last_ly = 0;
    static uint32_t stable_start = 0;
    static uint8_t  first_flag   = 0;
    static uint8_t  refresh_cnt  = 0;

    /* ==================== 第一阶段：等激光笔稳定 1.5 秒 ==================== */
    if (laser_circle_phase == 0)
    {
        uint16_t lx, ly;

        if (!K230_GetLaserPos(&lx, &ly))
        {
            first_flag = 0;
            return;
        }

        if (!first_flag)
        {
            last_lx = lx;
            last_ly = ly;
            first_flag   = 1;
            stable_start = GetTick();
            return;
        }

        if (abs((int)lx - (int)last_lx) > LASER_RANGE_PX ||
            abs((int)ly - (int)last_ly) > LASER_RANGE_PX)
        {
            last_lx = lx;
            last_ly = ly;
            stable_start = GetTick();
            return;
        }

        if (GetTick() - stable_start >= LASER_HOLD_MS)
        {
            laser_cx = (float)lx;
            laser_cy = (float)ly;

            laser_circle_phase = 1;

            laser_circle_targetX = laser_cx;
            laser_circle_targetY = laser_cy;

            PID_SetTarget(&pid_x_lc, laser_circle_targetX);
            PID_SetTarget(&pid_y_lc, laser_circle_targetY);
        }
        return;
    }

    /* ==================== 第二阶段：走到圆心 ==================== */
    if (laser_circle_phase == 1)
    {
        uint16_t bx, by;

        if (!K230_GetBlackPos(&bx, &by)) return;

        float errorX = laser_cx - (float)bx;
        float errorY = laser_cy - (float)by;
        float distance = sqrt(errorX*errorX + errorY*errorY);

        if (distance <= LASER_CIRCLE_ARRIVE)
        {
            laser_circle_phase = 2;
            laser_circle_index = 0;

            laser_circle_targetX = laser_cx + LASER_CIRCLE_RX;
            laser_circle_targetY = laser_cy;

            PID_SetTarget(&pid_x_lc, laser_circle_targetX);
            PID_SetTarget(&pid_y_lc, laser_circle_targetY);
            return;
        }

        float vx = PID_Calc(&pid_x_lc, (float)bx);
        float vy = PID_Calc(&pid_y_lc, (float)by);

        int32_t clk1 = (int32_t)( vx + vy);
        int32_t clk2 = (int32_t)( vx - vy);

        uint8_t dir1, dir2;

        if (clk1 >= 0) { dir1 = 0; }
        else           { dir1 = 1; clk1 = -clk1; }

        if (clk2 >= 0) { dir2 = 0; }
        else           { dir2 = 1; clk2 = -clk2; }

        if (clk1 == 0) clk1 = 1;
        if (clk1 > 500) clk1 = 500;

        if (clk2 < 5) clk2 = 5;
        if (clk2 > 1000) clk2 = 1000;

        OLED_DrawPoint(bx / 5, by / 8, 1);

        refresh_cnt++;
        if (refresh_cnt >= 5)
        {
            refresh_cnt = 0;
            OLED_Refresh();
        }

        Emm_V5_Pos_Control(1, dir1, 200, 100, (uint32_t)clk1, 0, 0);
        Emm_V5_Pos_Control_2(1, dir2, 200, 100, (uint32_t)clk2, 0, 0);
        return;
    }

    /* ==================== 第三阶段：画圆 ==================== */
    if (laser_circle_phase == 2)
    {
        uint16_t bx, by;

        if (!K230_GetBlackPos(&bx, &by)) return;

        float errorX = laser_circle_targetX - (float)bx;
        float errorY = laser_circle_targetY - (float)by;
        float distance = sqrt(errorX*errorX + errorY*errorY);

        if (distance <= LASER_CIRCLE_ARRIVE)
        {
            laser_circle_index++;

            if (laser_circle_index >= LASER_CIRCLE_POINTS)
            {
                laser_circle_running = 0;
                laser_circle_phase   = 0;
                return;
            }

            float theta = 2.0f * 3.1415926f * laser_circle_index / LASER_CIRCLE_POINTS;

            laser_circle_targetX = laser_cx + LASER_CIRCLE_RX * cosf(theta);
            laser_circle_targetY = laser_cy + LASER_CIRCLE_RY * sinf(theta);

            PID_SetTarget(&pid_x_lc, laser_circle_targetX);
            PID_SetTarget(&pid_y_lc, laser_circle_targetY);
            return;
        }

        float vx = PID_Calc(&pid_x_lc, (float)bx);
        float vy = PID_Calc(&pid_y_lc, (float)by);

        int32_t clk1 = (int32_t)( vx + vy);
        int32_t clk2 = (int32_t)( vx - vy);

        uint8_t dir1, dir2;

        if (clk1 >= 0) { dir1 = 0; }
        else           { dir1 = 1; clk1 = -clk1; }

        if (clk2 >= 0) { dir2 = 0; }
        else           { dir2 = 1; clk2 = -clk2; }

        if (clk1 == 0) clk1 = 1;
        if (clk1 > 500) clk1 = 500;

        if (clk2 < 5) clk2 = 5;
        if (clk2 > 1000) clk2 = 1000;

        OLED_DrawPoint(bx / 5, by / 8, 1);

        refresh_cnt++;
        if (refresh_cnt >= 5)
        {
            refresh_cnt = 0;
            OLED_Refresh();
        }

        Emm_V5_Pos_Control(1, dir1, 200, 100, (uint32_t)clk1, 0, 0);
        Emm_V5_Pos_Control_2(1, dir2, 200, 100, (uint32_t)clk2, 0, 0);
    }
}
void Func_Stop(void)
{
    Emm_V5_Stop_Now(1, false);
    Emm_V5_Stop_Now_2(1, false);
    reset_running = 0;
    OLED_ShowString(2, 1, "Motor Stop  ");
}

void Func_SelectNext(void)
{
    func_index++;
    if (func_index > 5) func_index = 1;
    OLED_ShowString(1, 1, "Select:");
    OLED_ShowNum(1, 9, func_index, 1);
}

void Func_Execute(void)
{
    if (func_index == 0) return;

    OLED_ShowString(2, 1, "Run:");
    OLED_ShowNum(2, 6, func_index, 1);

    if (func_index == 1)      { Func_LeftTop(); }   
    else if (func_index == 2) { Func_Edge(); }
    else if (func_index == 3) { Func_Track(); }
    else if (func_index == 4) { Func_Circle(); }
    else if (func_index == 5) { Func_LaserCircle();  }
}