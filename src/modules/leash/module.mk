MODULE_COMMAND = $(notdir $(shell pwd))

SRCS = main.cpp tones.cpp
EXTRACXXFLAGS = -std=gnu++11 -Dmain=$(MODULE_COMMAND)_main
