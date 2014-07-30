#ifndef _I2C_CONTROLLER_H
#define _I2C_CONTROLLER_H

#include <drivers/device/i2c.h>

#include "button_controller.h"
#include "common.h"

class __EXPORT I2C_CONTROLLER : public device::I2C
{
public:
	I2C_CONTROLLER(int bus, int addr);

	int				init(cButtonController *pbutton_ctrl);
	int				probe();

	void			cycle();

    void            start_blinking_led(i2c_led_t led, blinking_rate_t rate);
    void            stop_blinking_led(i2c_led_t led, bool should_restore_state);
    void            set_green_led_on(bool set_on);
    void            set_red_led_on(bool set_on);

private:
	cButtonController *m_pbutton_ctrl;

    led_s           _green_led;
    led_s           _red_led;
    bool            _led_update_in_progress;
    int			    _listening_interval;

    bool            _is_red_led_on;
    bool            _is_green_led_on;

    int             get_buttons_state_from_r1();
    int             get_buttons_state_from_r2();
    int             init_led();
};

#endif
