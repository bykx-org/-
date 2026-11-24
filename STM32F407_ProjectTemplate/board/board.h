#ifndef __BOARD_H
#define __BOARD_H

#include "stm32f4xx.h"

void systick_init(void);
void delay_us(uint32_t _us);
void delay_ms(uint32_t _ms);
void delay_1ms(uint32_t ms);
void delay_1us(uint32_t us);

#endif
