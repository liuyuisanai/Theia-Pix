/*
 * This class handles different sets of buttons according to their
 * previous and current status (pressed / not pressed), and duration of pressing a button.
 *
 * Duration of pressing a button is used to distinguish
 * button click and longpress (i.e. long button press)
 *
 * Status of all buttons in a set is defined by the bitmap variable `mask`, which
 * can be set using method `cButtonController::setMask(button_set bs, uint32_t bitmask)`
 * So count of buttons in a set is limited to number of bits in uint32_t (i.e. 32)
 * Bitmap variable `mask` is defined as follows:
 *	- Bit N is set to **1** if N'th button is **not pressed**
 *	- Bit N is set to **0** if N'th button is **pressed**
 *
 * Each set of buttons is checked in the method `cButtonController::cycle` by
 * calling `cButtonController::check(button_set, array_of_buttons, count_of_buttons)`
 * for each button set.
 *
 * Each button is handled by a callback that is defined for the button set.
 * First two arguments of a callback are:
 *	void*	- optional argument	(meant to be used to pass `this` variable
 *								 if callback method is located in a class)
 *	uint8_t	- number of a button
 *
 * Callback must return:
 *	BUTTON_HANDLED (defined as true)	- if the button *was successfully handled*
 *										- and *no* other callback must be called.
 *	BUTTON_IGNORED (defined as false)	- if the button *was not handled*
 *										- and another callback can be called.
 *
 * There are two types of callbacks:
 *  - Clicked:	called when a button was pressed in the previous iteration of cycle
 *				but is no longer pressed in the current iteration of cycle.
 *				(i.e. button pressed and released)
 *				Accepts additional argument:
 *					- `bool longpress`: whether the button was pressed longer than _LONG_PRESS_TIME_
 *
 *  - Pressed:	called when a button is pressed first time and each iteration of cycle.
 *				Accepts additional arguent:
 *					- `hrt_abstime time`: duration pressing the button
 *
 * If callback pressed returns BUTTON_IGNORED and
 * callback clicked is set and duration of pressing the button exceeds _LONG_PRESS_TIME_ then
 * callback **clicked** is called with argument `longpress=true`, if it is handled then
 * `ignore_mask` is set to ignore the pressed button in next cycle iterations untill it is released.
 *
 * There are 5 possible conditions of a button,
 * only 4 of them are handled:
+-------------------+---------------------+---------------------+
|	   Status       |     NOT pressed     |     WAS pressed     |
|  Current\Previous |                     |                     |
+-------------------+---------------------+---------------------+
|    NOT pressed    |               	  |      callback:      |
|                   |  nothing to handle  | clicked(t >= LPTIME)|
+-------------------+---------------------+---------------------+
|     IS pressed    |                     |      callback:      |
|      < LPTIME     |	   callback:      |       pressed       |
+-------------------+       pressed       +---------------------+
|     IS pressed    |     (time = 0)      |      callback:      |
|     >= LPTIME     |                     |    clicked(true)    |
+-------------------+---------------------+---------------------+
 */

#ifndef BUTTON_CONTROLLER_H
#define BUTTON_CONTROLLER_H

/*
 * Type: hrt_abstime
 * Measured in microseconds
 */
#define LONG_PRESS_TIME 1500000

/*
 * Type: hrt_abstime
 * Measured in microseconds
 */
#define PRESSED_BUTTON_RESEND_CYCLE 500000
#define LONG_PRESS_LENGTH 1000000

#define BUTTON_HANDLED true
#define BUTTON_IGNORED false

#include <string.h>

#include <drivers/drv_hrt.h>
#include <systemlib/err.h>

#include "common.h"

enum button_set {
	BS_I2C = 0,
	BS_COUNT
};

enum button_callback_type {
	BCBT_PRESSED = 0,
	BCBT_CLICKED,
	BCBT_COUNT
};

typedef bool (*bc_callback_pressed_t)(void *arg, uint8_t button, hrt_abstime time);
typedef bool (*bc_callback_clicked_t)(void *arg, uint8_t button, bool longpress);

class cButtonController
{
public:
	cButtonController(void);

	//Checks all sets of buttons and calls representative callback
	void cycle(void);

	//Sets bitmap mask of a button set
	void setMask(enum button_set bs, uint32_t m);

	void setCallback(enum button_set bs, enum button_callback_type bct, void *cb, void *arg = nullptr);

    

private:
	struct button_s {
		bool pressed;
		hrt_abstime time_pressed;
		hrt_abstime last_command_sent;
	};

	void check(button_set bs, struct button_s *buttons, int count);

	struct button_s bi2c[BUTTON_COUNT_I2C];

	void *callbacks[BS_COUNT][BCBT_COUNT];
	void *callback_args[BS_COUNT][BCBT_COUNT];

	uint32_t mask[BS_COUNT];
	uint32_t ignore_mask[BS_COUNT];

    bool long_press_sent = false;
};

#endif // BUTTON_CONTROLLER_H
