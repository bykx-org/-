#include "OLED.h"
#include "OLED_Font.h"

#define OLED_ADDR   0x78



//static uint8_t OLED_GRAM[8][128];
static uint8_t OLED_GRAM[8][128];



/* ================= 软件 I2C（PB8=SCL, PB9=SDA） ================= */

static void oled_i2c_delay(void)
{
    for (volatile uint32_t i = 0; i < 20; i++);
}

static void OLED_I2C_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

    GPIO_InitStruct.GPIO_Pin   = GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_SetBits(GPIOB, GPIO_Pin_8 | GPIO_Pin_9);
}

static void OLED_I2C_Start(void)
{
    GPIO_SetBits(GPIOB, GPIO_Pin_9);
    GPIO_SetBits(GPIOB, GPIO_Pin_8);
    oled_i2c_delay();
    GPIO_ResetBits(GPIOB, GPIO_Pin_9);
    oled_i2c_delay();
    GPIO_ResetBits(GPIOB, GPIO_Pin_8);
}

static void OLED_I2C_Stop(void)
{
    GPIO_ResetBits(GPIOB, GPIO_Pin_9);
    GPIO_SetBits(GPIOB, GPIO_Pin_8);
    oled_i2c_delay();
    GPIO_SetBits(GPIOB, GPIO_Pin_9);
    oled_i2c_delay();
}

static void OLED_I2C_SendByte(uint8_t byte)
{
    uint8_t i;
    for (i = 0; i < 8; i++)
    {
        if (byte & 0x80)
            GPIO_SetBits(GPIOB, GPIO_Pin_9);
        else
            GPIO_ResetBits(GPIOB, GPIO_Pin_9);

        oled_i2c_delay();
        GPIO_SetBits(GPIOB, GPIO_Pin_8);
        oled_i2c_delay();
        GPIO_ResetBits(GPIOB, GPIO_Pin_8);
        byte <<= 1;
    }
}

static uint8_t OLED_I2C_WaitAck(void)
{
    uint8_t ack;

    GPIO_SetBits(GPIOB, GPIO_Pin_9);
    oled_i2c_delay();
    GPIO_SetBits(GPIOB, GPIO_Pin_8);
    oled_i2c_delay();

    ack = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_9);

    GPIO_ResetBits(GPIOB, GPIO_Pin_8);
    return ack;
}

/* ================= OLED 命令 / 数据 ================= */

static void OLED_WriteCmd(uint8_t cmd)
{
    OLED_I2C_Start();
    OLED_I2C_SendByte(OLED_ADDR); OLED_I2C_WaitAck();
    OLED_I2C_SendByte(0x00);      OLED_I2C_WaitAck();
    OLED_I2C_SendByte(cmd);       OLED_I2C_WaitAck();
    OLED_I2C_Stop();
}

static void OLED_WriteData(uint8_t data)
{
    OLED_I2C_Start();
    OLED_I2C_SendByte(OLED_ADDR); OLED_I2C_WaitAck();
    OLED_I2C_SendByte(0x40);      OLED_I2C_WaitAck();
    OLED_I2C_SendByte(data);      OLED_I2C_WaitAck();
    OLED_I2C_Stop();
}

/* ================= 对外接口 ================= */

void OLED_Init(void)
{
    OLED_I2C_Init();

    OLED_WriteCmd(0xAE);
    OLED_WriteCmd(0xD5); OLED_WriteCmd(0x80);
    OLED_WriteCmd(0xA8); OLED_WriteCmd(0x3F);
    OLED_WriteCmd(0xD3); OLED_WriteCmd(0x00);
    OLED_WriteCmd(0x40);
    OLED_WriteCmd(0xA1);
    OLED_WriteCmd(0xC8);
    OLED_WriteCmd(0xDA); OLED_WriteCmd(0x12);
    OLED_WriteCmd(0x81); OLED_WriteCmd(0xCF);
    OLED_WriteCmd(0xD9); OLED_WriteCmd(0xF1);
    OLED_WriteCmd(0xDB); OLED_WriteCmd(0x40);
    OLED_WriteCmd(0xA4);
    OLED_WriteCmd(0xA6);
    OLED_WriteCmd(0x8D); OLED_WriteCmd(0x14);
    OLED_WriteCmd(0xAF);

    OLED_Clear();
}

void OLED_Clear(void)
{
    uint8_t i, n;
    for (i = 0; i < 8; i++)
    {
        OLED_WriteCmd(0xB0 + i);
        OLED_WriteCmd(0x00);
        OLED_WriteCmd(0x10);
        for (n = 0; n < 128; n++)
            OLED_WriteData(0x00);
    }
}

void OLED_ShowChar(uint8_t Line, uint8_t Column, char ch)
{
    uint8_t i;
    uint8_t idx;
    uint8_t x = (Column - 1) * 8;
    uint8_t y = (Line - 1) * 2;

    if (ch < ' ' || ch > '~') return;
    idx = ch - ' ';

    OLED_WriteCmd(0xB0 + y);
    OLED_WriteCmd(((x & 0xF0) >> 4) | 0x10);
    OLED_WriteCmd(x & 0x0F);
    for (i = 0; i < 8; i++)
        OLED_WriteData(OLED_F8x16[idx][i]);

    OLED_WriteCmd(0xB0 + y + 1);
    OLED_WriteCmd(((x & 0xF0) >> 4) | 0x10);
    OLED_WriteCmd(x & 0x0F);
    for (i = 0; i < 8; i++)
        OLED_WriteData(OLED_F8x16[idx][i + 8]);
}


void OLED_ShowString(uint8_t Line, uint8_t Column, const char *str)
{
    while (*str)
    {
        OLED_ShowChar(Line, Column, *str);
        Column++;
        str++;
    }
}

uint32_t OLED_Pow(uint32_t X, uint32_t Y)
{
    uint32_t Result = 1;
    while (Y--) Result *= X;
    return Result;
}

void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
    uint8_t i;
    for (i = 0; i < Length; i++)
    {
        OLED_ShowChar(Line, Column + i,
                      Number / OLED_Pow(10, Length - i - 1) % 10 + '0');
    }
}

void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length)
{
    uint8_t i;
    uint32_t Number1;

    if (Number >= 0)
    {
        OLED_ShowChar(Line, Column, '+');
        Number1 = Number;
    }
    else
    {
        OLED_ShowChar(Line, Column, '-');
        Number1 = -Number;
    }

    for (i = 0; i < Length; i++)
    {
        OLED_ShowChar(Line, Column + i + 1,
                      Number1 / OLED_Pow(10, Length - i - 1) % 10 + '0');
    }
}

void OLED_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
    uint8_t i, SingleNumber;
    for (i = 0; i < Length; i++)
    {
        SingleNumber = Number / OLED_Pow(16, Length - i - 1) % 16;
        if (SingleNumber < 10)
            OLED_ShowChar(Line, Column + i, SingleNumber + '0');
        else
            OLED_ShowChar(Line, Column + i, SingleNumber - 10 + 'A');
    }
}

void OLED_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
    uint8_t i;
    for (i = 0; i < Length; i++)
    {
        OLED_ShowChar(Line, Column + i,
                      Number / OLED_Pow(2, Length - i - 1) % 2 + '0');
    }
}





/* 清轨迹缓冲（只清内存，不刷屏） */
void OLED_Clear_trace(void)
{
    uint8_t i, n;
    for (i = 0; i < 8; i++)
        for (n = 0; n < 128; n++)
            OLED_GRAM[i][n] = 0x00;
}

/* 刷缓冲到屏幕 */
void OLED_Refresh(void)
{
    uint8_t i, n;
    for (i = 0; i < 8; i++)
    {
        OLED_WriteCmd(0xB0 + i);
        OLED_WriteCmd(0x00);
        OLED_WriteCmd(0x10);
        for (n = 0; n < 128; n++)
            OLED_WriteData(OLED_GRAM[i][n]);
    }
}

/* 画点 */
void OLED_DrawPoint(uint8_t x, uint8_t y, uint8_t mode)
{
    uint8_t page, bit;

    if (x > 127 || y > 63) return;

    page = y / 8;
    bit  = y % 8;

    if (mode)
        OLED_GRAM[page][x] |=  (1 << bit);
    else
        OLED_GRAM[page][x] &= ~(1 << bit);
}
