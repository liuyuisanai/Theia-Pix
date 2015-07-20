MODULE_COMMAND = frame_button

CXXFLAGS	+= -std=gnu++11
SRCS		= driver.cpp button_state.cpp
EXTRACXXFLAGS = -Dmain=$(MODULE_COMMAND)_main
