#ifndef _MENU_CONTROLLER_H
#define _MENU_CONTROLLER_H

#include "i2c_display_controller.h"
#include "button_controller.h"
#include "paramhandler.h"
#include <systemlib/err.h>

#define BUTTON_ACTION(type, cb) ({ type, { &MENU_CONTROLLER::cb } })

typedef enum {
	MENU_LEVEL_PARAMS = 0,
	MENU_LEVEL_SET,
    MENU_LEVEL_CHOOSE_FUNCTION,
	MAX_MENU_LEVEL,
} menu_level_t;

class MENU_CONTROLLER
{
public:
	MENU_CONTROLLER(I2C_DISPLAY_CONTROLLER *pi2c_disp_ctrl, cParamHandler *paramHandler);

	void handlePressedButton(uint8_t button, hrt_abstime time);
	void handleClickedButton(uint8_t button/*, bool long_press*/);

	void open(void);
	void close(void);

	inline bool isActive(void) { return active; }

private:
	typedef void (MENU_CONTROLLER::*button_pressed_t)(hrt_abstime time);
	typedef void (MENU_CONTROLLER::*button_clicked_t)(/*bool longpress*/);
	typedef union {
		button_pressed_t pressed;
		button_clicked_t clicked;
	} uni_button_callback_t;
	typedef struct {
		enum button_callback_type type;
		uni_button_callback_t func;
	} button_map_t;

	I2C_DISPLAY_CONTROLLER *m_pi2c_disp_ctrl;
	cParamHandler *m_paramHandler;

	bool active;

	menu_level_t currentLevel;

	int selectedParamId;

	void displaySelectedParam(void);
	void displaySelectedParamValue(void);

	void selectPrevParam(void);
	void selectNextParam(void);
	void editCurrentParam(void);

	void decCurrentParam(void/*hrt_abstime time*/);
	void incCurrentParam(void/*hrt_abstime time*/);
	void cancelEditing(void);
	void saveCurrentParam(void);

	inline void nop(void) { }
};

#endif //_MENU_CONTROLLER_H
