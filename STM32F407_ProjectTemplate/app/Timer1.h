#ifndef __TIMER1_H
#define __TIMER1_H

#include "stm32f4xx.h"

extern uint8_t pid_flag;
extern volatile uint32_t g_ms_tick;

void Timer1_Init(void);
uint32_t GetTick(void);

#endif