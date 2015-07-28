MODULE_COMMAND		= $(notdir $(shell pwd))

SRCS			= main.cpp leds.cpp params.c pwm_led.c

EXTRACXXFLAGS		+= -std=c++11 -Dmain=${MODULE_COMMAND}_main
