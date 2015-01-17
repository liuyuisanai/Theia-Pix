#include <nuttx/config.h>

#ifndef CONFIG_ARCH_CHIP_STM32
# error Only STM32 supported.
#endif

#include <cstdint>
#include <cstdio>

#include <stm32.h>
#include <stm32_gpio.h>

#include <board_leds.h>
#include <drivers/drv_led_config.h>

#include "leds.hpp"

__BEGIN_DECLS
extern void led_init(void);
__END_DECLS

namespace indication { namespace leds {

struct State
{
	uint32_t pattern_play_once;
	uint32_t pattern_repeat;
	uint32_t repeat_phase;

	State() { reset(); }

	inline void
	reset()
	{
		pattern_play_once = 0;
		pattern_repeat = 0;
		repeat_phase = 0;
	}
};

volatile State state[N_LEDS];

void set_default()
{ led_init(); }

void set_pattern_once(unsigned led, uint32_t pattern)
{
	if (led < N_LEDS)
	{
		state[led].pattern_play_once = pattern;
		state[led].repeat_phase = state[led].pattern_repeat;
	}
}

void set_pattern_repeat(unsigned led, uint32_t pattern)
{
	if (led < N_LEDS)
	{
		state[led].pattern_repeat = pattern;
		state[led].repeat_phase = state[led].pattern_repeat;
	}
}

void
update()
{
	for (size_t i=0; i < N_LEDS; ++i)
	{
		bool next_bit;

		if (state[i].repeat_phase == 0)
			state[i].repeat_phase = state[i].pattern_repeat;

		next_bit = state[i].repeat_phase & 1;
		state[i].repeat_phase >>= 1;

		if (state[i].pattern_play_once != 0)
		{
			// play once overrides repeat.
			next_bit = state[i].pattern_play_once & 1;
			state[i].pattern_play_once >>= 1;
		}

		bool on = next_bit ? LED_GPIO_ON : LED_GPIO_OFF;
		if (on != stm32_gpioread(led_config[i]))
			stm32_gpiowrite(led_config[i], on);
	}
}

void
status()
{
	for (size_t i=0; i < N_LEDS; ++i)
	{
		printf("LED %i: repeat 0x%08x, once 0x%08x.\n",
				i,
				state[i].pattern_repeat,
				state[i].pattern_play_once);
	}

}

}} // end of namespace indication::leds
