STACKSIZE_DAEMON_MAIN = 1024
STACKSIZE_DAEMON_IO = 2048
STACKSIZE_DAEMON_SERVICE = 4096
MODULE_COMMAND = $(notdir $(shell pwd))

SRCS = main.cpp \
	chardev.cpp \
	chardev_poll.cpp \
	daemon_main.cpp \
	daemon_multiplexer.cpp \
	daemon_service.cpp \
	device_connection_map.cpp \
	factory_addresses.cpp \
	io_multiplexer_global.cpp \
	laird/module_params.c \
	module_params.c \
	mutex.cpp \
# end of SRCS

EXTRACXXFLAGS += -std=c++11 \
		-DMODULE_COMMAND=$(MODULE_COMMAND) \
		-DSTACKSIZE_DAEMON_MAIN=$(STACKSIZE_DAEMON_MAIN) \
		-DSTACKSIZE_DAEMON_IO=$(STACKSIZE_DAEMON_IO) \
		-DSTACKSIZE_DAEMON_SERVICE=$(STACKSIZE_DAEMON_SERVICE) \
		-flto \
		-Werror \
# end of EXTRACXXFLAGS
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
