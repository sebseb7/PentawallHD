#ifndef _MAIN_H
#define _MAIN_H

#include <stdint.h>

#define LED_WIDTH	24
#define LED_HEIGHT	24

// multiple of 15
#define ZOOM 30


#define SIMULATOR

#define ATTRIBUTES	__attribute__((constructor));

typedef uint8_t (*tick_fun)(void);

void setLedXY(uint8_t x, uint8_t y, uint8_t red,uint8_t green, uint8_t blue);
void getLedXY(uint8_t x, uint8_t y, uint8_t* red,uint8_t* green, uint8_t* blue);
void registerAnimation(tick_fun tick, uint16_t t, uint16_t duration);
void registerApp(tick_fun tick, uint16_t t);

#endif

