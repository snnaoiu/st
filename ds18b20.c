/*
 * ds18b20.c
 *
 *  Created on: May 19, 2026
 *      Author: Administrator
 */

#include "ds18b20.h"

#define DS18B20_PORT GPIOC
#define DS18B20_PIN GPIO_PIN_8

#define DS18B20_DQ_LOW() \
	HAL_GPIO_WritePin(DS18B20_PORT, DS18B20_PIN, GPIO_PIN_RESET)
#define DS18B20_DQ_HIGH() \
	HAL_GPIO_WritePin(DS18B20_PORT, DS18B20_PIN, GPIO_PIN_SET)
#define DS18B20_READ() \
	HAL_GPIO_ReadPin(DS18B20_PORT, DS18B20_PIN)

void DWT_Delay_Init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

static void delay_us(uint32_t us)
{
    uint32_t clk = HAL_RCC_GetHCLKFreq() / 1000000U;
    uint32_t start = DWT->CYCCNT;
    uint32_t ticks = us * clk;

    while ((DWT->CYCCNT - start) < ticks);
}

uint8_t DS18B20_Reset(void){
	uint8_t retry = 0;
	DS18B20_DQ_LOW();
	delay_us(750);

	DS18B20_DQ_HIGH();
	delay_us(15);

	while(DS18B20_READ()){
		retry++;
		if(retry > 200){
			return 1;
		}
		delay_us(1);
	}
	retry = 0;

	while(!DS18B20_READ()){
		retry++;
		if(retry > 240){
			return 1;
		}

		delay_us(1);
	}
	return 0;
}

void DS18B20_WriteBit(uint8_t bit){
	if(bit){
		DS18B20_DQ_LOW();
		delay_us(2);

		DS18B20_DQ_HIGH();
		delay_us(60);
	}
	else{
		DS18B20_DQ_LOW();
		delay_us(60);

		DS18B20_DQ_HIGH();
		delay_us(2);
	}
}

void DS18B20_WriteByte(uint8_t byte){
	for(uint8_t i = 0; i < 8; i++){
		DS18B20_WriteBit(byte & 0x01);
		byte >>= 1;
	}
}

uint8_t DS18B20_ReadBit(void)
{
    uint8_t bit;

    DS18B20_DQ_LOW();
    delay_us(2);

    DS18B20_DQ_HIGH();   // 释放总线
    delay_us(12);        // 关键：等到约15us再采样

    bit = DS18B20_READ();

    delay_us(50);        // 补满一个时隙
    return bit;
}

uint8_t DS18B20_ReadByte(void){
	uint8_t i;
	uint8_t byte = 0;

	for(i = 0; i < 8; i++){

		if(DS18B20_ReadBit()){
			byte |= (1 << i);
		}
	}
	return byte;
}

float DS18B20_GetTemp(void)
{
    uint8_t temp_l;
    uint8_t temp_h;
    int16_t raw;
    int32_t temp100;

    if (DS18B20_Reset() != 0)
    {
        return -1000.0f;
    }

    DS18B20_WriteByte(0xCC);
    DS18B20_WriteByte(0x44);
    HAL_Delay(750);

    if (DS18B20_Reset() != 0)
    {
        return -1000.0f;
    }

    DS18B20_WriteByte(0xCC);
    DS18B20_WriteByte(0xBE);

    temp_l = DS18B20_ReadByte();
    temp_h = DS18B20_ReadByte();

    raw = (int16_t)(((uint16_t)temp_h << 8) | temp_l);

    temp100 = ((int32_t)raw * 100) / 16;

    return (float)temp100 / 100.0f;
}

void DS18B20_StartConvert(void)
{
    if (DS18B20_Reset() != 0)
    {
        return;
    }

    DS18B20_WriteByte(0xCC);   // 跳过ROM
    DS18B20_WriteByte(0x44);   // 开始温度转换
}

float DS18B20_ReadTemp(void)
{
    uint8_t temp_l;
    uint8_t temp_h;
    int16_t raw;
    int32_t temp100;

    if (DS18B20_Reset() != 0)
    {
        return -1000.0f;
    }

    DS18B20_WriteByte(0xCC);
    DS18B20_WriteByte(0xBE);

    temp_l = DS18B20_ReadByte();
    temp_h = DS18B20_ReadByte();

    raw = (int16_t)(((uint16_t)temp_h << 8) | temp_l);
    temp100 = ((int32_t)raw * 100) / 16;

    return (float)temp100 / 100.0f;
}
