#include "button_controller.h"
#include <stdio.h>

cButtonController::cButtonController(void)
{
	memset(bi2c, 0, sizeof(bi2c));

	memset(callbacks, 0, sizeof(callbacks));
	memset(callback_args, 0, sizeof(callback_args));

	memset(mask, 0, sizeof(mask));
	memset(ignore_mask, 0, sizeof(ignore_mask));
}

void cButtonController::setMask(button_set bs, uint32_t m) {
	mask[bs] = m ^ ignore_mask[bs];
}

void cButtonController::cycle(void) {
	//I2C button mask is set in i2c_controller
	check(BS_I2C, bi2c, ARRAY_SIZE(bi2c));
}

void cButtonController::check(enum button_set bs, struct button_s *buttons, int count) {
	hrt_abstime now;
	bool now_pressed;
	bc_callback_clicked_t cb_clicked;
	//bc_callback_pressed_t cb_pressed;

	now = hrt_absolute_time();

	cb_clicked = (bc_callback_clicked_t)callbacks[bs][BCBT_CLICKED];
	//cb_pressed = (bc_callback_pressed_t)callbacks[bs][BCBT_PRESSED];

	bool center_button_pressed = buttons[4].pressed;
	hrt_abstime center_button_time_pressed = buttons[4].time_pressed;

	for (int i = 0;i < count; i++){

		now_pressed = !(mask[bs] & (1 << i));

		// In last iteration wasn't pressed. Now it's pressed.
		if (now_pressed && !buttons[i].pressed) {
			buttons[i].time_pressed = now;
			buttons[i].last_command_sent = 0;
			buttons[i].pressed = true;
		}

        /*
		// Pressed buttons have to be handled first for menu to work.
		if(cb_pressed && (*cb_pressed)(callback_args[bs][BCBT_CLICKED], i, now - buttons[i].time_pressed)) {
			continue;
		}
        */

		// In last iteration button was pressed. Now it's now.
		if (!now_pressed && buttons[i].pressed) {

            // long press for play/ pause
            if ( i == 2) {

                if (!long_press_sent) {
                    // if not longpress was sent then it's short press
                    (*cb_clicked)(callback_args[bs][BCBT_CLICKED], i, false);
                } else {
                    // init value to false
                    long_press_sent = false;
                }
            }
			buttons[i].pressed = false;
		}

        // Center button special handling
        if (i==4 && buttons[i].pressed) {

            // Handle button press the first time
            if (buttons[i].last_command_sent == 0) {
                (*cb_clicked)(callback_args[bs][BCBT_CLICKED], i, false);
                buttons[i].last_command_sent = now;
            }

        }
        else

		if (i!=4 && buttons[i].pressed)
		{
			// If center button is held down, all other buttons will have different functionality.
			if (center_button_pressed && center_button_time_pressed < buttons[i].time_pressed) {

				// In combination with center button, we don't need
				// pressed button resend functionality

				int new_button_number = 0;

				switch (i) {
					// DOWN + CENTER
					case 1:
						new_button_number = 9;
						break;
					// UP + CENTER
					case 3:
						new_button_number = 10;
						break;
					// CENTER DOWN + CENTER
					case 5:
						new_button_number = 11;
						break;
					// CENTER RIGHT + CENTER
					case 6:
						new_button_number = 12;
						break;
					// CENTER UP + CENTER
					case 7:
						new_button_number = 13;
						break;
					// CENTER LEFT + CENTER
					case 8:
						new_button_number = 14;
						break;
				}

				if (new_button_number && buttons[i].last_command_sent == 0) {
					(*cb_clicked)(callback_args[bs][BCBT_CLICKED], new_button_number, false);
					buttons[i].last_command_sent = now;
				}

			// Handle buttons when center button is not pressed.
			} else {


                if ( i== 2) {

                    // Long press sent once the press time is longer than LONG_PRESS_LENGTH
                    if (now - buttons[i].time_pressed > LONG_PRESS_LENGTH) {
                        (*cb_clicked)(callback_args[bs][BCBT_CLICKED], i, true);
                        long_press_sent = true;
                    }

                }
                else {


                    // Handle button press the first time
                    if (buttons[i].last_command_sent == 0) {
                        (*cb_clicked)(callback_args[bs][BCBT_CLICKED], i, false);
                        buttons[i].last_command_sent = now;
                    }

                    // Pressed button resend cycle
                    // Resend will work with:
                    // DOWN, UP, CENTER DONW, CENTER RIGHT, CENTER UP, CENTER LEFT
                    if ( i == 1 || i == 3 || i == 5 || i == 6 || i == 7 || i == 8) {

                        if ( now - buttons[i].last_command_sent > PRESSED_BUTTON_RESEND_CYCLE) {
                            (*cb_clicked)(callback_args[bs][BCBT_CLICKED], i, false);
                            buttons[i].last_command_sent = now;
                        }
                    }
                }
			}
		}

	}
}

void cButtonController::setCallback(enum button_set bs, enum button_callback_type bct, void *cb, void *arg) {
	callbacks[bs][bct] = cb;
	callback_args[bs][bct] = arg;
}
