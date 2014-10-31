MODULE_COMMAND		= spi_ex

SRCS			= main.cpp

CXXFLAGS		+= -std=c++11 -Dmain=${MODULE_COMMAND}_main
