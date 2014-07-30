MODULE_COMMAND = airdog
SRCS = airdog.cpp \
	   i2c_controller.cpp \
	   i2c_display_controller.cpp \
	   menu_controller.cpp \
	   paramhandler.cpp \
	   button_controller.cpp \
	   airdog_params.c

CFLAGS += -Wno-unknown-pragmas -Wno-packed
