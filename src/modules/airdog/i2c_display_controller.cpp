#include "i2c_display_controller.h"

#include <nuttx/config.h>
#include <nuttx/clock.h>

#include <drivers/drv_hrt.h>

#include <sys/types.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>

#include <systemlib/perf_counter.h>
#include <systemlib/err.h>
#include <systemlib/systemlib.h>

#define I2C_BUTTON_COUNT 9
#define DISPLAY_PATH "/dev/display"
#define DISPLAY_NAME "display"
#define SET_MODE_CMD 0x4D
#define WRITE_DATA_CMD 0x00

I2C_DISPLAY_CONTROLLER::I2C_DISPLAY_CONTROLLER(int bus, int addr) :
    I2C(DISPLAY_NAME, DISPLAY_PATH, bus, addr, DEVICE_FREQUENCY)
{
}

I2C_DISPLAY_CONTROLLER::~I2C_DISPLAY_CONTROLLER()
{
}

int
I2C_DISPLAY_CONTROLLER::init()
{
	int ret;
	ret = I2C::init();

	if (ret != OK) {
		return ret;
	}

	return OK;
}

int
I2C_DISPLAY_CONTROLLER::probe()
{
    uint8_t requests[1] = {SET_MODE_CMD};

    int ret = transfer(&requests[0], sizeof(requests), nullptr, 0);
	return ret;
}

int
I2C_DISPLAY_CONTROLLER::set_symbols(uint8_t first, uint8_t second, uint8_t third)
{
    uint8_t requests[4] = {WRITE_DATA_CMD, first, second, third};
    return transfer(requests, sizeof(requests), nullptr, 0);
}

int
I2C_DISPLAY_CONTROLLER::clear_display()
{
    return set_symbols(SYMBOL_EMPTY, SYMBOL_EMPTY, SYMBOL_EMPTY);
}

void
I2C_DISPLAY_CONTROLLER::set_symbols_from_int(int number)
{
	symbol_t symbols[] = {
		map_char_to_symbol('0' + ((number / 100) % 10)),
		map_char_to_symbol('0' + ((number / 10) % 10)),
		map_char_to_symbol('0' + ((number) % 10))
	};
	for(int	i = 0; i < 2; i++) {
		if(symbols[i] == SYMBOL_0)
			symbols[i] = SYMBOL_EMPTY;
		else
			break;
	}
	set_symbols(symbols[0], symbols[1], symbols[2]);
}

void
I2C_DISPLAY_CONTROLLER::set_symbols_from_float(float number)
{
	char str[16];
	int i, j, len;

	len = sprintf(str, "%.2f", (double)number);

	uint8_t symbols[3] = {SYMBOL_EMPTY, SYMBOL_EMPTY,SYMBOL_EMPTY};

	for(i = 0, j = 0; i < len && j < 3; i++) {
		symbols[j] = map_char_to_symbol(str[i]);
		if(symbols[j] == SYMBOL_DOT) { //cannot be true when i == 0
			symbols[j - 1] |= SYMBOL_DOT;
		} else {
			j++;
		}
	}
	set_symbols(symbols[0], symbols[1], symbols[2]);
}

void
I2C_DISPLAY_CONTROLLER::set_symbols_from_str(const char str[3])
{
	set_symbols(
			map_char_to_symbol(str[0]),
			map_char_to_symbol(str[1]),
			map_char_to_symbol(str[2]));
}

symbol_t
I2C_DISPLAY_CONTROLLER::map_char_to_symbol(char c)
{
	return symbolAsciiMap[c];
}
