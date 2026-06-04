#include "ws2812b.h"

#define LED_NUM 4

#define WS_PORT GPIOC
#define WS_PIN  GPIO_PIN_9

#define WS_HIGH() (WS_PORT->BSRR = WS_PIN)
#define WS_LOW()  (WS_PORT->BSRR = (uint32_t)WS_PIN << 16)

static uint8_t led_buf[LED_NUM][3];

static inline void delay_short(void)
{
    __NOP();
    __NOP();
    __NOP();
    __NOP();
}

static inline void send_0(void)
{
    WS_HIGH();

    __NOP();
    __NOP();
    __NOP();
    __NOP();

    WS_LOW();

    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
}

static inline void send_1(void)
{
    WS_HIGH();

    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();

    WS_LOW();

    __NOP();
    __NOP();
    __NOP();
}

static void send_byte(uint8_t byte)
{
    for(uint8_t i=0;i<8;i++)
    {
        if(byte & 0x80)
        {
            send_1();
        }
        else
        {
            send_0();
        }

        byte <<= 1;
    }
}

void WS2812_Init(void)
{
    WS_LOW();
}

void WS2812_SetPixel(uint8_t index,uint8_t r,uint8_t g,uint8_t b)
{
    if(index >= LED_NUM)
        return;

    /*
    WS2812 是 GRB
    */

    led_buf[index][0] = g;
    led_buf[index][1] = r;
    led_buf[index][2] = b;
}

void WS2812_Clear(void)
{
    for(int i=0;i<LED_NUM;i++)
    {
        led_buf[i][0] = 0;
        led_buf[i][1] = 0;
        led_buf[i][2] = 0;
    }
}

void WS2812_Show(void)
{
    __disable_irq();

    for(int i=0;i<LED_NUM;i++)
    {
        send_byte(led_buf[i][0]);
        send_byte(led_buf[i][1]);
        send_byte(led_buf[i][2]);
    }

    __enable_irq();

    HAL_Delay(1);
}
