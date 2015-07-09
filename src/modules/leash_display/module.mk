MODULE_COMMAND		= leash_display

SRCS			= \
    font.cpp \
    main.cpp \
    block.cpp \
    screen.cpp \
    status.cpp \
    images/images.c

CXXFLAGS		+= -std=c++11 -Dmain=${MODULE_COMMAND}_main \
    -Werror \
    -Wno-error=shadow

