#ifndef __FUNCTION_H
#define __FUNCTION_H

#include "stm32f4xx.h"

extern uint8_t reset_running;
extern uint8_t left_top_running; 
extern uint8_t edge_running;
extern uint8_t track_running;
extern uint8_t circle_running;
extern uint8_t laser_circle_running;
//第一点
void Func_Reset_Init(void);
void Func_Reset(void);
void Func_Reset_Task(void);
//左上角
void Func_LeftTop_Init(void);
void Func_LeftTop(void);
void Func_LeftTop_Task(void);
//第二点
void Func_Edge_Init(void);
void Func_Edge(void);
void Func_Edge_Task(void);
//追踪系统
void Func_Track_Init(void);
void Func_Track(void);
void Func_Track_Task(void);
//中心画圆
void Func_Circle_Init(void);
void Func_Circle(void);
void Func_Circle_Task(void);
//激光笔画圆
void Func_LaserCircle_Init(void);
void Func_LaserCircle(void);
void Func_LaserCircle_Task(void);





void Func_Stop(void);
void Func_SelectNext(void);
void Func_Execute(void);

#endif