#ifndef __FIFO2_H
#define __FIFO2_H

#include <stm32f4xx.h>
#include <stdbool.h>

#define FIFO2_SIZE 128

typedef struct {
    uint16_t buffer[FIFO2_SIZE];
    __IO uint8_t ptrWrite;
    __IO uint8_t ptrRead;
} FIFO2_t;

extern __IO FIFO2_t rxFIFO2;

void     fifo2_initQueue(void);
void     fifo2_enQueue(uint16_t data);
uint16_t fifo2_deQueue(void);
bool     fifo2_isEmpty(void);
uint16_t fifo2_queueLength(void);

#endif
