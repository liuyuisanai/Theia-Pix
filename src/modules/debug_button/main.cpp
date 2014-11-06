extern "C" __EXPORT int main(int, const char * const * const);

#include <nuttx/config.h>
#include <board_config.h>

#include <stm32_gpio.h>

#include <stdio.h>

int
main(int, const char * const * const)
{
	bool active = stm32_gpioread(GPIO_DEBUG_BTN);
#if DEBUG_BTN_ACTIVE_LOW
	active = not active;
#endif
	printf("Debug button %s.\n", active ? "pressed" : "was not pressed.");
	return active ? 0 : 1;
}
