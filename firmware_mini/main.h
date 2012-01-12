#ifndef _MAIN_H
#define _MAIN_H

#define ATTRIBUTES  __attribute__ ((naked, used, section (".init8")));

#include <inttypes.h>

#define LED_WIDTH   4
#define LED_HEIGHT  4

void setLedXY(uint8_t,uint8_t,uint8_t,uint8_t,uint8_t); // x , y , r , g , b
void registerAnimation(uint8_t (*)(void),uint16_t,uint16_t);


#endif
