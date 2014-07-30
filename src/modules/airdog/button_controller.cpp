#include "button_controller.h"

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
	bc_callback_pressed_t cb_pressed;

	now = hrt_absolute_time();

	cb_clicked = (bc_callback_clicked_t)callbacks[bs][BCBT_CLICKED];
	cb_pressed = (bc_callback_pressed_t)callbacks[bs][BCBT_PRESSED];

	for(int i = 0; i < count; i++) {
		now_pressed = !(mask[bs] & (1 << i));
		if(!buttons[i].pressed) {
			if(!now_pressed) {
				continue;
			}
			if(ignore_mask[bs] & (1 << i)) {
				//If now_pressed == true because of ignore_mask,
				//unset ignore_mask (it's no more actual)
				ignore_mask[bs] &= ~(1 << i);
				continue;
			} else {
				buttons[i].time_pressed = now;
				buttons[i].pressed = true;
			}
		}

		if(now_pressed) {
			if(cb_pressed && (*cb_pressed)(callback_args[bs][BCBT_PRESSED], i, now - buttons[i].time_pressed)) {
				//If pressed callback is available and it is handled
				//do nothing
				continue;
			} else if(cb_clicked && now - buttons[i].time_pressed > LONG_PRESS_TIME) {
				//If clicked callback is available and it is longpress,
				//try to handle longpress now, not waiting for clicked event
				if((*cb_clicked)(callback_args[bs][BCBT_CLICKED], i, true)) {
					//If clicked callback is handled, setup ignore mask,
					//so we wouldn't call clicked callback multiple times
					buttons[i].pressed = false;
					ignore_mask[bs] |= (1 << i);
					mask[bs] &= ~(1 << i);
				}
			}
		} else {
			if(cb_clicked) {
				(*cb_clicked)(callback_args[bs][BCBT_PRESSED], i, (now - buttons[i].time_pressed) > LONG_PRESS_TIME);
			}
			buttons[i].pressed = false;
		}
	}
}

void cButtonController::setCallback(enum button_set bs, enum button_callback_type bct, void *cb, void *arg) {
	callbacks[bs][bct] = cb;
	callback_args[bs][bct] = arg;
}
