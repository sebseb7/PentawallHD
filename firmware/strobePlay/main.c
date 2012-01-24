#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <SDL/SDL.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#if defined(MAC_OS_X_VERSION_10_4) && (MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_4)
#include <sys/ioctl.h>
#include <IOKit/serial/ioss.h>
#include <errno.h>
#endif



int tty_fd;

#include "main.h"

uint8_t fac1 = 32;
uint8_t fac2 = 8;

int main(int argc, char *argv[])
{
		struct termios tio;
 
        memset(&tio,0,sizeof(tio));
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
//                if (read(tty_fd,&c,1)>0)        write(STDOUT_FILENO,&c,1);              // if new data is available on the serial port, print it out
//                if (read(STDIN_FILENO,&c,1)>0)  write(tty_fd,&c,1);                     // if new data is available on the console, send it to the serial port


	SDL_Surface* screen = SDL_SetVideoMode(
		200,
		200,
		32, SDL_SWSURFACE | SDL_DOUBLEBUF);
	int rerender=1;
	int running = 1;
	int intensity_r = 0;
	int intensity_g = 0;
	int intensity_b = 0;
	int strobe_r = 0;
	int strobe_g = 0;
	int strobe_b = 0;
	int fspeed = 2;
	int count=0;
	while(running) {
		count++;

		SDL_Event ev;
		while(SDL_PollEvent(&ev)) {

			switch(ev.type) {
			case SDL_QUIT:
				running = 0;
				break;
			case SDL_KEYUP:
				switch(ev.key.keysym.sym) {
				
				case SDLK_7:
					strobe_r=0;
					intensity_r = fac1;
					break;

				case SDLK_8:
					strobe_g=0;
					intensity_g= fac1;
					break;

				case SDLK_9:
					strobe_b=0;
					intensity_b = fac1;
					break;


				default: break;
				}
				break;
			case SDL_KEYDOWN:

				switch(ev.key.keysym.sym) {
				case SDLK_ESCAPE:
					running = 0;
					break;

				case SDLK_1:
					rerender=1;
					intensity_r=fac1;
					break;

				case SDLK_2:
					rerender=1;
					intensity_g=fac1;
					break;

				case SDLK_3:
					rerender=1;
					intensity_b=fac1;
					break;

				case SDLK_4:
					break;

				case SDLK_5:
					break;

				case SDLK_6:
					break;
				
				case SDLK_7:
					strobe_r=1;
					break;

				case SDLK_8:
					strobe_g=1;
					break;

				case SDLK_9:
					strobe_b=1;
					break;
				case SDLK_q:
					fspeed=1;
					break;
				case SDLK_w:
					fspeed=4;
					break;
				case SDLK_e:
					fspeed=8;
					break;

				case SDLK_r:
					fspeed=16;
					break;

				default: break;
				}
			default: break;
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


			SDL_Rect rect = {
				0,
				0,
				200,
				200
			};
			 SDL_FillRect(screen, &rect,
			 	
				SDL_MapRGB(
					screen->format, 
					intensity_r*fac2,
					intensity_g*fac2,
					intensity_b*fac2
					 
				
				));
				pixel(intensity_r*fac2,intensity_g*fac2,intensity_b*fac2);
			SDL_Flip(screen);
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


	}
	
	close(tty_fd);
	SDL_Quit();
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

	/*unsigned char c = 66;
	write(tty_fd,&c,1);
	c = 0;
	write(tty_fd,&c,1);
	c = 0;
	write(tty_fd,&c,1);
	c = r;
	write(tty_fd,&c,1);
	c = g;
	write(tty_fd,&c,1);
	c = b;
	write(tty_fd,&c,1);*/
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
        
