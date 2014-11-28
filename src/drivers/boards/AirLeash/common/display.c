#include <nuttx/config.h>

#include <string.h>
#include <unistd.h>

#include <debug.h>
#include <stm32_gpio.h>

#include <drivers/drv_lcd_st7565r.h>

#include "display.h"
#include "board_config.h"

FAR struct lcd_dev_s *dev;

#define CMD_SEND(x)      ( (*(volatile char *)LCD_SERVICE_AREA) = (x) )
#define CMD_SEND2(a, b)  ( CMD_SEND(a), CMD_SEND(b) )

void
up_display_mcu_setup(void)
{
	stm32_configgpio(GPIO_LCD_A0);
	stm32_configgpio(GPIO_LCD_RESET);
	stm32_gpiowrite(GPIO_LCD_RESET, 0);
	up_display_fmc();
}

void
display_test_nop(size_t n)
{
	for(size_t i = 0; i < n; ++i)
		(*(volatile char *)LCD_SERVICE_AREA)= 0xe3;
}

void
display_turn_on(void)
{
	fmc_enable();

	usleep(/* 100ms */ 100000);
	stm32_gpiowrite(GPIO_LCD_RESET, 1);
	usleep(/* 1ms */ 1000);
}

void
display_turn_off(void)
{
	usleep(/* 100ms */ 100000);
	stm32_gpiowrite(GPIO_LCD_RESET, 0);

	fmc_disable();
}

void
display_init(void)
{
	display_turn_on();

	CMD_SEND(0xA2);         // Set BIAS to 1/9 as required for this LCD
	CMD_SEND(0xA0);         // Set required segment scan direction (normal / reversed)
	CMD_SEND(0xC0);         // Set required COM scan direction (normal / reversed)
	CMD_SEND(0x23);         // Set Rb/Ra ratio to 0x03 (K=4,5), so set V0 (contrast) adjustment range to 5,7 ... 9,5V
	CMD_SEND2(0x81, 0x3F);  // Set V0 volume to 0x3F (a=0) to set maximum display contrast
	CMD_SEND2(0xF8, 0x00);  // Set voltage buster ratio 2x-4x
	CMD_SEND(0x2F);         // Set internal power supply mode to 0x07 (Single power supply Vdd, all circuits enabled).
	                        //     NOTE: From Reset release (2) state to power enable (10) must pass max. 5 msec

	CMD_SEND(0x40);	        // Set display Start line to 0
	CMD_SEND(0xAF);	        // Display ON
}

void
display_shutdown(void)
{
	display_enter_powersave();
	display_turn_off();
}

void
display_enter_powersave(void)
{
	CMD_SEND2(0xAC, 0x00);  // Static indicator OFF
	CMD_SEND(0xA5);         // Display All Points On Mode enable
	CMD_SEND(0xAE);         // Display OFF
}

void
display_exit_powersave(void)
{
	CMD_SEND(0xAF);         // Display ON
	CMD_SEND2(0xAC, 0x00);  // Static indicator OFF
}


/****************************************************************************
 * Name: up_lcdgetdev
 ****************************************************************************/

FAR struct lcd_dev_s *up_lcdgetdev(int lcddev)
{
  dev = st7565r_initialize();
  if (!dev)
    {
      glldbg("Failed to bind SSI port 0 to OLCD %d: %d\n", lcddev);
    }
  else
    {
      gllvdbg("Bound SSI port 0 to OLCD %d\n", lcddev);

      /* And turn the OLCD on (CONFIG_LCD_MAXPOWER should be 1) */
      (void)dev->setpower(dev, CONFIG_LCD_MAXPOWER);
      return dev;
    }

  return NULL;
}


#ifndef CONFIG_NX_LCDDRIVER
# error Only CONFIG_NX_LCDDRIVER supported.
#endif

FAR struct lcd_dev_s *up_nxdrvinit(unsigned int devno)
{ return up_lcdgetdev(devno); }
