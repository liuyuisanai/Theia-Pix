MODULE_COMMAND = calibrator
SRCS = calibrator.cpp \
	mag_calibration.cpp \
	gyro_calibration.cpp \
	accel_calibration.cpp

CXXFLAGS += -std=gnu++11
