#include "Key.h"
#include "board.h"

void Key_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;//rang不按时是3.3v
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_3 | GPIO_Pin_4 |
                                    GPIO_Pin_7 | GPIO_Pin_8;
    GPIO_Init(GPIOE, &GPIO_InitStructure);
}

uint8_t Key_GetNum(void)
{
    uint8_t KeyNum = 0;

    if (GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_3) == 0)
    {
        delay_ms(20);
        while (GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_3) == 0);
        delay_ms(20);
        KeyNum = 1;
    }

    if (GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_4) == 0)
    {
        delay_ms(20);
        while (GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_4) == 0);
        delay_ms(20);
        KeyNum = 2;
    }

    if (GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_7) == 0)
    {
        delay_ms(20);
        while (GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_7) == 0);
        delay_ms(20);
        KeyNum = 3;
    }

    if (GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_8) == 0)
    {
        delay_ms(20);
        while (GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_8) == 0);
        delay_ms(20);
        KeyNum = 4;
    }

    return KeyNum;
}