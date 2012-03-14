#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <time.h>
#include <termios.h>
#include <string.h>

#if defined(__APPLE__)
#include <AvailabilityMacros.h>
#endif

#if defined(MAC_OS_X_VERSION_10_4) && (MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_4)
#include <sys/ioctl.h>
#include <IOKit/serial/ioss.h>
#include <errno.h>
#endif



int tty_fd;


#include "main.h"


int leds[LED_HEIGHT][LED_WIDTH][3];
int leds_buf[LED_HEIGHT][LED_WIDTH][3];
int interval;
int pixels_updated = 0;
tick_fun tick_fp;

void setLedXY(uint8_t x, uint8_t y, uint8_t red,uint8_t green, uint8_t blue) {
	assert(x < LED_WIDTH);
	assert(y < LED_HEIGHT);
	leds[y][x][0] = red;
	leds[y][x][1] = green;
	leds[y][x][2] = blue;
	pixels_updated++;
}

void registerAnimation(tick_fun tick, uint16_t t, uint16_t ignore)
{
	tick_fp = tick;

	assert(t > 0);
	// 122Hz / tick
	interval = 1000000 / 122 * t;
}


int main(int argc, char *argv[]) {

	struct termios tio;

	tio.c_iflag=0;
	tio.c_oflag=0;
	tio.c_cflag=CS8|CREAD|CLOCAL;           // 8n1, see termios.h for more information
	tio.c_lflag=0;
	tio.c_cc[VMIN]=1;
	tio.c_cc[VTIME]=5;
#if defined(MAC_OS_X_VERSION_10_4) && (MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_4)
	if ( (tty_fd=open("/dev/cu.usbserial-A100DEF7", O_RDWR )) == -1)
	{
		printf( "Error %d opening device)\n", errno );
	}
	speed_t speed = 500000;
	if ( ioctl( tty_fd,	 IOSSIOSPEED, &speed ) == -1 )
	{
		printf( "Error %d calling ioctl( ..., IOSSIOSPEED, ... )\n", errno );
	}
#else
//       tty_fd=open("/dev/ttyUSB1", O_RDWR );
	tty_fd=open("/dev/ttyUSB1", O_RDWR | O_NONBLOCK);      
	cfsetospeed(&tio,B500000);
	cfsetispeed(&tio,B500000);
#endif 

	tcsetattr(tty_fd,TCSANOW,&tio);



	while(1) {

		pixels_updated=0;

		tick_fp();


		if(pixels_updated < 100)
		{
			int x, y;
			for(x = 0; x < LED_WIDTH; x++) {
				for(y = 0; y < LED_HEIGHT; y++) {

					if(
						(leds[y][x][0] != leds_buf[y][x][0])|
						(leds[y][x][1] != leds_buf[y][x][1])|
						(leds[y][x][2] != leds_buf[y][x][2])
					)
					{
						write_pixel(x,y,leds[y][x][0],leds[y][x][1],leds[y][x][2]);

						leds_buf[y][x][0]=leds[y][x][0];
						leds_buf[y][x][1]=leds[y][x][1];
						leds_buf[y][x][2]=leds[y][x][2];
					}

				}
			}
		}
		else
		{
			//write_Frame
		}
		usleep(interval);
	}

}

void write_pixel(uint8_t x,uint8_t y,uint8_t r,uint8_t g,uint8_t b) 
{
	unsigned char c = 0x42;
	write(tty_fd,&c,1);
	write_escaped(x);
	write_escaped(y);
	write_escaped(r);
	write_escaped(g);
	write_escaped(b);
}

void write_escaped(uint8_t byte)
{
	if(byte == 0x23)
	{
		uint8_t c = 0x65;
		write(tty_fd,&c,1);
		c = 1;
		write(tty_fd,&c,1);
	}
	else if(byte == 0x42)
	{
		uint8_t c = 0x65;
		write(tty_fd,&c,1);
		c = 2;
		write(tty_fd,&c,1);
	}
	else if(byte == 0x65)
	{
		uint8_t c = 0x65;
		write(tty_fd,&c,1);
		c = 3;
		write(tty_fd,&c,1);
	}
	else if(byte == 0x66)
	{
		uint8_t c = 0x65;
		write(tty_fd,&c,1);
		c = 4;
		write(tty_fd,&c,1);
	}
	else
	{
		unsigned char c = byte;
		write(tty_fd,&c,1);
	}
}
