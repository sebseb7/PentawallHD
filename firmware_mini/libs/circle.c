/*
 * modified version of : http://rosettacode.org/wiki/Xiaolin_Wu's_line_algorithm
 */



#include "stdio.h"
#include "main.h"
#include <math.h>

double pythagoras( double side1, double side2 );

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

void draw_filledCircle(
	unsigned int x, unsigned int y,
	uint8_t rad,
	uint8_t r,
	uint8_t g,
	uint8_t b )
{

	uint8_t i,j;

	for(i=0;i<rad;i++)
	{
		for(j=0;j<rad;j++)
		{
			double dist = pythagoras( j,i );
			if(dist <= rad-1)
			{
				setLedXY(y-j,x+i,r,g,b);
				setLedXY(y+j,x+i,r,g,b);
				setLedXY(y+j,x-i,r,g,b);
				setLedXY(y-j,x-i,r,g,b);
			}else if(dist < rad)
			{
				dla_plot(y-j,x+i,r,g,b,1-(dist-rad+1));
				dla_plot(y+j,x+i,r,g,b,1-(dist-rad+1));
				dla_plot(y+j,x-i,r,g,b,1-(dist-rad+1));
				dla_plot(y-j,x-i,r,g,b,1-(dist-rad+1));
			}
		}
	}



}

double pythagoras( double side1, double side2 )
{
	return sqrt(pow( side1, 2 ) + pow( side2, 2 ));
}


