#include "fifo2.h"

__IO FIFO2_t rxFIFO2 = {0};

void fifo2_initQueue(void)
{
    rxFIFO2.ptrRead  = 0;
    rxFIFO2.ptrWrite = 0;
}

void fifo2_enQueue(uint16_t data)
{
    rxFIFO2.buffer[rxFIFO2.ptrWrite] = data;
    ++rxFIFO2.ptrWrite;
    if (rxFIFO2.ptrWrite >= FIFO2_SIZE) rxFIFO2.ptrWrite = 0;
}

uint16_t fifo2_deQueue(void)
{
    uint16_t element = rxFIFO2.buffer[rxFIFO2.ptrRead];
    ++rxFIFO2.ptrRead;
    if (rxFIFO2.ptrRead >= FIFO2_SIZE) rxFIFO2.ptrRead = 0;
    return element;
}

bool fifo2_isEmpty(void)
{
    return (rxFIFO2.ptrRead == rxFIFO2.ptrWrite);
}

uint16_t fifo2_queueLength(void)
{
    if (rxFIFO2.ptrRead <= rxFIFO2.ptrWrite)
        return (rxFIFO2.ptrWrite - rxFIFO2.ptrRead);
    else
        return (FIFO2_SIZE - rxFIFO2.ptrRead + rxFIFO2.ptrWrite);
}
