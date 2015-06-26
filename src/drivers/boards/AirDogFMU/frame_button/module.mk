MODULE_COMMAND = $(notdir $(shell pwd))

CXXFLAGS	+= -std=gnu++11
EXTRACXXFLAGS = -Dmain=$(MODULE_COMMAND)_main
SRCS		= driver.cpp main.cpp
