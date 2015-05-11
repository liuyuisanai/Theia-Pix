MODULE_COMMAND = $(notdir $(shell pwd))

SRCS = main.cpp tones.cpp uorb_functions.cpp
EXTRACXXFLAGS = -std=gnu++11 -Dmain=$(MODULE_COMMAND)_main

#
# All error messages are essential for finding conflicting template candidates.
#
SHOW_ALL_ERRORS = yes
