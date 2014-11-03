MODULE_COMMAND		= debug_button_pressed

SRCS			= main.cpp

CXXFLAGS		+= -std=c++11 -Dmain=${MODULE_COMMAND}_main
