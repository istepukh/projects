#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <string.h>

#include "ds18b20.h"

#define DS18B20_SKIP_ROM_COMMAND 0xCC
#define DS18B20_CONVERT_TEMPERATURE 0x44
#define DS18B20_COPY_SCRATCHPAD 0xBE

static char arr_str[16][5] = {
	"0000",
	"0625",
	"1250",
	"1875",
	"2500",
	"3125",
	"3750",
	"4375",
	"5000",
	"5625",
	"6250",
	"6875",
	"7500",
	"8125",
	"8750",
	"9375"
};

static void ds18b20_Tx(bool level)
{
	if (level == 0) {
		// as output
		DDRB |= (1 << DDB0);
	} else {
		// as input
		DDRB &= ~(1 << DDB0);
	}
}

static bool ds18b20_Rx(void)
{
	bool pin = PINB & (1 << PINB0);
	
	return pin;
}

static void WriteSlot(bool bit)
{
	if (bit == 1) {
		// WriteSlot(1);
		ds18b20_Tx(DS18B20_DQ_LOW);
		_delay_us(5);
		ds18b20_Tx(DS18B20_DQ_HIGH);
		_delay_us(70);
		} else {
		// WriteSlot(0);
		ds18b20_Tx(DS18B20_DQ_LOW);
		_delay_us(70);
		ds18b20_Tx(DS18B20_DQ_HIGH);
		
	}
}

static bool ds18b20_InitizalizationSequence(void)
{
	bool level;
	ds18b20_Tx(DS18B20_DQ_LOW);
	_delay_us(480);
	ds18b20_Tx(DS18B20_DQ_HIGH);
	// implementation of pull up
	_delay_us(70);
	level = ds18b20_Rx();
	_delay_us(410);
	
	return level;
}

static bool ReadSlot(void)
{
	ds18b20_Tx(DS18B20_DQ_LOW);
	_delay_us(5);
	ds18b20_Tx(DS18B20_DQ_HIGH);
	_delay_us(5);
	bool bit = ds18b20_Rx();
	_delay_us(70);
	
	return bit;
}

static void ds18b20_WriteByte(uint8_t byte)
{
	bool bit;

	for (uint8_t i = 0; i < 8; i++){
		bit = byte & (1 << 0);
		byte = byte >> 1;
		
		WriteSlot(bit);
	}
}

static uint8_t ds18b20_ReadByte(void)
{
	uint8_t byte = 0;

	for (uint8_t i = 0; i < 8; i++) {
		bool bit = ReadSlot();
		byte = byte >> 1;
		byte |= (bit << 7);
	}
	
	return byte;
}

void ds18b20_Init(void)
{
	DDRB &= ~(1 << DDB0); // as input
	PORTB &= ~(1 << PB0); // set pin of port to low level
}


char *ds18b20_get_temperature_string(void)
{
	int16_t temperature = 0;
	static char temperature_str[10] = "         ";
	  
	ds18b20_InitizalizationSequence();
	ds18b20_WriteByte(DS18B20_SKIP_ROM_COMMAND);
	ds18b20_WriteByte(DS18B20_CONVERT_TEMPERATURE);
	while (ReadSlot() == 0);

	ds18b20_InitizalizationSequence();
	ds18b20_WriteByte(DS18B20_SKIP_ROM_COMMAND);
	ds18b20_WriteByte(DS18B20_COPY_SCRATCHPAD);
	
	temperature |= ds18b20_ReadByte() << 0;
	temperature |= ds18b20_ReadByte() << 8;	
	
	temperature_str[0] = temperature < 0 ? '-' : '+';
	
	itoa((temperature / 16), ((temperature / 16) < 100) ? &temperature_str[2] : &temperature_str[1], 10);
	temperature_str[4] = '.';
	strcpy(&temperature_str[5], &arr_str[temperature & 15][0]);
	
	return temperature_str;	
};