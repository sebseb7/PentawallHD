#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>



#include "main.h"
#include "spi.h"

uint16_t ch[48];

union 
{
	uint8_t dcbytes[36];

	struct 
	{
		// 24*7 = 168
		unsigned int dc00 : 7;
		unsigned int dc01 : 7;
		unsigned int dc02 : 7;
		unsigned int dc03 : 7;
		unsigned int dc04 : 7;
		unsigned int dc05 : 7;
		unsigned int dc06 : 7;
		unsigned int dc07 : 7;
		unsigned int dc08 : 7;
		unsigned int dc09 : 7;
		unsigned int dc10 : 7;
		unsigned int dc11 : 7;
		unsigned int dc12 : 7;
		unsigned int dc13 : 7;
		unsigned int dc14 : 7;
		unsigned int dc15 : 7;
		unsigned int dc16 : 7;
		unsigned int dc17 : 7;
		unsigned int dc18 : 7;
		unsigned int dc19 : 7;
		unsigned int dc20 : 7;
		unsigned int dc21 : 7;
		unsigned int dc22 : 7;
		unsigned int dc23 : 7;

		// 3*8 = 24
		uint8_t gb_r ;
		uint8_t gb_g ;
		uint8_t gb_b ;

		// 7
		unsigned int dc_range_r : 1;
		unsigned int dc_range_g : 1;
		unsigned int dc_range_b : 1;
		unsigned int auto_repeat : 1;
		unsigned int timing_mode : 1;
		unsigned int counter_mode : 2;

		// 17 userdata
		unsigned int userdata0 : 8;
		unsigned int userdata1 : 8;
		unsigned int userdata2 : 1;

		// 72 ignored
		uint8_t reserved0;
		uint8_t reserved1;
		uint8_t reserved2;
		uint8_t reserved3;
		uint8_t reserved4;
		uint8_t reserved5;
		uint8_t reserved6;
		uint8_t reserved7;
		uint8_t reserved8;
	} __attribute__ ((packed));
} dcdata;


uint16_t pwmtable_8[256]  PROGMEM = { 
	0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5,
	6, 6, 6, 6, 7, 7, 7, 7, 8, 8, 8, 8, 9, 9, 9, 10,
	10, 10, 11, 11, 11, 12, 12, 13, 13, 14, 14, 15, 15, 16,
	16, 17, 17, 18, 19, 19, 20, 21, 22, 22, 23, 24, 25,
	26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 37, 38, 39,
	41, 42, 44, 45, 47, 49, 50, 52, 54, 56, 58, 60, 62,
	64, 67, 69, 72, 74, 77, 79, 82, 85, 88, 91, 95, 98,
	102, 105, 109, 113, 117, 121, 126, 130, 135, 140, 145,
	150, 155, 161, 166, 172, 179, 185, 192, 198, 206, 213,
	221, 228, 237, 245, 254, 263, 272, 282, 292, 303, 314,
	325, 337, 349, 361, 374, 387, 401, 416, 431, 446, 462,
	479, 496, 513, 532, 551, 571, 591, 612, 634, 657, 680,
	705, 730, 756, 783, 811, 840, 870, 902, 934, 967, 1002,
	1038, 1075, 1114, 1154, 1195, 1238, 1282, 1328, 1376, 1425,
	1476, 1529, 1584, 1640, 1699, 1760, 1823, 1888, 1956, 2026,
	2099, 2174, 2252, 2332, 2416, 2502, 2592, 2685, 2781, 2881,
	2984, 3091, 3202, 3316, 3435, 3558, 3686, 3818, 3954, 4095 };


//led numbering (1 ist lower left, next to right)
uint8_t idx[16] = {10,11,12,13,9,8,15,14,6,7,0,1,5,4,3,2};


void SPI_send(uint8_t cData) 
{
	SPDR = cData; 
	while(!(SPSR & (1<<SPIF)));
}

void SetLed(uint8_t led,uint8_t red,uint8_t green, uint8_t blue)
{

	if(led > 0)
	{
		ch[idx[led-1]*3+2]=pgm_read_word(pwmtable_8+blue);
		ch[idx[led-1]*3+1]=pgm_read_word(pwmtable_8+green);
		ch[idx[led-1]*3]=pgm_read_word(pwmtable_8+red);
	}
	else
	{
		for(uint8_t i = 0;i<16;i++)
		{
			ch[idx[i]*3+2]=pgm_read_word(pwmtable_8+blue);
			ch[idx[i]*3+1]=pgm_read_word(pwmtable_8+green);
			ch[idx[i]*3]=pgm_read_word(pwmtable_8+red);
		}

	}

}

void SetAllLeds(uint8_t frameBuffer[])
{
	for(uint8_t i = 0;i<16;i++)
	{
		ch[idx[i]*3]  =pgm_read_word(pwmtable_8 + frameBuffer[i*3]);
		ch[idx[i]*3+1]=pgm_read_word(pwmtable_8 + frameBuffer[i*3+1]);
		ch[idx[i]*3+2]=pgm_read_word(pwmtable_8 + frameBuffer[i*3+2]);
	}
}

void writeChannels(void)
{

	PORTD &= ~(1<<PORTD5);


	for(uint8_t i = 24;i>0;i--)
	{
		SPI_send(ch[i*2-1]>>4);
		SPI_send((ch[i*2-1]<<4)|(ch[i*2-2]>>8));
		SPI_send(ch[i*2-2]);
	}
	
	asm volatile("nop");
	asm volatile("nop");

	PORTD |= (1<<PORTD5);

	asm volatile("nop");
	asm volatile("nop");
	asm volatile("nop");
	asm volatile("nop");
	asm volatile("nop");
	asm volatile("nop");

	PORTD &= ~(1<<PORTD5);

}

void writeDC(void)
{

	PORTD |= (1<<PORTD5);

	//288 bit to write
	//216 bit real data
	//17 bit are shifted to some shift register (DCSIN/DCSOUT) ? why
	//lower 199 bit are latched to 216-Bit DC/BC/FC/UD Data Latch


	dcdata.dc00 = 127;
	dcdata.dc01 = 127;
	dcdata.dc02 = 127;
	dcdata.dc03 = 127;
	dcdata.dc04 = 127;
	dcdata.dc05 = 127;
	dcdata.dc06 = 127;
	dcdata.dc07 = 127;
	dcdata.dc08 = 127;
	dcdata.dc09 = 127;
	dcdata.dc10 = 127;
	dcdata.dc11 = 127;
	dcdata.dc12 = 127;
	dcdata.dc13 = 127;
	dcdata.dc14 = 127;
	dcdata.dc15 = 127;
	dcdata.dc16 = 127;
	dcdata.dc17 = 127;
	dcdata.dc18 = 127;
	dcdata.dc19 = 127;
	dcdata.dc20 = 127;
	dcdata.dc21 = 127;
	dcdata.dc22 = 127;
	dcdata.dc23 = 127;

	dcdata.gb_r = 255;
	dcdata.gb_g = 255;
	dcdata.gb_b = 255;

	dcdata.dc_range_r =1;
	dcdata.dc_range_g =1;
	dcdata.dc_range_b =1;
	dcdata.auto_repeat=1;
	dcdata.timing_mode=1;
	dcdata.counter_mode= 0;


	//B

	for(uint8_t i = 36;i>0;i--)
	{
		SPI_send(dcdata.dcbytes[i-1]);
	}


	for(uint8_t i = 36;i>0;i--)
	// A
	{
		SPI_send(dcdata.dcbytes[i-1]);
	}

	asm volatile("nop");
	asm volatile("nop");

	PORTD &= ~(1<<PORTD5);

	asm volatile("nop");
	asm volatile("nop");
	asm volatile("nop");
	asm volatile("nop");
	asm volatile("nop");
	asm volatile("nop");

	PORTD |= (1<<PORTD5);

}

