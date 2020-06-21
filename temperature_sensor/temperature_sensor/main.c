#define F_CPU 16000000UL

#include "ds18b20.h"
#include "ds18b20_USART.h"
#include "LED_display.h"

extern uint8_t display[4];

static void Send_str(char *s)
{
	while (*s != '\0'){
		USART_Transmit(*s++);
	}
	USART_Transmit('\n');
}

static void Display_Temperature(char *temperature)
{
	Display_Digit(0, temperature[2]);
	Display_Digit(1, temperature[3]);
	Display_Digit(2, temperature[5]);
	Display_Digit(3, temperature[6]);
}

ISR(TIMER1_COMPA_vect)
{
	static uint8_t n = 0;
	switch (n){
		case 0:
		SPI_MasterTransmit(display[0]);
		SPI_MasterTransmit(0x10);
		n = 1;
		break;
		case 1:
		SPI_MasterTransmit(display[1] & ~(0x01));
		SPI_MasterTransmit(0x20);
		n = 2;
		break;
		case 2:
		SPI_MasterTransmit(display[2]);
		SPI_MasterTransmit(0x40);
		n = 3;
		break;
		case 3:
		SPI_MasterTransmit(display[3]);
		SPI_MasterTransmit(0x80);
		n = 0;
		break;
		default:
		n = 0;
		break;
	}
	
	Display_Trigger();
}

int main(void)
{ 
	char *temperature;
		
	ds18b20_Init();
	USART_Init(MYUBRR);
	LED_Display_Init();
	
	sei();
	
    while (1) 
    {
		temperature = ds18b20_get_temperature_string();
		
		Send_str(temperature);
		
		Display_Temperature(temperature);
		
		//_delay_ms(3000);
		//USART_Transmit(T_HIGH);
		//USART_Transmit(T_LOW);
	}
}

