MODULE_COMMAND		= kbd_test

SRCS			= main.cpp

CXXFLAGS		+= -std=c++11 -Dmain=${MODULE_COMMAND}_main
