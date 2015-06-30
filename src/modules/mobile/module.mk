MODULE_COMMAND = $(notdir $(shell pwd))

SRCS = main.cpp
SHOW_ALL_ERRORS = yes
TOLERATE_MISSING_DECLARATION = yes

EXTRACXXFLAGS = -std=c++11 \
		-Dmain=$(MODULE_COMMAND)_main \
		#end of EXTRACXXFLAGS
