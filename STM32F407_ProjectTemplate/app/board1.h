#ifndef __BOARD1_H
#define __BOARD1_H

#include "stm32f4xx.h"

void nvic_init(void);
void clock_init(void);
void usart_init(void);
void usart2_init(void);
void board_init(void);

#endif
