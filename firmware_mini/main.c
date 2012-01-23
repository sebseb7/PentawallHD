// links neue messung , rechts allte messung mit mehr puffer
// spanungen blau: 3.3-3.6 1800/4700 == 3,34(soll) 
// rot : 2.5-2.9 1500/4700 == 3,19(soll) (3,208 ist) (mit 1200 muesste es bis 3,03 runter gehen)
// gruen 3.5-3.9 2200/4700 == 3,55(soll)

//alternativ blau: 2200/4700 == 3,55
//alternativ gruen: 2700/4700 == 3,81


#include <inttypes.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "main.h"
#include "spi.h"
#include "usart.h"


// XLAT  == PD5
// SCLK == SCK/PB7
// SIN == MOSI/PB5
// SOUT == MISO/PB6




typedef void (*AppPtr_t)(void) __attribute__ ((noreturn)); 



#define MAX_NUMBER_OF_ANIMATIONS 10

uint8_t (*aniTick_fp[MAX_NUMBER_OF_ANIMATIONS])(void);
uint16_t aniInterval_count[MAX_NUMBER_OF_ANIMATIONS];
uint16_t aniInterval_duration[MAX_NUMBER_OF_ANIMATIONS];

uint8_t animations = 0;

void registerAnimation(uint8_t (*fp)(void),uint16_t tickInterval, uint16_t intervals)
{
	if(animations < MAX_NUMBER_OF_ANIMATIONS)
	{
		aniTick_fp[animations] = fp;
		aniInterval_count[animations]=intervals;
		aniInterval_duration[animations]=tickInterval;

		animations++;
	}
}





int main (void)
{
	// set mosi/sck out
	DDRB |= (1<<DDB5)|(1<<DDB7);

	// clock output (OC0A)
	DDRB |= (1<<DDB3);

	// latch aus
	PORTD &= ~(1<<PORTD5);
	// blank = low (all LEDs off)
	PORTD &= ~(1<<PORTD4);

	//latch out
	DDRD |= (1<<DDD5);
	//blank out
	DDRD |= (1<<DDD4);



	//SPI_Init()
	SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0);


	//enable pullups ununsed pins
	//	PORTD |= (1<<PORTD4);
	//	PORTD |= (1<<PORTD5);
	//	PORTD |= (1<<PORTD6);
	//	PORTB |= (1<<PORTB0);
		PORTB |= (1<<PORTB4);//pull slave select high
	//	PORTC |= (1<<PORTC4);
	//	PORTC |= (1<<PORTC5);

	//disable input buffers on unused pins
	//	DIDR1 |= (1<<AIN0D);
	//	DIDR0 |= (1<<ADC4D);
	//	DIDR0 |= (1<<ADC5D);

	//disable unused hardware (twi,adc,acd,timer0,timer2)
	//	PRR |= (1<<PRTWI)|(1<<PRADC)|(1<<PRTIM0)|(1<<PRTIM2);
	//	ACSR |= (1<<ACD); 


	//grayscale clock (mit ctc kommt man angeblich auf 10Mhz, momentan 5mhz)
	TCCR0A |= (1<<COM0A0)|(1<<WGM01)|(1<<WGM00);
	TCCR0B |= (1<<WGM02)|(1<<CS00);
	OCR0A = 1;

	//set defined values and enable it
	SetLed(0,0,0,0);
	flush();
	_delay_ms(1);
	writeDC();
	_delay_ms(1);
	// pull black to activate the led outputs
	PORTD |= (1<<PORTD4);


	//enable UART RX
	USART0_Init();

	//enable interrupts
	sei();


	uint8_t pixel_nr = 0;
	uint8_t pixel_r = 0;
	uint8_t pixel_g = 0;

	uint8_t data = 0;  
	uint8_t escape = 0;
	uint8_t idx = 0;
	uint8_t i = 0;


	

	while(1)
	{
		(*aniTick_fp[0])();
		flush();
		_delay_ms(33);
	}

	while(1)
	{
		i++;
		SetLed(1,i,0,0);
		SetLed(2,i+50,0,0);
		SetLed(3,i+100,0,0);
		SetLed(4,i+150,0,0);
		SetLed(5,0,i+150,0);
		SetLed(6,0,i,0);
		SetLed(7,0,i+100,0);
		SetLed(8,0,i+50,0);
		SetLed(9,0,0,i+100);
		SetLed(10,0,0,i+50);
		SetLed(11,0,0,i+150);
		SetLed(12,0,0,i);
		SetLed(13,i,i,i+100);
		SetLed(14,i,i+100,i);
		SetLed(15,i+100,i,i);
		SetLed(16,i+50,i,i+50);

		flush();
		_delay_ms(30);

		if(USART0_Getc_nb(&data))
		{
			break;		
		}
	}




	while(1)
	{
		if(USART0_Getc_nb(&data))
		{

			if(data == 0x42)
			{
				// sync
				idx = 0;
				continue;
			}
			else if(data == 0x65)
			{
				escape = 1;
				continue;
			}
			else if(data == 0x66)
			{
				// jump to bootloader
				GPIOR2=255;
				AppPtr_t AppStartPtr = (AppPtr_t)0x1800; 
				AppStartPtr();
			}


			if(escape == 1)
			{
				escape = 0;
				if(data == 0x02)
				{
					data = 0x42;
				}
				else if(data == 0x03)
				{
					data = 0x65;
				}
				else if(data == 0x04)
				{
					data = 0x66;
				}
			}


			if(idx == 0)
			{
				pixel_nr = data;
			}
			if(idx == 1)
			{
				pixel_r = data;
			}
			if(idx == 2)
			{
				pixel_g = data;
			}
			if(idx == 3)
			{
				SetLed(pixel_nr,pixel_r,pixel_g,data);
				flush();
			}
			idx++;

		}
	}
}


void setLedXY(uint8_t x ,uint8_t y,uint8_t r,uint8_t g,uint8_t b)
{

	SetLed(y*4+x+1,r,g,b);
}

