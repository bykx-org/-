#include "Buzzer.h"

void Buzzer_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

    GPIO_InitStruct.GPIO_Pin   = GPIO_Pin_10;
    GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOE, &GPIO_InitStruct);

    GPIO_SetBits(GPIOE, GPIO_Pin_10);    
}



void Buzzer_On(void)
{
    GPIO_ResetBits(GPIOE, GPIO_Pin_10);
}
void Buzzer_Off(void)
{
    GPIO_SetBits(GPIOE, GPIO_Pin_10);
}

