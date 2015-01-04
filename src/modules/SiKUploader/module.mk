MODULE_COMMAND		= $(notdir $(shell pwd))

SRCS			= binary_format.cpp flash.cpp main.cpp

EXTRACXXFLAGS		+= -std=c++11 \
			   -Dmain=${MODULE_COMMAND}_main \
			# end of EXTRACXXFLAGS
