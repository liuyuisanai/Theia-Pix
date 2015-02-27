MODULE_COMMAND = $(notdir $(shell pwd))

SRCS = main.cpp

EXTRACXXFLAGS += -std=c++11 -DMODULE_COMMAND=${MODULE_COMMAND}
