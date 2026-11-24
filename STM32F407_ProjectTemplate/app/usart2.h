#ifndef __USART2_H
#define __USART2_H

#include "board1.h"
#include "fifo2.h"

extern __IO bool    rxFrameFlag2;
extern __IO uint8_t rxCmd2[FIFO2_SIZE];
extern __IO uint8_t rxCount2;

void usart2_SendCmd(__IO uint8_t *cmd, uint8_t len);
void usart2_SendByte(uint16_t data);

#endif
