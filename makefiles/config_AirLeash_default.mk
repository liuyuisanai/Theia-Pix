#
# Makefile for the px4fmu_default configuration
#

#
# Use the configuration's ROMFS, copy the px4iov2 firmware into
# the ROMFS if it's available
#
# TODO: crete clean airleash romfs root
ROMFS_ROOT	 = $(PX4_BASE)/ROMFS/AirLeash

# With custom SiK firmware
#ROMFS_OPTIONAL_FILES += $(wildcard $(PX4_BASE)/ROMFS/FW/*)

#
# Board support modules
#
MODULES		+= drivers/boards/AirLeash
MODULES		+= drivers/boards/AirLeash/kbd
MODULES		+= drivers/device
MODULES		+= drivers/gps
MODULES		+= drivers/l3gd20
MODULES		+= drivers/led
MODULES		+= drivers/lsm303d
MODULES		+= drivers/mpu6000
MODULES		+= drivers/ms5611
MODULES		+= drivers/stm32
MODULES		+= drivers/stm32/adc
MODULES		+= drivers/stm32/tone_alarm
MODULES		+= modules/airdog/trajectory_calculator
MODULES		+= modules/gpio_tool
MODULES		+= modules/kbd_test
MODULES		+= modules/sensors
MODULES		+= modules/sensors_probe
MODULES		+= modules/sensors_switch
MODULES		+= modules/spi_exchange
#MODULES		+= drivers/airspeed
#MODULES		+= drivers/blinkm
#MODULES		+= drivers/ets_airspeed
#MODULES		+= drivers/frsky_telemetry
#MODULES		+= drivers/hil
#MODULES		+= drivers/hmc5883
#MODULES		+= drivers/hott/hott_sensors
#MODULES		+= drivers/hott/hott_telemetry
#MODULES		+= drivers/ll40ls
#MODULES		+= drivers/mb12xx
#MODULES		+= drivers/mb1230serial
#MODULES		+= drivers/meas_airspeed
#MODULES		+= drivers/pca8574
#MODULES		+= drivers/px4fmu
#MODULES		+= drivers/px4io
#MODULES		+= drivers/rgbled
#MODULES		+= drivers/sf0x

# Needs to be burned to the ground and re-written; for now,
# just don't build it.
#MODULES		+= drivers/mkblctrl

#
# System commands
#
MODULES		+= systemcmds/bl_update
MODULES		+= systemcmds/boardinfo
MODULES		+= systemcmds/config
MODULES		+= systemcmds/dumpfile
MODULES		+= systemcmds/mtd
MODULES		+= systemcmds/nshterm
MODULES		+= systemcmds/param
MODULES		+= systemcmds/perf
MODULES		+= systemcmds/preflight_check
MODULES		+= systemcmds/reboot
MODULES		+= systemcmds/top
MODULES		+= systemcmds/ver
#MODULES		+= systemcmds/esc_calib
#MODULES		+= systemcmds/mixer
#MODULES		+= systemcmds/pwm
#MODULES		+= systemcmds/tests
#MODULES		+= systemcmds/writefile

#
# General system control
#
# TODO remove commander
MODULES		+= modules/commander
MODULES		+= modules/mavlink
#MODULES		+= modules/navigator
#MODULES		+= modules/gpio_led
#MODULES		+= modules/uavcan

# TODO rewrite airdog
#MODULES 	+= modules/airdog

#
# Estimation modules (EKF/ SO3 / other filters)
#
MODULES		+= modules/attitude_estimator_ekf
MODULES		+= modules/position_estimator_inav
#MODULES		+= modules/attitude_estimator_so3
#MODULES		+= modules/ekf_att_pos_estimator
#MODULES		+= examples/flow_position_estimator

#
# Vehicle Control
#
#MODULES		+= modules/segway # XXX Needs GCC 4.7 fix
#MODULES		+= modules/fw_pos_control_l1
#MODULES		+= modules/fw_att_control
#MODULES		+= modules/mc_att_control
#MODULES		+= modules/mc_pos_control

#
# Logging
#
# There is no filesystem.
#MODULES		+= modules/sdlog2
#MODULES		+= modules/sdlog2_lite

#
# Unit tests
#
#MODULES 	+= modules/unit_test
#MODULES 	+= modules/commander/commander_tests

#
# Library modules
#
MODULES		+= modules/systemlib
MODULES		+= modules/controllib
MODULES		+= modules/uORB
MODULES		+= modules/dataman # required by mavlink
#MODULES		+= modules/systemlib/mixer

#
# Libraries
#
LIBRARIES	+= lib/mathlib/CMSIS
MODULES		+= lib/mathlib
MODULES		+= lib/mathlib/math/filter
MODULES		+= lib/geo
MODULES		+= lib/geo_lookup
MODULES		+= lib/conversion
#MODULES		+= lib/ecl
#MODULES		+= lib/external_lgpl
#MODULES		+= lib/launchdetection

# Hardware test
#MODULES			+= examples/hwtest

# Hardware test
#MODULES			+= examples/hwtest

# Airdog modules
#MODULES			+= modules/bt_cfg
#MODULES			+= modules/serial_echo
#MODULES			+= modules/serial_measure_latency
#MODULES			+= modules/SiKUploader

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
