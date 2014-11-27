#include <nuttx/config.h>

#include <drivers/drv_airleash.h>

#include "board_config.h"

__EXPORT void
halt_and_power_off()
{
	stm32_gpiowrite(GPIO_VDD_SYSPOWER_OFF, 1);
	// It will turn off after 20ms.
}
