#include "usart2.h"

__IO bool    rxFrameFlag2 = false;
__IO uint8_t rxCmd2[FIFO2_SIZE] = {0};
__IO uint8_t rxCount2 = 0;

void USART2_IRQHandler(void)
{
    __IO uint16_t i = 0;

    if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
    {
        fifo2_enQueue((uint8_t)USART2->DR);
        USART_ClearITPendingBit(USART2, USART_IT_RXNE);
    }
    else if (USART_GetITStatus(USART2, USART_IT_IDLE) != RESET)
    {
        USART2->SR; USART2->DR;

        rxCount2 = fifo2_queueLength();
        for (i = 0; i < rxCount2; i++) { rxCmd2[i] = fifo2_deQueue(); }

        rxFrameFlag2 = true;
    }
}

void usart2_SendCmd(__IO uint8_t *cmd, uint8_t len)
{
    __IO uint8_t i = 0;
    for (i = 0; i < len; i++) { usart2_SendByte(cmd[i]); }
}

void usart2_SendByte(uint16_t data)
{
    __IO uint16_t t0 = 0;

    USART2->DR = (data & (uint16_t)0x01FF);

    while (!(USART2->SR & USART_FLAG_TXE))
    {
        ++t0; if (t0 > 8000) { return; }
    }
}

