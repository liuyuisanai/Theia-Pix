#pragma once

#include <nuttx/config.h>
#include <stddef.h>

__BEGIN_DECLS

__EXPORT   void display_enter_powersave(void);
__EXPORT   void display_exit_powersave(void);
__EXPORT   void display_init(void);
__EXPORT   void display_shutdown(void);
__EXPORT   void display_test_nop(size_t n);
__EXPORT   void display_turn_off(void);
__EXPORT   void display_turn_on(void);

__EXPORT   void fmc_enable(void);
__EXPORT   void fmc_disable(void);

__EXPORT   void up_display_mcu_setup(void);
__EXPORT   void up_display_fmc(void);

__EXPORT FAR struct lcd_dev_s *up_lcdgetdev(int lcddev);
__EXPORT FAR struct lcd_dev_s *up_nxdrvinit(unsigned int devno);

__END_DECLS
