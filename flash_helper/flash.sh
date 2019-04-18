#!/usr/bin/env bash

mcu=atmega16u2
#mcu=atmega32u4
#file=../Joystick.hex # fresh build
#file=../builds/Joystick-atmega32u4.hex # leonardo, teensy 2.0, etc
file=../builds/Joystick-atmega16u2.hex # uno, mega, etc
#file=Arduino-usbserial-atmega16u2-Uno-Rev3.hex # the original hex file for the uno

echo "Erasing..."
dfu-programmer $mcu erase
echo "Flashing $file..."
dfu-programmer $mcu flash $file
