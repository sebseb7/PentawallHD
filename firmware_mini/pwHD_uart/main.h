#ifndef _MAIN_H
#define _MAIN_H

#include <stdint.h>

#define LED_WIDTH	24
#define LED_HEIGHT	24

#define ATTRIBUTES	__attribute__((constructor));

typedef uint8_t (*tick_fun)(void);

void setLedXY(uint8_t x, uint8_t y, uint8_t red,uint8_t green, uint8_t blue);
void registerAnimation(tick_fun tick, uint16_t t, uint16_t duration);

void write_escaped(uint8_t byte);
void write_pixel(uint8_t x,uint8_t y,uint8_t r,uint8_t g,uint8_t b) ;

#endif

