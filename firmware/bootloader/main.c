/*****************************************************************************
*
* Minimal Bootloader for Atmega644(p/a)
* 
*
* Based on:
*
* AVRPROG compatible boot-loader
* Version  : 0.85 (Dec. 2008)
* Compiler : avr-gcc 4.1.2 / avr-libc 1.4.6
* size     : depends on features and startup ( minmal features < 512 words)
* by       : Martin Thomas, Kaiserslautern, Germany
*            eversmith@heizung-thomas.de
*            Additional code and improvements contributed by:
*           - Uwe Bonnes
*           - Bjoern Riemer
*           - Olaf Rempel
*
* License  : Copyright (c) 2006-2008 M. Thomas, U. Bonnes, O. Rempel
*            Free to use. You have to mention the copyright
*            owners in source-code and documentation of derived
*            work. No warranty! (Yes, you can insert the BSD
*            license here)
*
******************************************************************************/

 
/* MCU frequency */
#ifndef F_CPU
#define F_CPU 20000000
#endif

/* UART Baudrate */
#define BAUDRATE 230400

/* use "Double Speed Operation" */
#define UART_DOUBLESPEED

/* use second UART on mega128 / can128 / mega162 / mega324p / mega644p */
//#define UART_USE_SECOND

/* Device-Type:
   For AVRProg the BOOT-option is prefered 
   which is the "correct" value for a bootloader.
   avrdude may only detect the part-code for ISP */
#define DEVTYPE     DEVTYPE_BOOT
// #define DEVTYPE     DEVTYPE_ISP

/*
 * Pin "STARTPIN" on port "STARTPORT" in this port has to grounded
 * (active low) to start the bootloader
 */
#define BLPORT		PORTD
#define BLDDR		DDRD
#define BLPIN		PIND
#define BLPNUM		PIND5

/*
 * Watchdog-reset is issued at exit 
 * define the timeout-value here (see avr-libc manual)
 */
#define EXIT_WDT_TIME   WDTO_250MS


/*
 * define the following if the bootloader should not output
 * itself at flash read (will fake an empty boot-section)
 */
#define READ_PROTECT_BOOTLOADER


#define VERSION_HIGH '0'
#define VERSION_LOW  '8'


#ifdef UART_DOUBLESPEED
// #define UART_CALC_BAUDRATE(baudRate) (((F_CPU*10UL) / ((baudRate) *8UL) +5)/10 -1)
#define UART_CALC_BAUDRATE(baudRate) ((uint32_t)((F_CPU) + ((uint32_t)baudRate * 4UL)) / ((uint32_t)(baudRate) * 8UL) - 1)
#else
// #define UART_CALC_BAUDRATE(baudRate) (((F_CPU*10UL) / ((baudRate)*16UL) +5)/10 -1)
#define UART_CALC_BAUDRATE(baudRate) ((uint32_t)((F_CPU) + ((uint32_t)baudRate * 8UL)) / ((uint32_t)(baudRate) * 16UL) - 1)
#endif


#include <stdint.h>
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/boot.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "chipdef.h"
#include "spi.h"

uint8_t gBuffer[SPM_PAGESIZE];

static void sendchar(uint8_t data)
{
	while (!(UART_STATUS & (1<<UART_TXREADY)));
	UART_DATA = data;
}

static uint8_t recvchar(void)
{
	while (!(UART_STATUS & (1<<UART_RXREADY)));
	return UART_DATA;
}

static inline void eraseFlash(void)
{
	// erase only main section (bootloader protection)
	uint32_t addr = 0;
	while (APP_END > addr) {
		boot_page_erase(addr);		// Perform page erase
		boot_spm_busy_wait();		// Wait until the memory is erased.
		addr += SPM_PAGESIZE;
	}
	boot_rww_enable();
}

static inline void recvBuffer(pagebuf_t size)
{
	pagebuf_t cnt;
	uint8_t *tmp = gBuffer;

	for (cnt = 0; cnt < sizeof(gBuffer); cnt++) {
		*tmp++ = (cnt < size) ? recvchar() : 0xFF;
	}
}

static inline uint16_t writeFlashPage(uint16_t waddr, pagebuf_t size)
{
	uint32_t pagestart = (uint32_t)waddr<<1;
	uint32_t baddr = pagestart;
	uint16_t data;
	uint8_t *tmp = gBuffer;

	do {
		data = *tmp++;
		data |= *tmp++ << 8;

		if ( baddr < APP_END ) 
		{
			boot_page_fill(baddr, data);	// call asm routine.
		}

		baddr += 2;			// Select next word in memory
		size -= 2;			// Reduce number of bytes to write by two
	} while (size);				// Loop until all bytes written

	boot_page_write(pagestart);
	boot_spm_busy_wait();
	boot_rww_enable();		// Re-enable the RWW section

	return baddr>>1;
}

static inline uint16_t writeEEpromPage(uint16_t address, pagebuf_t size)
{
	uint8_t *tmp = gBuffer;

	do {
		eeprom_write_byte( (uint8_t*)address, *tmp++ );
		address++;			// Select next byte
		size--;				// Decreas number of bytes to write
	} while (size);				// Loop until all bytes written

	// eeprom_busy_wait();

	return address;
}

static inline uint16_t readFlashPage(uint16_t waddr, pagebuf_t size)
{
	uint32_t baddr = (uint32_t)waddr<<1;
	uint16_t data;

	do {
#ifndef READ_PROTECT_BOOTLOADER
#warning "Bootloader not read-protected"
#if defined(RAMPZ)
		data = pgm_read_word_far(baddr);
#else
		data = pgm_read_word_near(baddr);
#endif
#else
		// don't read bootloader
		if ( baddr < APP_END ) {
#if defined(RAMPZ)
			data = pgm_read_word_far(baddr);
#else
			data = pgm_read_word_near(baddr);
#endif
		}
		else {
			data = 0xFFFF; // fake empty
		}
#endif
		sendchar(data);			// send LSB
		sendchar((data >> 8));		// send MSB
		baddr += 2;			// Select next word in memory
		size -= 2;			// Subtract two bytes from number of bytes to read
	} while (size);				// Repeat until block has been read

	return baddr>>1;
}

static inline uint16_t readEEpromPage(uint16_t address, pagebuf_t size)
{
	do {
		sendchar( eeprom_read_byte( (uint8_t*)address ) );
		address++;
		size--;				// Decrease number of bytes to read
	} while (size);				// Repeat until block has been read

	return address;
}


static void send_boot(void)
{
	sendchar('A');
	sendchar('V');
	sendchar('R');
	sendchar('B');
	sendchar('O');
	sendchar('O');
	sendchar('T');
}

static void (*jump_to_app)(void) = 0x0000;

int main(void)
{

#ifdef WDT_OFF_SPECIAL
#warning "using target specific watchdog_off"
    bootloader_wdt_off();
#else
	cli();
	wdt_reset();
	wdt_disable();
#endif

	uint16_t address = 0;
	uint8_t device = 0, val;

	BLDDR  &= ~(1<<BLPNUM);		// set as Input
	BLPORT |= (1<<BLPNUM);		// Enable pullup


    // set mosi/sck out
	DDRB = (1<<DDB5)|(1<<DDB3)|(1<<DDB2);
        
	// latch aus
	PORTB &= ~(1<<PORTB1);
	// blank = high (all off)
	PORTD |= (1<<PORTD7);

	//latch out
	DDRB |= (1<<DDB1);
	//blank out
	DDRD |= (1<<DDD7);
                        
                                        
	SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0);
                                            
	SetLed(0,0,0,0);
                                                
	// blank = low (all on)
	PORTD &= ~(1<<PORTD7);
                                                        


    _delay_ms(500);

	if ((BLPIN & (1<<BLPNUM)) && (GPIOR2 == 0) ) {

	    SetLed(0,10,10,0);
    	_delay_ms(500);

		// jump to main app if pin is not grounded and GPIOR2 is zero
		BLPORT &= ~(1<<BLPNUM);		// set to default		
		jump_to_app();			// Jump to application sector
	}
	
    SetLed(0,0,10,10);
	// Set baud rate
	UART_BAUD_HIGH = (UART_CALC_BAUDRATE(BAUDRATE)>>8) & 0xFF;
	UART_BAUD_LOW = (UART_CALC_BAUDRATE(BAUDRATE) & 0xFF);

#ifdef UART_DOUBLESPEED
	UART_STATUS = ( 1<<UART_DOUBLE );
#endif

	UART_CTRL = UART_CTRL_DATA;
	UART_CTRL2 = UART_CTRL2_DATA;

    SetLed(0,10,0,10);

	if(GPIOR2 != 0)
    {
		GPIOR2=0;
	}
   	_delay_ms(500);
                        

	for(;;) {
		val = recvchar();
		// Autoincrement?
		if (val == 'a') {
			sendchar('Y');			// Autoincrement is quicker

		//write address
		} else if (val == 'A') {
			address = recvchar();		//read address 8 MSB
			address = (address<<8) | recvchar();
			sendchar('\r');

		// Buffer load support
		} else if (val == 'b') {
			sendchar('Y');					// Report buffer load supported
			sendchar((sizeof(gBuffer) >> 8) & 0xFF);	// Report buffer size in bytes
			sendchar(sizeof(gBuffer) & 0xFF);

		// Start buffer load
		} else if (val == 'B') {
			pagebuf_t size;
			size = recvchar() << 8;				// Load high byte of buffersize
			size |= recvchar();				// Load low byte of buffersize
			val = recvchar();				// Load memory type ('E' or 'F')
			recvBuffer(size);

			if (device == DEVTYPE) {
				if (val == 'F') {
					address = writeFlashPage(address, size);
				} else if (val == 'E') {
					address = writeEEpromPage(address, size);
				}
				sendchar('\r');
			} else {
				sendchar(0);
			}

		// Block read
		} else if (val == 'g') {
			pagebuf_t size;
			size = recvchar() << 8;				// Load high byte of buffersize
			size |= recvchar();				// Load low byte of buffersize
			val = recvchar();				// Get memtype

			if (val == 'F') {
				address = readFlashPage(address, size);
			} else if (val == 'E') {
				address = readEEpromPage(address, size);
			}

		// Chip erase
 		} else if (val == 'e') {
			if (device == DEVTYPE) {
				eraseFlash();
			}
			sendchar('\r');

		// Exit upgrade
		} else if (val == 'E') {
			wdt_enable(EXIT_WDT_TIME); // Enable Watchdog Timer to give reset
			sendchar('\r');

		// Enter programming mode
		} else if (val == 'P') {
			sendchar('\r');

		// Leave programming mode
		} else if (val == 'L') {
			sendchar('\r');

		// return programmer type
		} else if (val == 'p') {
			sendchar('S');		// always serial programmer

		// Return device type
		} else if (val == 't') {
			sendchar(DEVTYPE);
			sendchar(0);

		// clear and set LED ignored
		} else if ((val == 'x') || (val == 'y')) {
			recvchar();
			sendchar('\r');

		// set device
		} else if (val == 'T') {
			device = recvchar();
			sendchar('\r');

		// Return software identifier
		} else if (val == 'S') {
			send_boot();

		// Return Software Version
		} else if (val == 'V') {
			sendchar(VERSION_HIGH);
			sendchar(VERSION_LOW);

		// Return Signature Bytes (it seems that 
		// AVRProg expects the "Atmel-byte" 0x1E last
		// but shows it first in the dialog-window)
		} else if (val == 's') {
			sendchar(SIG_BYTE3);
			sendchar(SIG_BYTE2);
			sendchar(SIG_BYTE1);

		/* ESC */
		} else if(val != 0x1b) {
			sendchar('?');
		}
	}
	return 0;
}
