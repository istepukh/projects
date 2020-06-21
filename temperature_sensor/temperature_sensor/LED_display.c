#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>

#include "LED_display.h"

void Display_Trigger(void)
{
	PORTB |= (1 << PB1);
	PORTB &= ~(1 << PB1);
}

void SPI_MasterTransmit(char cData)
{
	SPDR = cData; // Start transmission
	while(!(SPSR & (1 << SPIF))); // Wait for transmission complete
}

static void Display_Init(void)
{
	PORTB &= ~(1 << PB1);
	DDRB |= (1 << DDB1);
}

static void SPI_MasterInit(void)
{
	DDRB = (1 << DDB3) | (1 << DDB5) | (1 << DDB2); // Set MOSI and SCK output, all others input
	SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR0) | (1 << DORD); // Enable SPI, Master, set clock rate fck/16
}

static void Timer_Init(void)
{
	TCCR1B = (1 << CS11) | (1 << WGM12);
	TIMSK1 |= (1 << OCIE1A) ;
	OCR1A = ( F_CPU / (500 * 8) - 1);
}

void Display_Digit(uint8_t digit, char value)
{
	switch (value - '0'){
		case 0:
		display[digit] = 0x03;
		break;
		case 1:
		display[digit] = 0x9F;
		break;
		case 2:
		display[digit] = 0x25;
		break;
		case 3:
		display[digit] = 0x0D;
		break;
		case 4:
		display[digit] = 0x99;
		break;
		case 5:
		display[digit] = 0x49;
		break;
		case 6:
		display[digit] = 0x41;
		break;
		case 7:
		display[digit] = 0x1F;
		break;
		case 8:
		display[digit] = 0x01;
		break;
		case 9:
		display[digit] = 0x09;
		break;
		default:
		break;
	}
}

void LED_Display_Init(void)
{
	SPI_MasterInit();
	Display_Init();
	Timer_Init();
}