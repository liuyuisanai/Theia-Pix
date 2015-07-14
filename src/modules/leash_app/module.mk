MODULE_COMMAND		= $(notdir $(shell pwd))

SRCS			= \
    modes/base.cpp \
    modes/logo.cpp \
    modes/main.cpp \
    modes/menu.cpp \
    main.cpp  \
    datamanager.cpp \
    displayhelper.cpp \
	button_handler.cpp

CXXFLAGS		+= -std=c++11 -Dmain=${MODULE_COMMAND}_main
