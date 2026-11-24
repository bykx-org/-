#ifndef __K230_H
#define __K230_H

#include "stm32f4xx.h"

void    K230_Init(void);
void    K230_Process(void);

uint8_t K230_GetBlackPos(uint16_t *x, uint16_t *y);
uint8_t K230_GetLaserPos(uint16_t *x, uint16_t *y);
uint8_t K230_IsBlackValid(void);
uint8_t K230_IsLaserValid(void);

#endif