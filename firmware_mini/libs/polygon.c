/*
 * modified version of : http://rosettacode.org/wiki/Xiaolin_Wu's_line_algorithm
 */



#include "stdio.h"
#include "main.h"
#include <math.h>


static void dla_plot(int x, int y, uint8_t r,uint8_t g , uint8_t b, float br)
{
	uint8_t o_red;
	uint8_t o_green;
	uint8_t o_blue;

	getLedXY(x,y,&o_red,&o_green,&o_blue);

	r=br*r+((1-br)*o_red);
	g=br*g+((1-br)*o_green);
	b=br*b+((1-br)*o_blue);
	setLedXY(x, y, r,g,b);


}

void draw_filledPoly(
	unsigned int x1, unsigned int y1,
	unsigned int x2, unsigned int y2,
	unsigned int x3, unsigned int y3,
	uint8_t r,
	uint8_t g,
	uint8_t b )
{
	unsigned int maxx = x1;
	unsigned int minx = x1;
	unsigned int maxy = y1;
	unsigned int miny = y1;
	if(x2>maxx) maxx=x2;
	if(x3>maxx) maxx=x3;
	if(y2>maxy) maxy=y2;
	if(y3>maxy) maxy=y3;
	if(x2<minx) minx=x2;
	if(x3<minx) minx=x3;
	if(y2<miny) miny=x2;
	if(y3<miny) miny=x3;

	int inside = 0;

	for(unsigned int i = minx;i<maxx;i++)
	{
		for(unsigned int j = miny;j<maxy;j++)
		{
			
			setLedXY(i,j,r,g,b);
		}
	}


}


