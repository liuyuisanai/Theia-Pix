#include <nuttx/config.h>

#include <string.h>
#include <unistd.h>

#include <debug.h>
#include <stm32_gpio.h>

#include <drivers/drv_lcd_st7565r.h>

#include "display.h"
#include "board_config.h"


FAR struct lcd_dev_s *dev;

static unsigned char volatile * volatile display_service_area = LCD_SERVICE_AREA;

void display_sleep();

#define CMD_SEND(x)      ( (*display_service_area) = ((unsigned char)x), display_sleep() )
#define DATA_SEND(x)      ( (*display_service_area) = ((unsigned char)x), display_sleep(), display_sleep() )
#define CMD_SEND2(a, b)  ( CMD_SEND(a), CMD_SEND(b) )


#define PAGE_COL_COUNT 128
#define PAGE_COUNT 8

typedef uint8_t Page[PAGE_COL_COUNT];

typedef struct
{
    Page page[PAGE_COUNT];
} Screen;

Screen screen;

void display_sleep()
{
    volatile int i = 0;
    (void)i;
}

void display_goto_page(int page, int column)
{
    stm32_gpiowrite(GPIO_LCD_A0, 0);
    // set page
    CMD_SEND((0xF & page) | 0xB0);
    // set collumn 0
    CMD_SEND(0x00 | (0xF & column));
    CMD_SEND(0x10 | ((0xF0 & column) >> 4));
}

void display_clear()
{
    memset(screen.page, 0, sizeof(screen.page));
    display_redraw_all();
}

void display_put_pixel(int x, int y)
{
    screen.page[y / 8][x] |= 1 << (y % 8);
}

void display_clear_pixel(int x, int y)
{
    screen.page[y / 8][x] &= ~(1 << (y % 8));
}

void display_draw_line(int x1, int y1, int x2, int y2)
{
    if (x1 > x2)
    {
        display_draw_line(x2, y2, x1, y1);
    }
    if (x1 == x2)
    {
        int y = 0;

        if (y1 > y2)
        {
            // swap y1 and y2
            y = y1;
            y1 = y2;
            y2 = y;
        }

        for (y = y1; y <= y2; y++)
        {
            display_put_pixel(x1, y);
        }
    }
    else
    {
        int dx = x2 - x1;
        int dy = y2 - y1;
        int x = 0;

        for (x = x1; x <= x2; x++)
        {
            int y = y1 + (int)((double)(dy * (x - x1)) / (double)dx);
            display_put_pixel(x, y);
        }
    }
}

void display_draw_rectangle(int x1, int y1, int x2, int y2)
{
    display_draw_line(x1, y1, x2, y1);
    display_draw_line(x2, y1, x2, y2);
    display_draw_line(x2, y2, x1, y2);
    display_draw_line(x1, y2, x1, y1);
}

void display_fill_rectangle(int x1, int y1, int x2, int y2)
{
    int x = 0;

    if (x1 > x2)
    {
        // swap y1 and y2
        x = x1;
        x1 = x2;
        x2 = x;
    }

    for (x = x1; x < x2; x++)
    {
        display_draw_line(x, y1, x, y2);
    }
}
void display_redraw_all()
{
    int pi = 0; // page index
    int ci = 0; // page column index
    //char cmd;

    for (pi = 0; pi < PAGE_COUNT; pi++) {
        display_goto_page(pi, 4);

        // send page data
        stm32_gpiowrite(GPIO_LCD_A0, 1);
        for (ci = 0; ci < PAGE_COL_COUNT; ci++) {
            DATA_SEND(screen.page[pi][ci]);
        }
    }
}

void display_bitmap(int x, int y, int width, int height, const unsigned char *data)
{
    int cx, cy;
    int value = 0;

    for (cy = 0; cy < height; cy++) {
        for (cx = 0; cx < width; cx++) {
            int index = cy * width + cx;

            value = data[index / 8] & (0x80 >> (index % 8));

            if (value)
            {
                display_put_pixel(x + cx, y + cy);
            }
            else
            {
                display_clear_pixel(x + cx, y + cy);
            }

        }
    }

}

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
        CMD_SEND(0xA1);         // Set required segment scan direction (normal / reversed)
        CMD_SEND(0xC0);         // Set required COM scan direction (normal / reversed)
        CMD_SEND(0x23);         // Set Rb/Ra ratio to 0x03 (K=4,5), so set V0 (contrast) adjustment range to 5,7 ... 9,5V
        CMD_SEND2(0x81, 0x3F);  // Set V0 volume to 0x3F (a=0) to set maximum display contrast
        CMD_SEND2(0xF8, 0x00);  // Set voltage buster ratio 2x-4x
        CMD_SEND(0x2F);         // Set internal power supply mode to 0x07 (Single power supply Vdd, all circuits enabled).
                                //     NOTE: From Reset release (2) state to power enable (10) must pass max. 5 msec

        CMD_SEND(0xA6);
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
