#include <stdbool.h>
#include <stdint.h>

enum DS18B20_DQ_LEVEL{
	DS18B20_DQ_HIGH = true,
	DS18B20_DQ_LOW = false
};

void ds18b20_Init(void);
char *ds18b20_get_temperature_string(void);