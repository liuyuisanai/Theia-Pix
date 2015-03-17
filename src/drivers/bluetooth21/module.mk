MODULE_COMMAND = $(notdir $(shell pwd))

SRCS = main.cpp \
	chardev.cpp \
	chardev_poll.cpp \
	daemon_multiplexer.cpp \
	daemon_service.cpp \
	factory_addresses.cpp \
	io_multiplexer_global.cpp \
	laird/module_params.c \
	mutex.cpp \
# end of SRCS

EXTRACXXFLAGS += -std=c++11 -DMODULE_COMMAND=${MODULE_COMMAND} -Werror -flto
SHOW_ALL_ERRORS = yes
TOLERATE_MISSING_DECLARATION = yes

ifneq ($(DEBUG_BLUETOOTH21),)
#
# To enable debug output add DEBUG_BLUETOOTH21=yes to make arguments,
# like follows
#
#   make px4fmu-v2_default DEBUG_BLUETOOTH21=yes
#
# or define environment variable
#
#   export DEBUG_BLUETOOTH21=yes
#   make px4fmu-v2_default
#
# Switching on and off the debug requires _clean_ module rebuild.
#
EXTRACXXFLAGS += -DCONFIG_DEBUG_BLUETOOTH21
endif
