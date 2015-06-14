#include "menu_controller.h"
#include "common.h"

#include <nuttx/config.h>
#include <nuttx/clock.h>

#include <drivers/drv_hrt.h>

#include <sys/types.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>

#include <systemlib/perf_counter.h>
#include <systemlib/err.h>
#include <systemlib/systemlib.h>

MENU_CONTROLLER::MENU_CONTROLLER(I2C_DISPLAY_CONTROLLER *pi2c_disp_ctrl, cParamHandler *paramHandler) :
	m_pi2c_disp_ctrl(pi2c_disp_ctrl),
	m_paramHandler(paramHandler),
	active(false),
	currentLevel(MENU_LEVEL_PARAMS),
	selectedParamId(0)
{
}

void MENU_CONTROLLER::handlePressedButton(uint8_t button, hrt_abstime time) {
	if(!m_paramHandler->allParamsAreLoaded())
		return;

	if(button >= BUTTON_COUNT_I2C)
		return;

	//button_map_t cb = this->buttonMap[currentLevel][button];
    /*
	switch(cb.type) {
		case BCBT_PRESSED:
			(this->*cb.func.pressed)(time);
			break;
	}
    */
}

void MENU_CONTROLLER::handleClickedButton(uint8_t button/*, bool long_press*/) {

	if(!m_paramHandler->allParamsAreLoaded())
		return;
	if(button >= BUTTON_COUNT_I2C)
		return;
    /*
	button_map_t cb = this->buttonMap[currentLevel][button];
	switch(cb.type) {
		case BCBT_CLICKED:
			(this->*cb.func.clicked)(long_press);
			break;
	}
    */

	switch(button) {
		case 0:
			// ON/OFF button
            close();
			break;
		case 1:
			// DOWN button
			break;
		case 2:
			// PLAY button
			break;
		case 3:
			// UP button
			break;
		case 4:
			// CENTER button
            if (currentLevel == MENU_LEVEL_SET) {
                saveCurrentParam();
                cancelEditing();
            }
            else
            if (currentLevel == MENU_LEVEL_PARAMS) {
                editCurrentParam();
            }

			break;
		case 5:
			// CENTER DOWN
            if (currentLevel == MENU_LEVEL_SET) {
                decCurrentParam();
            }
            else
            if (currentLevel == MENU_LEVEL_PARAMS) {
                selectPrevParam();
            }
			break;
		case 6:
			// CENTER RIGHT
			break;
		case 7:
			// CENTER UP

            if (currentLevel == MENU_LEVEL_SET) {
                incCurrentParam();
            }
            else
            if (currentLevel == MENU_LEVEL_PARAMS) {
                selectNextParam();
            }
			break;
		case 8:
			// CENTER LEFT
			break;
		case 9:
			// DOWN + CENTER
			break;
		case 10:
			// UP + CENTER
			break;
		case 11:
			// CENTER DOWN + CENTER
			break;
		case 12:
		{
			// CENTER RIGHT + CENTER
			break;
		}
		case 13:
		{
			// CENTER UP + CENTER
			break;
		}
		case 14:
			// CENTER LEFT + CENTER
			break;
	}

}

void MENU_CONTROLLER::open(void) {

	if(!m_paramHandler->allParamsAreLoaded()) {

		m_pi2c_disp_ctrl->set_symbols(SYMBOL_DOT, SYMBOL_DOT, SYMBOL_DOT);
		return;

	}

	active = true;
	displaySelectedParam();
}

void MENU_CONTROLLER::close(void) {
	if(currentLevel == MENU_LEVEL_SET)
		cancelEditing();
	m_paramHandler->uploadAllParams();
	active = false;
}

void MENU_CONTROLLER::displaySelectedParam(void) {
	m_pi2c_disp_ctrl->set_symbols_from_str(m_paramHandler->getDisplaySymbols(selectedParamId));
}

void MENU_CONTROLLER::displaySelectedParamValue(void) {
	switch(m_paramHandler->getEditingType(selectedParamId)) {
		case PTYPE_INT:
			m_pi2c_disp_ctrl->set_symbols_from_int(m_paramHandler->getEditingValue(selectedParamId));
			break;
		case PTYPE_FLOAT:
			m_pi2c_disp_ctrl->set_symbols_from_float(m_paramHandler->getEditingValue(selectedParamId));
			break;
	}
}

void MENU_CONTROLLER::selectPrevParam(void) {
	selectedParamId--;
	if(selectedParamId < 0)
		selectedParamId = m_paramHandler->getParamCount() - 1;
	displaySelectedParam();
}

void MENU_CONTROLLER::selectNextParam(void) {
	selectedParamId++;
	if(selectedParamId >= m_paramHandler->getParamCount())
		selectedParamId = 0;
	displaySelectedParam();
}

void MENU_CONTROLLER::editCurrentParam(void) {
	currentLevel = MENU_LEVEL_SET;
	displaySelectedParamValue();
}

void MENU_CONTROLLER::decCurrentParam(/*hrt_abstime time*/void) {
	//if(time == 0 || time > LONG_PRESS_TIME)
    //
	m_paramHandler->decParam(selectedParamId);
	displaySelectedParamValue();
}

void MENU_CONTROLLER::incCurrentParam(/*hrt_abstime time*/void) {
	//if(time == 0 || time > LONG_PRESS_TIME)
	m_paramHandler->incParam(selectedParamId);
	displaySelectedParamValue();
}

void MENU_CONTROLLER::cancelEditing(void) {
	currentLevel = MENU_LEVEL_PARAMS;
	m_paramHandler->resetParam(selectedParamId);
	displaySelectedParam();
}

void MENU_CONTROLLER::saveCurrentParam(void) {
	if(m_paramHandler->setParam(selectedParamId)) {
		currentLevel = MENU_LEVEL_PARAMS;
		displaySelectedParam();
	}
}
