#
# Some commands to do stuff from commandline for testing purposes. 
# For example simultating drone button functionality.
#

MODULE_COMMAND		= clh

SRCS			= main.cpp

CXXFLAGS		+= -std=c++11 -Dmain=${MODULE_COMMAND}_main
