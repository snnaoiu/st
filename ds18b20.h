/*
 * ds18b20.h
 *
 *  Created on: May 19, 2026
 *      Author: Administrator
 */

#ifndef INC_DS18B20_H_
#define INC_DS18B20_H_


#include "main.h"

uint8_t DS18B20_Reset(void);

void DS18B20_WriteBit(uint8_t bit);
void DS18B20_WriteByte(uint8_t byte);

uint8_t DS18B20_ReadBit(void);
uint8_t DS18B20_ReadByte(void);
void DWT_Delay_Init(void);

void DS18B20_StartConvert(void);
float DS18B20_ReadTemp(void);

float DS18B20_GetTemp(void);

#endif /* INC_DS18B20_H_ */
