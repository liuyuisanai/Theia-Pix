#ifndef _I2C_DISPLAY_CONTROLLER_H
#define _I2C_DISPLAY_CONTROLLER_H

#include <drivers/device/i2c.h>
#include "common.h"

typedef enum {
	SYMBOL_0 = 0xFC,
	SYMBOL_1 = 0x60,
	SYMBOL_2 = 0xDA,
	SYMBOL_3 = 0xF2,
	SYMBOL_4 = 0x66,
	SYMBOL_5 = 0xB6,
	SYMBOL_6 = 0xBE,
	SYMBOL_7 = 0xE0,
	SYMBOL_8 = 0xFE,
	SYMBOL_9 = 0xF6,
	SYMBOL_A = 0xEE,
	SYMBOL_U = 0x7C,
	SYMBOL_Y = 0x76,
	SYMBOL_F = 0x8E,
	SYMBOL_P = 0xCE,
	SYMBOL_L = 0x1C,
	SYMBOL_B = 0x3E,
	SYMBOL_H = 0x6E,
	SYMBOL_E = 0x9E,
	SYMBOL_D = 0x7A,
	SYMBOL_C = 0x9C,
	SYMBOL_T = 0x8C,
	SYMBOL_R = 0x0A,
	SYMBOL_EMPTY = 0x00,
	SYMBOL_DOT = 0x01,
	SYMBOL_MINUS = 0x02
} symbol_t;


class __EXPORT I2C_DISPLAY_CONTROLLER : public device::I2C
{
public:
	I2C_DISPLAY_CONTROLLER(int bus, int addr);
	virtual ~I2C_DISPLAY_CONTROLLER();

	virtual int		init();
	virtual int		probe();
    int             set_symbols(uint8_t first, uint8_t second, uint8_t third);
    int             clear_display();
    void            set_symbols_from_int(int number);
    void            set_symbols_from_float(float number);
	void            set_symbols_from_str(const char str[3]);
	symbol_t        map_char_to_symbol(char c);

private:
	#include "asciisymbolmap.h"
};

#endif
