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
	
	/*reads and outputs the current level
	  how to read current level? what function to read?*/
	return pin;
}

bool ds18b20_InitizalizationSequence(void)
{
	bool level;
	ds18b20_Tx(DS18B20_DQ_LOW);
	_delay_us(480);
	ds18b20_Tx(DS18B20_DQ_HIGH);
	// how to use/implement pull up?
	_delay_us(70);
	level = ds18b20_Rx();
	_delay_us(410);
	
	return level;
}

int main(void)
{
	ds18b20_Init();
	
    while (1) 
    {
		ds18b20_InitizalizationSequence();
    }
}

