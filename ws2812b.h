/*
 * ws2812.h
 *
 *  Created on: May 27, 2026
 *      Author: Administrator
 */

#ifndef INC_WS2812B_H_
#define INC_WS2812B_H_

#include "main.h"

void WS2812_Init(void);
void WS2812_SetPixel(uint8_t index,uint8_t r,uint8_t g,uint8_t b);
void WS2812_Show(void);
void WS2812_Clear(void);

#endif /* INC_WS2812B_H_ */
