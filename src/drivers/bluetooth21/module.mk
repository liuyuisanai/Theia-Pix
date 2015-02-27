MODULE_COMMAND = $(notdir $(shell pwd))

SRCS = \
	chardev.cpp \
	daemon_multiplexer.cpp \
	io_multiplexer_global.cpp \
	main.cpp \
	mutex.cpp \
	# end of SRCS

EXTRACXXFLAGS += -std=c++11 -DMODULE_COMMAND=${MODULE_COMMAND}
