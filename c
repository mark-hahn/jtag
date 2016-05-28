#   -*-  grammar-ext: sh  -*-
cat c

export ARCHITECTURE=avr
export ARDUINO_DIR=~/dev/arduino/arduino-1.6.8
export ARDUINO_PORT=/dev/ttyACM0
export ARDUINO_LIBS="NeoHWSerial"
# export ARDUINO_QUIET=true

make
