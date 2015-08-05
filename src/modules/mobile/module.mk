MODULE_COMMAND = $(notdir $(shell pwd))

SRCS = main.cpp
TOLERATE_MISSING_DECLARATION = yes

EXTRACXXFLAGS = -std=c++11 \
		 -Dmain=$(MODULE_COMMAND)_main \
# end of EXTRACXXFLAGS

#
# All error messages are essential for finding conflicting template candidates.
#
SHOW_ALL_ERRORS = yes
