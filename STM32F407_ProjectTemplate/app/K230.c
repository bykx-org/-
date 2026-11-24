#include "k230.h"
#include <stm32f4xx.h>

/* ---------- 接收缓冲区 ---------- */
#define RX_BUF_SIZE  64
static uint8_t rx_buf[RX_BUF_SIZE];
static uint8_t rx_index = 0;
static uint8_t rx_line_ready = 0;

/* ---------- 最新解析出的数据 ---------- */
static uint16_t black_x = 0, black_y = 0;
static uint16_t laser_x = 0, laser_y = 0;
static uint8_t  black_valid = 0;
static uint8_t  laser_valid = 0;

/* ---------- 内部函数 ---------- */
static void ParseLine(uint8_t *str, uint8_t len);

/**
  * @brief  初始化 USART3：PB10=TX, PB11=RX
  */
void K230_Init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef  NVIC_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_USART3);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_USART3);

    USART_InitStructure.USART_BaudRate            = 115200;
    USART_InitStructure.USART_WordLength          = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits            = USART_StopBits_1;
    USART_InitStructure.USART_Parity              = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART3, &USART_InitStructure);

    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
    USART_Cmd(USART3, ENABLE);

    NVIC_InitStructure.NVIC_IRQChannel                   = USART3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0;
    NVIC_Init(&NVIC_InitStructure);
}

/**
  * @brief  主循环调用，检查有没有完整一帧要解析
  */
void K230_Process(void)
{
    if (rx_line_ready)
    {
        uint8_t line[RX_BUF_SIZE];
        uint8_t len = rx_index;

        for (uint8_t i = 0; i < len; i++) line[i] = rx_buf[i];

        rx_index = 0;
        rx_line_ready = 0;

        ParseLine(line, len);
    }
}

/**
  * @brief  取黑色物块像素坐标
  */
uint8_t K230_GetBlackPos(uint16_t *x, uint16_t *y)
{
    if (black_valid)
    {
        *x = black_x;
        *y = black_y;
        return 1;
    }
    return 0;
}

/**
  * @brief  取激光笔像素坐标
  */
uint8_t K230_GetLaserPos(uint16_t *x, uint16_t *y)
{
    if (laser_valid)
    {
        *x = laser_x;
        *y = laser_y;
        return 1;
    }
    return 0;
}

uint8_t K230_IsBlackValid(void) { return black_valid; }
uint8_t K230_IsLaserValid(void) { return laser_valid; }

/* ---------- 串口中断 ---------- */
void USART3_IRQHandler(void)
{
    if (USART_GetITStatus(USART3, USART_IT_RXNE) == SET)
    {
        uint8_t ch = USART_ReceiveData(USART3);

        if (rx_index < RX_BUF_SIZE - 1)
        {
            rx_buf[rx_index++] = ch;

            /* 帧尾 0x0D，或者换行 '\n' 都当作一帧结束 */
            if (ch == 0x0D || ch == '\n')
            {
                rx_buf[rx_index] = 0;
                rx_line_ready = 1;
            }
        }
        else
        {
            rx_buf[RX_BUF_SIZE - 1] = 0;
            rx_line_ready = 1;
            rx_index = 0;
        }

        USART_ClearITPendingBit(USART3, USART_IT_RXNE);
    }
}

/**
  * @brief  解析一帧二进制数据
  *         帧格式：0xAA 0x55
  *                 [black_ok][bx_H][bx_L][by_H][by_L]
  *                 [laser_ok][lx_H][lx_L][ly_H][ly_L]
  *                 0x0D
  */
static void ParseLine(uint8_t *str, uint8_t len)
{
    /* 找 0xAA 0x55 */
    for (uint8_t i = 0; i + 12 < len; i++)
    {
        if (str[i] == 0xAA && str[i+1] == 0x55)
        {
            black_valid = str[i+2];
            black_x = (str[i+3] << 8) | str[i+4];
            black_y = (str[i+5] << 8) | str[i+6];

            laser_valid = str[i+7];
            laser_x = (str[i+8] << 8) | str[i+9];
            laser_y = (str[i+10] << 8) | str[i+11];

            return;
        }
    }

    black_valid = 0;
    laser_valid = 0;
}