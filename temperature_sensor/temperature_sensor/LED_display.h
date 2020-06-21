#include <avr/interrupt.h>
#include <stdint.h>

uint8_t display[4];

void Display_Digit(uint8_t digit, char value);
void SPI_MasterTransmit(char cData);
void Display_Trigger(void);
void LED_Display_Init(void);