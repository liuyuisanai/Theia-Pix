#pragma once

#include <stm32_gpio.h>
#include <up_arch.h>

using reg_value_t = uint32_t;

/****************************************************************************
 * Name: stm32_gpioread_port
 *
 * Description:
 *   Read GPIO input port value, all pins at once.
 *
 ****************************************************************************/
inline reg_value_t
stm32_gpioread_port(uint32_t port)
{
	port >>= GPIO_PORT_SHIFT;
	if (port < STM32_NGPIO_PORTS)
	{
		uint32_t p = g_gpiobase[port];
		p += STM32_GPIO_IDR_OFFSET;
		return getreg32(p);
	}
	return 0;
}

/**
 * GPIO single pin bit shift in the register.
 * \param pin_index is GPIO_PINx constant.
 */
static constexpr int
gpio_reg_pin_shift(int pin_index)
{ return (pin_index & GPIO_PIN_MASK) >> GPIO_PIN_SHIFT; }

/**@{*/
/**
 * \fn reg_value_t gpio_reg_pin_mask<GPIO_PINx, ...>::value
 * \brief Compose bit mask of separate pin indexes.
 * \param pin_index list of GPIO_PINx constant.
 */

template<int pin, int ... other_pins>
struct gpio_reg_pin_mask
{
	static constexpr reg_value_t value =
		gpio_reg_pin_mask<pin>::value |
		gpio_reg_pin_mask<other_pins...>::value;
};

template<int pin>
struct gpio_reg_pin_mask<pin>
{
	static constexpr reg_value_t value = 1 << gpio_reg_pin_shift(pin);
	static_assert(value != 0, "reg_value_t is too narrow.");
};

/**@}*/
