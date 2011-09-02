// spanungen blau: 3.6
// rot : 2.9
// gruen 3.9

#include <inttypes.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "main.h"
#include "spi.h"
#include "usart.h"


//compute our position from this using addr
#define DISPLAY_WIDTH 24
#define DISPLAY_HEIGHT 24
                                                                                                                                 // BLANC == PB2
// XLAT  == PB1
// SCLK == SCK/PB5
// SIN == MOSI/PB3
// SOUT == MISO/PB4


typedef void (*AppPtr_t)(void) __attribute__ ((noreturn)); 

uint8_t pixelIsOurs(uint8_t,uint8_t);
uint8_t volatile pushData = 0;


ISR (TIMER1_OVF_vect)
{
//	writeChannels();
	pushData = 1;
}

int main (void)
{
	// set mosi/sck out
	DDRB = (1<<DDB5)|(1<<DDB3)|(1<<DDB2);
	
	//latch out
	DDRB |= (1<<DDB1);
	//blank out
	DDRD |= (1<<DDD7);

	// latch aus
	PORTB &= ~(1<<PORTB1);
	// blank = high (all LEDs off)
	PORTD |= (1<<PORTD7);
	
	//SPI_Init()
	SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0);

	//fill the RAM of the TLC with defined values
	SetLed(0,0,0,0);
	writeChannels();

	// blank = low (enable LEDs)
	PORTD &= ~(1<<PORTD7);
	
	DDRC &= ~(1<<PORTC0);
	DDRC &= ~(1<<PORTC1);
	DDRC &= ~(1<<PORTC2);
	DDRC &= ~(1<<PORTC3);
	DDRD &= ~(1<<PORTD2);
	DDRD &= ~(1<<PORTD3);
	PORTC |= (1<<PORTC0)|(1<<PORTC1)|(1<<PORTC2)|(1<<PORTC3);
	PORTD |= (1<<PORTD2)|(1<<PORTD3);
	// wait for the pin to synchronize
	_delay_ms(1);
	uint8_t addr = ((~PINC) & 0x0F)|(((~PIND) & 0x0C)<<4);


	//enable pullups ununsed pins
	PORTD |= (1<<PORTD4);
	PORTD |= (1<<PORTD5);
	PORTD |= (1<<PORTD6);
	PORTB |= (1<<PORTB0);
	PORTB |= (1<<PORTB2);
	PORTC |= (1<<PORTC4);
	PORTC |= (1<<PORTC5);
	
	//disable input buffers on unused pins
	DIDR1 |= (1<<AIN0D);
	DIDR0 |= (1<<ADC4D);
	DIDR0 |= (1<<ADC5D);
	
	//disable unused hardware (twi,adc,acd,timer0,timer2)
	PRR |= (1<<PRTWI)|(1<<PRADC)|(1<<PRTIM0)|(1<<PRTIM2);
	ACSR |= (1<<ACD); 

	//timer1 for tlc sync

	//set to FastPWM Mode & prescaler 8
	TCCR1A |= (1<<WGM10)|(1<<WGM11);
	TCCR1B |= (1<<WGM12)|(1<<WGM13)|(1<<CS11);//|(1<<CS10);
	//this is one cycle length of the TLC (2560)
	OCR1A = 2780; //2780 looks good
	//enable interrupt
	TIMSK1 |= (1<<TOIE1);
	


	//enable UART RX
	USART0_Init();
	
	//enable interrupts
	sei();


	//dummy initialization code (needs to be replaced)
	for(uint8_t i = 1;i<17;i++)
	{
		SetLed(i,90,0,0); 
		writeChannels();
		_delay_ms(10);
	}
	for(uint8_t i = 1;i<17;i++)
	{
		SetLed(i,0,90,0); 
		writeChannels();
		_delay_ms(10);
	}
	for(uint8_t i = 1;i<17;i++)
	{
		SetLed(i,0,0,90); 
		writeChannels();
		_delay_ms(10);
	}
	for(uint8_t i = 1;i<17;i++)
	{
		SetLed(i,0,0,0);
		writeChannels();
		_delay_ms(10);
	}

	SetLed(0,0,0,55);
	writeChannels();
	
	uint8_t pixel_x = 0;
	uint8_t pixel_y = 0;
	uint8_t pixel_r = 0;
	uint8_t pixel_g = 0;
	uint8_t pixel_b = 0;
	uint8_t pixel_nr = 0;
	
	uint8_t frameBuffer[16*3];
	
	uint8_t data = 0;  
	uint8_t state = 0;
	uint8_t escape = 0;
	uint16_t idx = 0;

	while(1)
	{
		if(USART0_Getc_nb(&data))
		{

			if(data == 0x42)
			{
				// single pixel
				state = 1;
				idx = 0;
				continue;
			}
			if(data == 0x23)
			{
				// full frame
				state = 2;
				idx = 0;
				continue;
			}
			if(data == 0x65)
			{
				escape = 1;
				continue;
			}
			if(data == 0x66)
			{
				// bootloader
				state = 3;
				continue;
			}
			if(escape == 1)
			{
				escape = 0;
				if(data == 0x01)
				{
					data = 0x23;
				}
				else if(data == 0x02)
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
			
		
			if(state == 1)
			{
				// wait for our pixel
				if(idx == 0)
				{
					pixel_x = data;
				}
				if(idx == 1)
				{
					pixel_y = data;
				}
				if(idx == 2)
				{
					pixel_r = data;
				}
				if(idx == 3)
				{
					pixel_g = data;
				}
				if(idx == 4)
				{
					pixel_b = data;
				}
				idx++;
				
				if((pixel_x == 0) && (pixel_y == 0))
				{
					SetLed(0,pixel_r,pixel_g,pixel_b);
				}
				else
				{
					pixel_nr = pixelIsOurs(pixel_x,pixel_y);
					if(pixel_nr != 0)
					{
						SetLed(pixel_nr,pixel_r,pixel_g,pixel_b);
					}
				}
			}

			if(state == 2)
			{
				// wait for our part of the frame

				//fill frameBuffer[]

				if(idx == (DISPLAY_HEIGHT*DISPLAY_WIDTH))
				{
					SetAllLeds(frameBuffer);
				}
			}

			if(state == 3)
			{
				if(data == 0xff)
				{
					// get addr
					// display addr on LEDs
					SetLed(0,0,0,0);
					for(uint8_t i = 0;i<8;i++)
					{
						if((addr & (1<<i))==(1<<i))
						{
							SetLed(i+1,0xa0,0,0);
						}
					}
				}
				else if(data == addr)
				{
					// jump to bootloader
					GPIOR2=255;
					AppPtr_t AppStartPtr = (AppPtr_t)0x1800; 
					AppStartPtr();
				}
				else
				{
					//disable UART for a few seconds
			        UCSR0B &= ~(1 << RXCIE0);
				    UCSR0B &= ~(1 << RXEN0);
					SetLed(0,0,0,0);
					for(uint8_t i = 0;i < 16;i++)
					{
						_delay_ms(0x1ff);
						SetLed(i+1,0,0x50,0);
					}
					_delay_ms(0x1ff);
					SetLed(0,0,0,0);
				    UCSR0B |= (1 << RXEN0);
			        UCSR0B |= (1 << RXCIE0);
					// sleep for bootloader of differend device display progress on LEDs
				}
				state = 0;
			}

		}
		if(pushData == 1)
		{
			pushData = 0;
//			cli();
			writeChannels();
//			sei();
		}
	}
}

//returns 0 of that pixel is not on out tile, otherwise LED number (1..16)
uint8_t pixelIsOurs(uint8_t x,uint8_t y)
{
	return 0;
}
