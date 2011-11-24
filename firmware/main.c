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
uint8_t addr = 0;
uint8_t module_column = 0;
uint8_t module_row    = 0;

ISR (TIMER1_OVF_vect)
{
//	writeChannels();
	pushData = 1;
}

int main (void)
{
	//make sure D1 is input;
	DDRD &= ~(1<<DDD1);
	PORTD &= ~(1<<PORT1);

	// set mosi/sck out
	DDRB = (1<<DDB5)|(1<<DDB3)|(1<<DDB2);
	

	// latch aus
	PORTB &= ~(1<<PORTB1);
	// blank = high (all LEDs off)
	PORTD |= (1<<PORTD7);

	//latch out
	DDRB |= (1<<DDB1);
	//blank out
	DDRD |= (1<<DDD7);

	
	//SPI_Init()
	SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0);

	//fill the RAM of the TLC with defined values
	SetLed(0,0,0,0);
	writeChannels();

	_delay_ms(1);

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
	addr = ((~PINC) & 0x0F)|(((~PIND) & 0x0C)<<2);
	module_column = addr % (DISPLAY_WIDTH/4);
	module_row    = (addr - module_column) / (DISPLAY_HEIGHT/4);


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

	//make sure D1 is input;
	DDRD &= ~(1<<DDD1);
	PORTD &= ~(1<<PORT1);


	SetLed(0,0,0,0);
	writeChannels();writeChannels();


	// display Addr Info on startup
	for(uint8_t i = 0;i<8;i++)
	{
		if((addr & (1<<i))==(1<<i))
		{
			SetLed(i+1,0xa0,0,0);
		}
		if((module_row & (1<<i))==(1<<i))
		{
			SetLed(i+9,0,0xa0,0);
		}
		if((module_column & (1<<i))==(1<<i))
		{
			SetLed(i+13,0,0,0xa0);
		}
	}
	writeChannels();writeChannels();
	_delay_ms(0xaff);
	SetLed(0,0,0,0);
	writeChannels();writeChannels();


	//initialisation sequence

	for(uint8_t ax = 0;ax < DISPLAY_WIDTH;ax++)
	{
		for(uint8_t ay = 0;ay < DISPLAY_HEIGHT;ay++)
		{
			uint8_t	a_nr = pixelIsOurs(ax+1,ay+1);
			if(a_nr != 0)
			{
				SetLed(a_nr,100,0,0);
				writeChannels();
			}
			else
			{
				writeChannels();
			}
			_delay_ms(1);
		}
	}
	for(uint8_t ax = 0;ax < DISPLAY_WIDTH;ax++)
	{
		for(uint8_t ay = 0;ay < DISPLAY_HEIGHT;ay++)
		{
			uint8_t	a_nr = pixelIsOurs(ax+1,ay+1);
			if(a_nr != 0)
			{
				SetLed(a_nr,0,100,0);
				writeChannels();
			}
			else
			{
				writeChannels();
			}
			_delay_ms(1);
		}
	}
	for(uint8_t ax = 0;ax < DISPLAY_WIDTH;ax++)
	{
		for(uint8_t ay = 0;ay < DISPLAY_HEIGHT;ay++)
		{
			uint8_t	a_nr = pixelIsOurs(ax+1,ay+1);
			if(a_nr != 0)
			{
				SetLed(a_nr,0,0,100);
				writeChannels();
			}
			else
			{
				writeChannels();
			}
			_delay_ms(1);
		}
	}
	writeChannels();
	
	
	uint8_t pixel_x = 0;
	uint8_t pixel_y = 0;
	uint8_t pixel_r = 0;
	uint8_t pixel_g = 0;
	uint8_t pixel_b = 0;
	uint8_t pixel_nr = 0;
	
	uint8_t frameBuffer[16*3];
	for(uint8_t i = 0;i<(16*3);i++)
	{
		frameBuffer[i]=0;
	}
	
	uint8_t data = 0;  
	uint8_t state = 0;
	uint8_t escape = 0;
	uint8_t idx = 0;
	uint8_t color_state = 0;
	uint8_t x_state = 0;
	uint8_t y_state = 0;


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

				color_state = 0;
				x_state = 0;
				y_state = 0;
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
				idx++;
				
			}

			if(state == 2)
			{
				// wait for our part of the frame


				pixel_nr = pixelIsOurs(x_state+1,y_state+1);
				if(pixel_nr != 0)
				{
						frameBuffer[((pixel_nr-1)*3)+color_state] = data;
				}
				
				color_state++;
				if(color_state == 3) 
				{
					color_state = 0;
					y_state++;
				}
				if(y_state == DISPLAY_WIDTH)
				{
					y_state=0;
					x_state++;
				}
				if(x_state == DISPLAY_HEIGHT)
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
				else if(data == 0xfa)
				{
					UBRR0L = 0;
				}
				else if(data == 0xfe)
				{
					// jump to bootloader
					//GPIOR2=255;
					//AppPtr_t AppStartPtr = (AppPtr_t)0x1800; 
					//AppStartPtr();
				}
				else
				{
					//disable UART for a few seconds
			        UCSR0B &= ~(1 << RXCIE0);
				    UCSR0B &= ~(1 << RXEN0);
					SetLed(0,0,200,0);
					writeChannels();
					for(uint8_t i = 0;i < 16;i++)
					{
						_delay_ms(0x3ff);
						SetLed(i+1,0,0x50,0);
						writeChannels();
						_delay_ms(0x3ff);
						SetLed(i+1,0x50,0,0);
						writeChannels();
					}
					_delay_ms(0x1ff);
					SetLed(0,0,0,0);
				writeChannels();
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
			writeChannels();
		}
	}
}

//returns 0 if that pixel is not on out tile, otherwise LED number (1..16)
uint8_t pixelIsOurs(uint8_t x,uint8_t y)
{
	x--;
	y--;
	
	if( 
		(x >=  module_row      *5) && 
		(x <  (module_row+1)   *5) &&
		(y >=  module_column   *5) && 
		(y <  (module_column+1)*5)
	)
	{
		uint8_t row = x - module_row*5;
		uint8_t col = y - module_column*5;
	
		
	
		return row*5+col+1;
	} 

	return 0;
}
