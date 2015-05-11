#
# Makefile for the px4fmu_default configuration
#

#
# Use the configuration's ROMFS, copy the px4iov2 firmware into
# the ROMFS if it's available
#
ROMFS_ROOT	 = $(PX4_BASE)/ROMFS/AirDogFMU

# IO Chip -- px4io firmware
ROMFS_OPTIONAL_FILES = $(PX4_BASE)/Images/px4io-v2_default.bin

# IO Chip -- AirDog custom firmware
#ROMFS_OPTIONAL_FILES = $(PX4_BASE)/Images/AirDogIO_default.bin

# With custom SiK firmware
#ROMFS_OPTIONAL_FILES += $(wildcard $(PX4_BASE)/ROMFS/FW/*)

#
# Board support modules
#
#MODULES		+= drivers/bluetooth21
MODULES		+= drivers/boards/AirDogFMU/$(CONFIG_BOARD_REVISION)
MODULES		+= drivers/calibration
MODULES		+= drivers/device
MODULES		+= drivers/frsky_telemetry
MODULES		+= drivers/gps
MODULES		+= drivers/hmc5883spi
MODULES		+= drivers/l3gd20
MODULES		+= drivers/led
MODULES		+= drivers/ll905
MODULES		+= drivers/lsm303d
MODULES		+= drivers/mpu6000
MODULES		+= drivers/ms5611
MODULES		+= drivers/px4io
MODULES		+= drivers/stm32
MODULES		+= drivers/stm32/adc
MODULES		+= drivers/stm32/tone_alarm
MODULES		+= modules/airdog
MODULES		+= modules/debug_button
MODULES		+= modules/gpio_tool
MODULES		+= modules/sensors
MODULES		+= modules/sensors_probe
MODULES		+= modules/sensors_switch
MODULES		+= modules/spi_exchange
#MODULES		+= drivers/airspeed
#MODULES		+= drivers/blinkm
#MODULES		+= drivers/ets_airspeed
#MODULES		+= drivers/hil
#MODULES		+= drivers/hmc5883
#MODULES		+= drivers/hott/hott_sensors
#MODULES		+= drivers/hott/hott_telemetry
#MODULES		+= drivers/mb1230serial
#MODULES		+= drivers/mb12xx
#MODULES		+= drivers/meas_airspeed
#MODULES		+= drivers/pca8574
#MODULES		+= drivers/px4flow
#MODULES		+= drivers/px4fmu
#MODULES		+= drivers/rgbled
#MODULES		+= drivers/sf0x
#MODULES		+= modules/SiKUploader

# Needs to be burned to the ground and re-written; for now,
# just don't build it.
#MODULES		+= drivers/mkblctrl

#
# System commands
#
MODULES		+= modules/airdog/calibrator
MODULES		+= systemcmds/bl_update
MODULES		+= systemcmds/boardinfo
MODULES		+= systemcmds/config
MODULES		+= systemcmds/dumpfile
MODULES		+= systemcmds/esc_calib
MODULES		+= systemcmds/mixer
MODULES		+= systemcmds/mtd
MODULES		+= systemcmds/nshterm
MODULES		+= systemcmds/param
MODULES		+= systemcmds/perf
MODULES		+= systemcmds/preflight_check
MODULES		+= systemcmds/pwm
MODULES		+= systemcmds/reboot
MODULES		+= systemcmds/top
MODULES		+= systemcmds/ver
MODULES		+= systemcmds/writefile
#MODULES		+= systemcmds/tests

#
# General system control
#
MODULES		+= modules/commander
MODULES		+= modules/gpio_led
MODULES		+= modules/mavlink
MODULES		+= modules/navigator
#MODULES		+= modules/uavcan

#
# Estimation modules (EKF/ SO3 / other filters)
#
MODULES		+= modules/attitude_estimator_ekf
MODULES		+= modules/position_estimator_inav
#MODULES		+= examples/flow_position_estimator
#MODULES		+= modules/airdog/trajectory_calculator
#MODULES		+= modules/attitude_estimator_so3
#MODULES		+= modules/ekf_att_pos_estimator

#
# Vehicle Control
#
MODULES		+= modules/mc_att_control
MODULES		+= modules/mc_pos_control

#
# Logging
#
MODULES		+= modules/sdlog2

#
# Unit tests
#
#MODULES 	+= modules/commander/commander_tests
#MODULES 	+= modules/unit_test

#
# Library modules
#
MODULES		+= modules/controllib
MODULES		+= modules/dataman
MODULES		+= modules/systemlib
MODULES		+= modules/systemlib/mixer
MODULES		+= modules/uORB

#
# Libraries
#
MODULES		+= lib/conversion
MODULES		+= lib/geo
MODULES		+= lib/geo_lookup
MODULES		+= lib/mathlib
MODULES		+= lib/mathlib/math/filter
LIBRARIES	+= lib/mathlib/CMSIS
#MODULES		+= lib/ecl
#MODULES		+= lib/external_lgpl
#MODULES		+= lib/launchdetection

#
# OBC challenge
#
#MODULES		+= modules/bottle_drop

#
# Demo apps
#
#MODULES		+= examples/math_demo
# Tutorial code from
# https://pixhawk.ethz.ch/px4/dev/hello_sky
#MODULES		+= examples/px4_simple_app

# Tutorial code from
# https://pixhawk.ethz.ch/px4/dev/daemon
#MODULES		+= examples/px4_daemon_app

# Tutorial code from
# https://pixhawk.ethz.ch/px4/dev/debug_values
#MODULES		+= examples/px4_mavlink_debug

# Tutorial code from
# https://pixhawk.ethz.ch/px4/dev/example_fixedwing_control
#MODULES		+= examples/fixedwing_control

# Hardware test
#MODULES		+= examples/hwtest
#MODULES		+= modules/serial_echo

#
# Transitional support - add commands from the NuttX export archive.
#
# In general, these should move to modules over time.
#
# Each entry here is <command>.<priority>.<stacksize>.<entrypoint> but we use a helper macro
# to make the table a bit more readable.
#
define _B
	$(strip $1).$(or $(strip $2),SCHED_PRIORITY_DEFAULT).$(or $(strip $3),CONFIG_PTHREAD_STACK_DEFAULT).$(strip $4)
endef

#                  command                 priority                   stack  entrypoint
BUILTIN_COMMANDS := \
	$(call _B, sercon,                 ,                          2048,  sercon_main                ) \
	$(call _B, serdis,                 ,                          2048,  serdis_main                )
