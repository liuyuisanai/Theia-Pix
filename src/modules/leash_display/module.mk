MODULE_COMMAND = leash_display

SRCS = \
    block.cpp \
    datamanager.cpp \
    errormessages.cpp \
    images/images.c \
    font.cpp \
    main.cpp \
    screen.cpp

CXXFLAGS += -std=c++11 -Dmain=${MODULE_COMMAND}_main \
    -Werror \
    -Wno-error=shadow

