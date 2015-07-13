MODULE_COMMAND		= $(notdir $(shell pwd))

SRCS			= \
    modes/base.cpp \
    modes/logo.cpp \
    main.cpp  \
    datamanager.cpp \
    displayhelper.cpp

CXXFLAGS		+= -std=c++11 -Dmain=${MODULE_COMMAND}_main
