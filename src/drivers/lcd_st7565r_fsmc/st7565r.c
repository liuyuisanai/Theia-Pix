/**************************************************************************************
 * drivers/lcd/st7567.c
 *
 * Driver for the TM12864J1CCWGWA Display with the ST7567 LCD
 * controller.
 *
 *   Copyright (C) 2013 Zilogic Systems. All rights reserved.
 *   Author: Manikandan <code@zilogic.com>
 *
 * Based on drivers/lcd/ug-9664hswag01.c
 *
 *   Copyright (C) 2011 Gregory Nutt. All rights reserved.
 *   Author: Gregory Nutt <gnutt@nuttx.org>
 *
 * Reference: "Product Specification, OEL Display Module, ST7567", Univision
 *            Technology Inc., SAS1-6020-B, January 3, 2008.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name NuttX nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 **************************************************************************************/

/**************************************************************************************
 * Included Files
 **************************************************************************************/

#include <nuttx/config.h>

#include <sys/types.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <debug.h>

#include <nuttx/arch.h>
#include <nuttx/lcd/lcd.h>

#include <board_config.h>
#include <display.h>
#include <drivers/drv_lcd_st7565r.h>

#include "st7567_defs.h"

/**************************************************************************************
 * Pre-processor Definitions
 **************************************************************************************/

/* Configuration **********************************************************************/
/* ST7567 Configuration Settings:
 *
 * CONFIG_ST7567_NINTERFACES - Specifies the number of physical
 *   ST7567 devices that will be supported.  NOTE:  At present, this
 *   must be undefined or defined to be 1.
 * CONFIG_ST7567_POWER
 *   If the hardware supports a controllable OLED a power supply, this
 *   configuration shold be defined.  (See st7565r_power() below).
 * CONFIG_LCD_ST7565R_DEBUG - Enable detailed ST7567 debst7567 output
 *   (CONFIG_DEBUG and CONFIG_VERBOSE must also be enabled).
 *
 * Required LCD driver settings:
 * CONFIG_LCD_ST7567 - Enable ST7567 support
 * CONFIG_LCD_MAXCONTRAST should be 255, but any value >0 and <=255 will be accepted.
 * CONFIG_LCD_MAXPOWER should be 2:  0=off, 1=dim, 2=normal
 */

/* Verify that all configuration requirements have been met */

/* CONFIG_ST7567_NINTERFACES determines the number of physical interfaces
 * that will be supported.
 */

#ifndef CONFIG_ST7567_NINTERFACES
#  define CONFIG_ST7567_NINTERFACES 1
#endif

#if CONFIG_ST7567_NINTERFACES != 1
#  warning "Only a single ST7567 interface is supported"
#  undef CONFIG_ST7567_NINTERFACES
#  define CONFIG_ST7567_NINTERFACES 1
#endif

/* Verbose debst7567 must also be enabled to use the extra OLED debst7567 */

#ifndef CONFIG_DEBUG
#  undef CONFIG_DEBUG_VERBOSE
#  undef CONFIG_DEBUG_GRAPHICS
#endif

#ifndef CONFIG_DEBUG_VERBOSE
#  undef CONFIG_LCD_ST7565R_DEBUG
#endif

/* Check contrast selection */

#ifndef CONFIG_LCD_MAXCONTRAST
#  define CONFIG_LCD_MAXCONTRAST 255
#endif

#if CONFIG_LCD_MAXCONTRAST <= 0 || CONFIG_LCD_MAXCONTRAST > 255
#  error "CONFIG_LCD_MAXCONTRAST exceeds supported maximum"
#endif

#if CONFIG_LCD_MAXCONTRAST < 255
#  warning "Optimal setting of CONFIG_LCD_MAXCONTRAST is 255"
#endif

/* Check power setting */

#if !defined(CONFIG_LCD_MAXPOWER)
#  define CONFIG_LCD_MAXPOWER 1
#endif

#if CONFIG_LCD_MAXPOWER != 1
#  warning "CONFIG_LCD_MAXPOWER should be 1"
#  undef CONFIG_LCD_MAXPOWER
#  define CONFIG_LCD_MAXPOWER 1
#endif

/* Color is 1bpp monochrome with leftmost column contained in bits 0  */

#ifdef CONFIG_NX_DISABLE_1BPP
#  warning "1 bit-per-pixel support needed"
#endif

/* Color Properties *******************************************************************/
/* The ST7567 display controller can handle a resolution of 128x64.
 */
/* Display Resolution */

#ifdef CONFIG_ST7567_XRES
#define ST7567_XRES         CONFIG_ST7567_XRES
#else
#define ST7567_XRES         128
#endif

#ifdef CONFIG_ST7567_YRES
#define ST7567_YRES         CONFIG_ST7567_YRES
#else
#define ST7567_YRES         64
#endif

/* Color depth and format */

#define ST7567_BPP          1
#define ST7567_COLORFMT     FB_FMT_Y1

/* Bytes per logical row andactual device row */

#define ST7567_XSTRIDE      (ST7567_XRES >> 3) /* Pixels arrange "horizontally for user" */
#define ST7567_YSTRIDE      (ST7567_YRES >> 3) /* But actual device arrangement is "vertical" */

/* The size of the shadow frame buffer */

#define ST7567_FBSIZE       (ST7567_XRES * ST7567_YSTRIDE)

/* Bit helpers */

#define LS_BIT          (1 << 0)
#define MS_BIT          (1 << 7)

/**************************************************************************************
 * Private Type Definition
 **************************************************************************************/

/* This structure describes the state of this driver */

struct st7565r_dev_s
{
  /* Publically visible device structure */

  struct lcd_dev_s dev;

  /* Private LCD-specific information follows */

  uint8_t contrast;
  uint8_t powered;


 //
 // The comment is historic one and left untouched.
 //
 /* The ST7567 does not support reading from the display memory in SPI mode.
  * Since there is 1 BPP and access is byte-by-byte, it is necessary to keep
  * a shadow copy of the framebuffer memory.
  */

  uint8_t fb[ST7567_FBSIZE];
};

#define st7565r_powerstring(x) ((x) ? "yes" : "no")
/**************************************************************************************
 * Private Function Protototypes
 **************************************************************************************/

/* LCD Data Transfer Methods */

static int st7565r_putrun(fb_coord_t row, fb_coord_t col, FAR const uint8_t *buffer,
                     size_t npixels);
static int st7565r_getrun(fb_coord_t row, fb_coord_t col, FAR uint8_t *buffer,
                     size_t npixels);

/* LCD Configuration */

static int st7565r_getvideoinfo(FAR struct lcd_dev_s *dev,
                           FAR struct fb_videoinfo_s *vinfo);
static int st7565r_getplaneinfo(FAR struct lcd_dev_s *dev, unsigned int planeno,
                           FAR struct lcd_planeinfo_s *pinfo);

/* LCD RGB Mapping */

#ifdef CONFIG_FB_CMAP
#  error "RGB color mapping not supported by this driver"
#endif

/* Cursor Controls */

#ifdef CONFIG_FB_HWCURSOR
#  error "Cursor control not supported by this driver"
#endif

/* LCD Specific Controls */

static int st7565r_getpower(struct lcd_dev_s *dev);
static int st7565r_setpower(struct lcd_dev_s *dev, int power);
static int st7565r_getcontrast(struct lcd_dev_s *dev);
static int st7565r_setcontrast(struct lcd_dev_s *dev, unsigned int contrast);

/* Initialization */

static inline void up_clear(FAR struct st7565r_dev_s  *priv);

static inline void send_data_page(FAR const uint8_t * const bytes, size_t n);

#define CMD_SEND(x) ( (*(volatile char *)LCD_SERVICE_AREA) = (x) )

/**************************************************************************************
 * Private Data
 **************************************************************************************/

/* This is working memory allocated by the LCD driver for each LCD device
 * and for each color plane.  This memory will hold one raster line of data.
 * The size of the allocated run buffer must therefore be at least
 * (bpp * xres / 8).  Actual alignment of the buffer must conform to the
 * bitwidth of the underlying pixel type.
 *
 * If there are multiple planes, they may share the same working buffer
 * because different planes will not be operate on concurrently.  However,
 * if there are multiple LCD devices, they must each have unique run buffers.
 */

static uint8_t g_runbuffer[ST7567_XSTRIDE+1];

/* This structure describes the overall LCD video controller */

static const struct fb_videoinfo_s g_videoinfo =
{
  .fmt     = ST7567_COLORFMT,    /* Color format: RGB16-565: RRRR RGGG GGGB BBBB */
  .xres    = ST7567_XRES,        /* Horizontal resolution in pixel columns */
  .yres    = ST7567_YRES,        /* Vertical resolution in pixel rows */
  .nplanes = 1,              /* Number of color planes supported */
};

/* This is the standard, NuttX Plane information object */

static const struct lcd_planeinfo_s g_planeinfo =
{
  .putrun = st7565r_putrun,             /* Put a run into LCD memory */
  .getrun = st7565r_getrun,             /* Get a run from LCD memory */
  .buffer = (uint8_t*)g_runbuffer, /* Run scratch buffer */
  .bpp    = ST7567_BPP,                /* Bits-per-pixel */
};

/* This is the standard, NuttX LCD driver object */

static struct st7565r_dev_s g_st7565r_dev =
{
  .dev =
  {
    /* LCD Configuration */

    .getvideoinfo = st7565r_getvideoinfo,
    .getplaneinfo = st7565r_getplaneinfo,

    /* LCD RGB Mapping -- Not supported */
    /* Cursor Controls -- Not supported */

    /* LCD Specific Controls */

    .getpower     = st7565r_getpower,
    .setpower     = st7565r_setpower,
    .getcontrast  = st7565r_getcontrast,
    .setcontrast  = st7565r_setcontrast,
  },
};

/**************************************************************************************
 * Private Functions
 **************************************************************************************/

/**************************************************************************************
 * Name:  st7565r_putrun
 *
 * Description:
 *   This method can be used to write a partial raster line to the LCD:
 *
 *   row     - Starting row to write to (range: 0 <= row < yres)
 *   col     - Starting column to write to (range: 0 <= col <= xres-npixels)
 *   buffer  - The buffer containing the run to be written to the LCD
 *   npixels - The number of pixels to write to the LCD
 *             (range: 0 < npixels <= xres-col)
 *
 **************************************************************************************/

static int st7565r_putrun(fb_coord_t row, fb_coord_t col, FAR const uint8_t *buffer,
                       size_t npixels)
{
  /* Because of this line of code, we will only be able to support a single ST7567 device */

  FAR struct st7565r_dev_s *priv = &g_st7565r_dev;
  FAR uint8_t *fbptr;
  FAR uint8_t *ptr;
  uint8_t fbmask;
  uint8_t page;
  uint8_t usrmask;
  uint8_t i;
  int     pixlen;

  gvdbg("row: %d col: %d npixels: %d\n", row, col, npixels);
  DEBUGASSERT(buffer);

  /* Clip the run to the display */

  pixlen = npixels;
  if ((unsigned int)col + (unsigned int)pixlen > (unsigned int)ST7567_XRES)
    {
      pixlen = (int)ST7567_XRES - (int)col;
    }

  /* Verify that some portion of the run remains on the display */

  if (pixlen <= 0 || row > ST7567_YRES)
    {
      return OK;
    }

  /* Get the page number.  The range of 64 lines is divided up into eight
   * pages of 8 lines each.
   */

  page = row >> 3;

  /* Update the shadow frame buffer memory. First determine the pixel
   * position in the frame buffer memory.  Pixels are organized like
   * this:
   *
   *  --------+---+---+---+---+-...-+-----+
   *  Segment | 0 | 1 | 2 | 3 | ... | 131 |
   *  --------+---+---+---+---+-...-+-----+
   *  Bit 0   |   | X |   |   |     |     |
   *  Bit 1   |   | X |   |   |     |     |
   *  Bit 2   |   | X |   |   |     |     |
   *  Bit 3   |   | X |   |   |     |     |
   *  Bit 4   |   | X |   |   |     |     |
   *  Bit 5   |   | X |   |   |     |     |
   *  Bit 6   |   | X |   |   |     |     |
   *  Bit 7   |   | X |   |   |     |     |
   *  --------+---+---+---+---+-...-+-----+
   *
   * So, in order to draw a white, horizontal line, at row 45. we
   * would have to modify all of the bytes in page 45/8 = 5.  We
   * would have to set bit 45%8 = 5 in every byte in the page.
   */

  fbmask  = 1 << (row & 7);
  fbptr   = &priv->fb[page * ST7567_XRES + col];
  ptr     = fbptr;
#ifdef CONFIG_NX_PACKEDMSFIRST
  usrmask = MS_BIT;
#else
  usrmask = LS_BIT;
#endif

  for (i = 0; i < pixlen; i++)
    {
      /* Set or clear the corresponding bit */

      if ((*buffer & usrmask) != 0)
        {
          *ptr++ |= fbmask;
        }
      else
        {
          *ptr++ &= ~fbmask;
        }

      /* Inc/Decrement to the next source pixel */

#ifdef CONFIG_NX_PACKEDMSFIRST
      if (usrmask == LS_BIT)
        {
          buffer++;
          usrmask = MS_BIT;
        }
      else
        {
          usrmask >>= 1;
        }
#else
      if (usrmask == MS_BIT)
        {
          buffer++;
          usrmask = LS_BIT;
        }
      else
        {
          usrmask <<= 1;
        }
#endif
    }

  /* Set the starting position for the run */

  CMD_SEND(ST7567_SETPAGESTART+page);         /* Set the page start */
  CMD_SEND(ST7567_SETCOLL + (col & 0x0f));    /* Set the low column */
  CMD_SEND(ST7567_SETCOLH + (col >> 4));      /* Set the high column */

  /* Then transfer all of the data */

  send_data_page(fbptr, pixlen);

  return OK;
}

/**************************************************************************************
 * Name:  st7565r_getrun
 *
 * Description:
 *   This method can be used to read a partial raster line from the LCD:
 *
 *  row     - Starting row to read from (range: 0 <= row < yres)
 *  col     - Starting column to read read (range: 0 <= col <= xres-npixels)
 *  buffer  - The buffer in which to return the run read from the LCD
 *  npixels - The number of pixels to read from the LCD
 *            (range: 0 < npixels <= xres-col)
 *
 **************************************************************************************/

static int st7565r_getrun(fb_coord_t row, fb_coord_t col, FAR uint8_t *buffer,
                     size_t npixels)
{
  /* Because of this line of code, we will only be able to support a single ST7567 device */

  FAR struct st7565r_dev_s *priv = &g_st7565r_dev;
  FAR uint8_t *fbptr;
  uint8_t page;
  uint8_t fbmask;
  uint8_t usrmask;
  uint8_t i;
  int     pixlen;

  gvdbg("row: %d col: %d npixels: %d\n", row, col, npixels);
  DEBUGASSERT(buffer);

  /* Clip the run to the display */

  pixlen = npixels;
  if ((unsigned int)col + (unsigned int)pixlen > (unsigned int)ST7567_XRES)
    {
      pixlen = (int)ST7567_XRES - (int)col;
    }

  /* Verify that some portion of the run is actually the display */

  if (pixlen <= 0 || row > ST7567_YRES)
    {
      return -EINVAL;
    }

  /* Then transfer the display data from the shadow frame buffer memory */
  /* Get the page number.  The range of 64 lines is divided up into eight
   * pages of 8 lines each.
   */

  page = row >> 3;

  /* Update the shadow frame buffer memory. First determine the pixel
   * position in the frame buffer memory.  Pixels are organized like
   * this:
   *
   *  --------+---+---+---+---+-...-+-----+
   *  Segment | 0 | 1 | 2 | 3 | ... | 131 |
   *  --------+---+---+---+---+-...-+-----+
   *  Bit 0   |   | X |   |   |     |     |
   *  Bit 1   |   | X |   |   |     |     |
   *  Bit 2   |   | X |   |   |     |     |
   *  Bit 3   |   | X |   |   |     |     |
   *  Bit 4   |   | X |   |   |     |     |
   *  Bit 5   |   | X |   |   |     |     |
   *  Bit 6   |   | X |   |   |     |     |
   *  Bit 7   |   | X |   |   |     |     |
   *  --------+---+---+---+---+-...-+-----+
   *
   * So, in order to draw a white, horizontal line, at row 45. we
   * would have to modify all of the bytes in page 45/8 = 5.  We
   * would have to set bit 45%8 = 5 in every byte in the page.
   */

  fbmask  = 1 << (row & 7);
  fbptr   = &priv->fb[page * ST7567_XRES + col];
#ifdef CONFIG_NX_PACKEDMSFIRST
  usrmask = MS_BIT;
#else
  usrmask = LS_BIT;
#endif

  *buffer = 0;
  for (i = 0; i < pixlen; i++)
    {
      /* Set or clear the corresponding bit */

      uint8_t byte = *fbptr++;
      if ((byte & fbmask) != 0)
        {
          *buffer |= usrmask;
        }

      /* Inc/Decrement to the next destination pixel. Hmmmm. It looks like
       * this logic could write past the end of the user buffer.  Revisit
       * this!
       */

#ifdef CONFIG_NX_PACKEDMSFIRST
      if (usrmask == LS_BIT)
        {
          buffer++;
         *buffer = 0;
          usrmask = MS_BIT;
        }
      else
        {
          usrmask >>= 1;
        }
#else
      if (usrmask == MS_BIT)
        {
          buffer++;
         *buffer = 0;
          usrmask = LS_BIT;
        }
      else
        {
          usrmask <<= 1;
        }
#endif
    }

  return OK;
}

/**************************************************************************************
 * Name:  st7565r_getvideoinfo
 *
 * Description:
 *   Get information about the LCD video controller configuration.
 *
 **************************************************************************************/

static int st7565r_getvideoinfo(FAR struct lcd_dev_s *dev,
                              FAR struct fb_videoinfo_s *vinfo)
{
  DEBUGASSERT(dev && vinfo);
  gvdbg("fmt: %d xres: %d yres: %d nplanes: %d\n",
         g_videoinfo.fmt, g_videoinfo.xres, g_videoinfo.yres, g_videoinfo.nplanes);
  memcpy(vinfo, &g_videoinfo, sizeof(struct fb_videoinfo_s));
  return OK;
}

/**************************************************************************************
 * Name:  st7565r_getplaneinfo
 *
 * Description:
 *   Get information about the configuration of each LCD color plane.
 *
 **************************************************************************************/

static int st7565r_getplaneinfo(FAR struct lcd_dev_s *dev, unsigned int planeno,
                              FAR struct lcd_planeinfo_s *pinfo)
{
  DEBUGASSERT(dev && pinfo && planeno == 0);
  gvdbg("planeno: %d bpp: %d\n", planeno, g_planeinfo.bpp);
  memcpy(pinfo, &g_planeinfo, sizeof(struct lcd_planeinfo_s));
  return OK;
}

/**************************************************************************************
 * Name:  st7565r_getpower
 *
 * Description:
 *   Get the LCD panel power status (0: full off - CONFIG_LCD_MAXPOWER: full on). On
 *   backlit LCDs, this setting may correspond to the backlight setting.
 *
 **************************************************************************************/

static int st7565r_getpower(struct lcd_dev_s *dev)
{
  struct st7565r_dev_s *priv = (struct st7565r_dev_s *)dev;
  DEBUGASSERT(priv);
  gvdbg("powered: %s\n", st7565r_powerstring(priv->powered));
  return priv->powered;
}

/**************************************************************************************
 * Name:  st7565r_setpower
 *
 * Description:
 *   Enable/disable LCD panel power (0: full off - CONFIG_LCD_MAXPOWER: full on). On
 *   backlit LCDs, this setting may correspond to the backlight setting.
 *
 **************************************************************************************/

static int st7565r_setpower(struct lcd_dev_s *dev, int power)
{
  struct st7565r_dev_s *priv = (struct st7565r_dev_s *)dev;

  DEBUGASSERT(priv && (unsigned)power <= CONFIG_LCD_MAXPOWER);
  gvdbg("power: %s powered: %s\n",
        st7565r_powerstring(power), st7565r_powerstring(priv->powered));

  /* Select and lock the device */

  if (power <= ST7567_POWER_OFF)
    {
      /* Turn the display off */

      CMD_SEND(ST7567_DISPOFF);       /* Display off */
      priv->powered = ST7567_POWER_OFF;
    }
  else
    {
      CMD_SEND(ST7567_DISPON);        /* Display on, normal mode */
      power = ST7567_POWER_ON;

      CMD_SEND(ST7567_DISPRAM);       /* Resume to RAM content display */
      priv->powered = power;
    }

  return OK;
}

/**************************************************************************************
 * Name:  st7565r_getcontrast
 *
 * Description:
 *   Get the current contrast setting (0-CONFIG_LCD_MAXCONTRAST).
 *
 **************************************************************************************/

static int st7565r_getcontrast(struct lcd_dev_s *dev)
{
  struct st7565r_dev_s *priv = (struct st7565r_dev_s *)dev;
  DEBUGASSERT(priv);
  return (int)priv->contrast;
}

/**************************************************************************************
 * Name:  st7565r_setcontrast
 *
 * Description:
 *   Set LCD panel contrast (0-CONFIG_LCD_MAXCONTRAST).
 *
 **************************************************************************************/

static int st7565r_setcontrast(struct lcd_dev_s *dev, unsigned int contrast)
{
  struct st7565r_dev_s *priv = (struct st7565r_dev_s *)dev;

  gvdbg("contrast: %d\n", contrast);
  DEBUGASSERT(priv);

  if (contrast > 255)
    {
      return -EINVAL;
    }

  /* Set the contrast */

  CMD_SEND(ST7567_SETEV);         /* Set contrast control register */
  CMD_SEND(contrast);             /* Data 1: Set 1 of 256 contrast steps */
  priv->contrast = contrast;

  return OK;
}

/**************************************************************************************
 * Name:  up_clear
 *
 * Description:
 *   Clear the display.
 *
 **************************************************************************************/

static inline void up_clear(FAR struct st7565r_dev_s  *priv)
{
  int page;
  int i;

  /* Clear the framebuffer */

  memset(priv->fb, ST7567_Y1_BLACK, ST7567_FBSIZE);

  /* Go throst7567h all 8 pages */

  for (page = 0, i = 0; i < 8; i++)
    {

      /* Set the starting position for the run */

      CMD_SEND(ST7567_SETPAGESTART+i);
      CMD_SEND(ST7567_SETCOLL);
      CMD_SEND(ST7567_SETCOLH);

      /* Then transfer all 96 columns of data */

      send_data_page(&priv->fb[page * ST7567_XRES], ST7567_XRES);
    }
}


static inline void send_data_page(FAR const uint8_t * const bytes, size_t n)
{
	volatile uint8_t * const p = LCD_DATA_AREA;
	for (size_t i = 0; i < n; ++i)
		*p = bytes[i];
}

/**************************************************************************************
 * Public Functions
 **************************************************************************************/

/**************************************************************************************
 * Name:  st7565r_initialize
 *
 * Description:
 *   Initialize the ST7567 video hardware.  The initial state of the
 *   OLED is fully initialized, display memory cleared, and the OLED ready to
 *   use, but with the power setting at 0 (full off == sleep mode).
 *
 * Input Parameters:
 *
 *   spi - A reference to the SPI driver instance.
 *   devno - A value in the range of 0 throst7567h CONFIG_ST7567_NINTERFACES-1.
 *     This allows support for multiple OLED devices.
 *
 * Returned Value:
 *
 *   On success, this function returns a reference to the LCD object for the specified
 *   OLED.  NULL is returned on any failure.
 *
 **************************************************************************************/

__EXPORT FAR struct lcd_dev_s *st7565r_initialize();
FAR struct lcd_dev_s *st7565r_initialize()
{
  /* Configure and enable LCD */

  FAR struct st7565r_dev_s  *priv = &g_st7565r_dev;

  display_init();

  priv->powered = CONFIG_LCD_MAXPOWER;
  priv->contrast = CONFIG_LCD_MAXCONTRAST;

//  gvdbg("Initializing\n");
//
//  /* Make sure that the OLED off */
//
//  st7565r_setpower(priv, false);
//
//  /* Set the starting position for the run */
//
//  CMD_SEND(ST7567_EXIT_SOFTRST);
//  CMD_SEND(ST7567_BIAS_1_7);
//  CMD_SEND(SSD1305_MAPCOL0);
//  CMD_SEND(ST7567_SETCOMREVERSE);
//  CMD_SEND(ST7567_REG_RES_RR1);
//  CMD_SEND(ST7567_SETEV);
//  CMD_SEND(0x32);
//  CMD_SEND(ST7567_POWERCTRL);
//  CMD_SEND(ST7567_SETSTARTLINE);
//  CMD_SEND(ST7567_SETPAGESTART);
//  CMD_SEND(ST7567_SETCOLH);
//  CMD_SEND(ST7567_SETCOLL);
//  CMD_SEND(ST7567_DISPON);
//  CMD_SEND(ST7567_DISPRAM);

  /* Clear the framebuffer */

  up_mdelay(100);
  up_clear(priv);
  return &priv->dev;
}
