MODULE_COMMAND		= $(notdir $(shell pwd))

SRCS			= \
    modes/base.cpp \
    modes/calibrate.cpp \
    modes/list.cpp \
    modes/logo.cpp \
    modes/main.cpp \
    modes/menu.cpp \
    modes/connect.cpp \
    modes/acquiring_gps.cpp \
    main.cpp  \
    uorb_functions.cpp  \
    datamanager.cpp \
    displayhelper.cpp \
    button_handler.cpp \


DEFAULT_VISIBILITY = protected
CXXFLAGS		+= -std=c++11 -Dmain=${MODULE_COMMAND}_main
