
The Arduino Uno and Mega uses an atmega16u2 (atmega8u2 on older models) as a USB to serial converter. You can use this USB microcontroller as the Switch controller, which you then talk to using a regular Uno/Mega program.

This requires updating the firmware on the serial converter. They made it relatively easy by using a DFU bootloader. See this page for how to upload a new program to this secondary chip: https://www.arduino.cc/en/Hacking/DFUProgramming8U2

Note that you cannot update the code on your Uno/Mega while the USB mcu is running the Switch UART code. So you need to put on the default serial code, flash your Uno/Mega, then put back the Switch code.

The Uno/Mega talks to the USB mcu through the default UART (Serial 0) port, so you should not send any through Serial 0. If you need communication, either use software serial (Uno) or a different Serial port (Mega).
