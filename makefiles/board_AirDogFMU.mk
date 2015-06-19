#
# Board-specific definitions for the AirDogFMU
#

#
# Configure the toolchain
#
CONFIG_ARCH			 = CORTEXM4F
CONFIG_BOARD			 = AIRDOG_FMU

include $(PX4_MK_DIR)/toolchain_gnu-arm-eabi.mk
