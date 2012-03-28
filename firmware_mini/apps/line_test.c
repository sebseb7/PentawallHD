#include <stdlib.h>
#include "main.h"
#include <stdio.h>

#include "wuline.h";

static uint8_t tick(void) {

//	drawLine(1,1,10,10,255,0,0);
//	drawLine(10,1,10,10,255,255,0);
//	drawLine(1,10,10,10,255,0,255);
//	drawLine(1,3,10,10,255,0,255);
	draw_line(30,1,2,200,255,0,255);

	return 0;
}

static void init(void) ATTRIBUTES
void init(void) {
	registerAnimation(tick, 6, 450);
}




