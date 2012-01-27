#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <termios.h>

#include "keyboard.h"
#include "main.h"

int tty_fd;


uint8_t fac1 = 255;
uint8_t fac2 = 1;

// 48 50 52 53 55 57 59 60 62  64 65 67

static int running = 1;
static void leave(int sig) { running = 0; }


int main(int argc, char** argv) {
	signal(SIGINT, leave);


		struct termios tio;
 
        memset(&tio,0,sizeof(tio));
        tio.c_iflag=0;
        tio.c_oflag=0;
        tio.c_cflag=CS8|CREAD|CLOCAL;           // 8n1, see termios.h for more information
        tio.c_lflag=0;
        tio.c_cc[VMIN]=1;
        tio.c_cc[VTIME]=5;
        tty_fd=open("/dev/ttyUSB0", O_RDWR | O_NONBLOCK);      
        cfsetospeed(&tio,B500000);
		cfsetispeed(&tio,B500000);
 
        tcsetattr(tty_fd,TCSANOW,&tio);


	keyboard_init();


	KeyboardEvent e;

	int intensity_r = 0;
	int intensity_g = 0;
	int intensity_b = 0;
	int strobe_r = 0;
	int strobe_g = 0;
	int strobe_b = 0;
	int rise_r = 0;
	int rise_g = 0;
	int rise_b = 0;
	int on_r = 0;
	int on_g = 0;
	int on_b = 0;
	int fspeed = 64;
	int count=0;
	int rerender=1;

	while(running) {
		count++;

		while(keyboard_poll(&e)) {
			printf("pressed  %3d %3d %3d\n", e.x, e.y,e.type);
			if(e.type == 144)
			{
				switch(e.x){

				case 49:
					if(fspeed < 512)
						fspeed*=2;
					break;
				case 51:
					if(fspeed > 1)
						fspeed/=2;
					break;
				case 48:
					rerender=1;
					intensity_r=e.y*2;
					break;
				case 50:
					rerender=1;
					intensity_g=e.y*2;
					break;
				case 52:
					rerender=1;
					intensity_b=e.y*2;
					break;

				case 53:
					on_r=1;
					rerender=1;
					break;

				case 55:
					on_g=1;
					rerender=1;
					break;

				case 57:
					on_b=1;
					rerender=1;
					break;

				case 59:
					strobe_r=1;
					break;

				case 60:
					strobe_g=1;
					break;

				case 62:
					strobe_b=1;
					break;

				case 64:
					rerender=1;
					rise_r=1;
					break;

				case 65:
					rerender=1;
					rise_g=1;
					break;

				case 67:
					rerender=1;
					rise_b=1;
					break;
				}
			
			
				printf("pressed  %3d %3d\n", e.x, e.y);
			}
			else if(e.type == 128)
			{

				switch(e.x){

				case 53:
					on_r=0;
					intensity_r = fac1;
					rerender=1;
					break;

				case 55:
					on_g=0;
					intensity_g= fac1;
					rerender=1;
					break;

				case 57:
					on_b=0;
					intensity_b = fac1;
					rerender=1;
					break;
				case 59:
					strobe_r=0;
					intensity_r = fac1;
					break;

				case 60:
					strobe_g=0;
					intensity_g= fac1;
					break;

				case 62:
					strobe_b=0;
					intensity_b = fac1;
					break;
				case 64:
					rerender=1;
					rise_r=0;
					break;

				case 65:
					rerender=1;
					rise_g=0;
					break;

				case 67:
					rerender=1;
					rise_b=0;
					break;
				}


				printf("released %3d %3d\n", e.x, e.y);
			}
		}

		if(strobe_r || strobe_b || strobe_g)
		{
			rerender=1;
		}

		if(rerender) {
			rerender = 0;

			if(strobe_r == 1)
			{
				intensity_r = ((count&fspeed)/fspeed)*255;
			}
			if(strobe_g == 1)
			{
				intensity_g = ((count&fspeed)/fspeed)*255;
			}
			if(strobe_b == 1)
			{
				intensity_b = ((count&fspeed)/fspeed)*255;
			}
			if(on_r == 1)
			{
				intensity_r = 55;
			}
			if(on_g == 1)
			{
				intensity_g = 55;
			}
			if(on_b == 1)
			{
				intensity_b = 55;
			}


			pixel(intensity_r*fac2,intensity_g*fac2,intensity_b*fac2);
			if(on_r == 1)
			{
				intensity_r = 0;
			}
			if(on_g == 1)
			{
				intensity_g = 0;
			}
			if(on_b == 1)
			{
				intensity_b = 0;
			}
		}

		if(intensity_r > 0)
		{
			intensity_r--;
			rerender=1;
		}
		if(intensity_g > 0)
		{
			intensity_g--;
			rerender=1;
		}
		if(intensity_b > 0)
		{
			intensity_b--;
			rerender=1;
		}



		usleep(1000);
	}

	keyboard_kill();

	return 0;
}



void pixel(uint8_t r,uint8_t g,uint8_t b) 
{
	unsigned char c = 66;
	write(tty_fd,&c,1);
	write_escaped(0);
	write_escaped(0);
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
        

