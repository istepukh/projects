#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdbool.h>

enum DS18B20_DQ_LEVEL{
	DS18B20_DQ_HIGH = true,
	DS18B20_DQ_LOW = false
};
	
void ds18b20_Init(void)
{
	DDRB &= ~(1 << DDB0); // as input 
	PORTB &= ~(1 << PB0); // set pin of port to low level
}

void ds18b20_Tx(bool level)
{
	if (level == 0) {
		// as output
		DDRB |= (1 << DDB0);
	} else {
		// as input
		DDRB &= ~(1 << DDB0);
	}
}

bool ds18b20_Rx(void)
{
	bool pin = PINB & (1 << PINB0);
	
	return pin;
}

bool ds18b20_InitizalizationSequence(void)
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

void WriteSlot(bool bit)
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

bool ReadSlot(void)
{
	ds18b20_Tx(DS18B20_DQ_LOW);
	_delay_us(5);
	ds18b20_Tx(DS18B20_DQ_HIGH);
	_delay_us(5);
	bool bit = ds18b20_Rx();
	_delay_us(70);
	
	return bit;
}

void ds18b20_WriteByte(uint8_t byte)
{
	bool bit;

	for (uint8_t i = 0; i < 8; i++){
		bit = byte & (1 << 0);
		byte = byte >> 1;
		
		WriteSlot(bit);
	}
	
}

uint8_t ds18b20_ReadByte(void)
{
	uint8_t byte = 0;

	for (uint8_t i = 0; i < 8; i++) {
		bool bit = ReadSlot();
		byte |= (bit << 0);
		byte = byte << 1; 
	}
	
	return byte;
}

int main(void)
{
	
	ds18b20_Init();
	
    while (1) 
    {
		ds18b20_InitizalizationSequence();
		ds18b20_WriteByte(0xCC);
		ds18b20_WriteByte(0x44);
		while (ReadSlot() == 0);
		ds18b20_WriteByte(0xCC);
		ds18b20_WriteByte(0xBE);
		//ds18b20_WriteByte(0xB8);
		//ds18b20_WriteByte(0xB4);
	}
}

