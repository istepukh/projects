#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdbool.h>
#include <stdlib.h>

#define BAUD 9600
#define MYUBRR F_CPU/16/BAUD-1
#define SKIP_ROM_COMMAND 0xCC
#define CONVERT_TEMPERATURE 0x44
#define COPY_SCRATCHPAD 0xBE

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
		byte = byte >> 1;
		byte |= (bit << 7);
	}
	
	return byte;
}

unsigned char USART_Receive(void)
{
	/* Wait for data to be received */
	while (!(UCSR0A & (1<<RXC0)))
	;
	/* Get and return received data from buffer */
	return UDR0;
}

void USART_Transmit(unsigned char data)
{
	/* Wait for empty transmit buffer */
	while (!( UCSR0A & (1<<UDRE0)))
	;
	/* Put data into buffer, sends the data */
	UDR0 = data;
}

void USART_Init(unsigned int ubrr)
{
	/*Set baud rate */
	UBRR0H = (unsigned char)(ubrr>>8);
	UBRR0L = (unsigned char)ubrr;
	/*Enable receiver and transmitter */
	UCSR0B = (1<<RXEN0)|(1<<TXEN0);
	/* Set frame format: 8data, 2stop bit */
	UCSR0C = (3<<UCSZ00);
}

void Send_str(char *s)
{
	while (*s != '\0'){
		USART_Transmit(*s++);
	}
}

char arr_str[16][5] = {
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

void SPI_MasterInit(void)
{
	DDRB = (1 << DDB3) | (1 << DDB5) | (1 << DDB2); // ????????? MOSI ? SCK ??? ?????, ??? ????????? ??????? ??? ???? 
	SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR0) | (1 << DORD); // ????????? ?????? SPI, ????? Master, ?????????? ???????? ?????? fck/16 
}

void SPI_MasterTransmit(char cData)
{
	SPDR = cData; // ?????? ???????? ?????? 
	while(!(SPSR & (1 << SPIF))); // ???????? ?????????? ???????? 
}

void SPI_SlaveInit(void)
{
	DDRB = (1 << DDB4); // ????????? ????? ????? MISO ??? ?????, ??? ????????? ??????? ??? ???? 
	SPCR = (1 << SPE); // ????????? ?????? SPI
}

char SPI_SlaveReceive(void)
{
	while(!(SPSR & (1 << SPIF))); // ???????? ?????????? ??????
	
	return SPDR; // ?????? ???????? ?????? ? ???????
}

int main(void)
{
	int16_t temperature;
	char temperature_str[10];
	char point[2] = ".";
	char sign[2] = "+"; 
	ds18b20_Init();
	USART_Init(MYUBRR);
	SPI_MasterInit();

    while (1) 
    {
		//USART_Transmit(USART_Receive());
		ds18b20_InitizalizationSequence();
		ds18b20_WriteByte(SKIP_ROM_COMMAND);
		ds18b20_WriteByte(CONVERT_TEMPERATURE);
		while (ReadSlot() == 0);

		ds18b20_InitizalizationSequence();
		ds18b20_WriteByte(SKIP_ROM_COMMAND);
		ds18b20_WriteByte(COPY_SCRATCHPAD);

		uint8_t T_LOW = ds18b20_ReadByte();
		uint8_t T_HIGH = ds18b20_ReadByte();
		temperature = (T_HIGH << 8) | T_LOW;

		itoa((temperature / 16), temperature_str, 10);
		Send_str(sign);
		Send_str(temperature_str);
		Send_str(point);

		Send_str(&arr_str[T_LOW & 15][0]);
		USART_Transmit('\n');
		SPI_MasterTransmit(0x80);
		SPI_MasterTransmit(0x60);
		SPI_MasterTransmit(0x40);
		SPI_MasterTransmit(0xDA);
		SPI_MasterTransmit(0x20);
		SPI_MasterTransmit(0xF2);
		SPI_MasterTransmit(0x10);
		SPI_MasterTransmit(0x66);
		//_delay_ms(3000);
		//USART_Transmit(T_HIGH);
		//USART_Transmit(T_LOW);
	}
}

