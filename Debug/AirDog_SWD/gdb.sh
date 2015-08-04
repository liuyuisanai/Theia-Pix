#
# Run from firmware "root".  It requires ./Debug catalog.
#
# Usage:
#
#  1. run ./Debug/AirDog_SWD/openocd.sh in background or other terminal/screen
#
#     $ ./Debug/AirDog_SWD/openocd.sh
#
#  2. run ./Debug/AirDog_SWD/gdb.sh <firmware.elf> like
#
#     $ ./Debug/AirDog_SWD/gdb.sh Build/AirDogFMU.build/firmware.elf
#

arm-none-eabi-gdb -x ./Debug/AirDog_SWD/gdb.openocd
