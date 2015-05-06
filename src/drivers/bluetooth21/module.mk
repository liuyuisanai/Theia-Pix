#
# You can add the following argiments to make's command line
#
# * BLUETOOTH21_DEBUG=yes -- enables debug output from the code.
#
# * BLUETOOTH21_TRACE_IO=stderr -- enables writing full communication trace
#   to a bluetooth module to stderr.
#
# * BLUETOOTH21_TRACE_IO=file   -- enables writing full communication trace
#   to a bluetooth module to a file.
#
# * BLUETOOTH21_TRACE_SERVICE=stderr -- enables writing trace of service
#   commands, responses and events to stderr.
#
# * BLUETOOTH21_TRACE_SERVICE=file   -- enables writing trace of service
#   commands, responses and events to a file.
#
# Usage example:
#
#   make px4fmu-v2_default BLUETOOTH21_DEBUG=yes BLUETOOTH21_TRACE_IO=stderr \
#   BLUETOOTH21_TRACE_SERVICE=stderr
#
# or
#
#   export BLUETOOTH21_DEBUG=yes
#   export BLUETOOTH21_TRACE_IO=stderr
#   export BLUETOOTH21_TRACE_SERVICE=stderr
#   make px4fmu-v2_default
#
# Changing any of these settings requires _clean_ module rebuild.
#

ifneq ($(DEBUG_BLUETOOTH21),)
$(error Use BLUETOOTH21_DEBUG=yes)
endif

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

ifeq ($(BLUETOOTH21_DEBUG),yes)
EXTRACXXFLAGS += -DCONFIG_DEBUG_BLUETOOTH21
else ifneq ($(BLUETOOTH21_DEBUG),)
$(error Invalid BLUETOOTH21_DEBUG value: $(BLUETOOTH21_DEBUG))
endif

ifeq ($(BLUETOOTH21_TRACE_IO),)
EXTRACXXFLAGS += -DMULTIPLEXER_TRACE=BT::DebugTraceKind::NO_TRACE
else ifeq ($(BLUETOOTH21_TRACE_IO),stderr)
EXTRACXXFLAGS += -DMULTIPLEXER_TRACE=BT::DebugTraceKind::STDERR
else ifeq ($(BLUETOOTH21_TRACE_IO),file)
EXTRACXXFLAGS += -DMULTIPLEXER_TRACE=BT::DebugTraceKind::FILE
else
$(error Invalid BLUETOOTH21_TRACE_IO value: $(BLUETOOTH21_TRACE_IO))
endif

ifeq ($(BLUETOOTH21_TRACE_SERVICE),)
EXTRACXXFLAGS += -DSERVICE_TRACE=BT::DebugTraceKind::NO_TRACE
else ifeq ($(BLUETOOTH21_TRACE_SERVICE),stderr)
EXTRACXXFLAGS += -DSERVICE_TRACE=BT::DebugTraceKind::STDERR
else ifeq ($(BLUETOOTH21_TRACE_SERVICE),file)
EXTRACXXFLAGS += -DSERVICE_TRACE=BT::DebugTraceKind::FILE
else
$(error Invalid BLUETOOTH21_TRACE_SERVICE value: $(BLUETOOTH21_TRACE_SERVICE))
endif
