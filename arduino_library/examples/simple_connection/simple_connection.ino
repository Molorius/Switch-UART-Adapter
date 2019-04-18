
/*
This simply presses and releases L and R forever.
Used as a simple example and as a way to test that
the Switch recognizes your Arduino.

Upload this to the main mcu while the USB mcu is 
running the default Arduino code, then change the
USB mcu to the Switch UART code. 
See https://www.arduino.cc/en/Hacking/DFUProgramming8U2
*/

#include "Switch.hpp"

Switch_Controller Switch;

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, 0);
    Switch.init(); // set up the communication with the Switch adapter
}

void loop() {
    digitalWrite(LED_BUILTIN, 1); // turn on the LED, for debugging
    Switch.map.btn.bit.L = 1; // press the L button
    Switch.map.btn.bit.R = 1; // press the R button
    Switch.send(); // send the updated values to the Switch
    delay(1000);

    digitalWrite(LED_BUILTIN, 0); // turn off the LED, for debugging
    Switch.map.btn.bit.L = 0; // release the L button
    Switch.map.btn.bit.R = 0; // release the R button
    Switch.send(); // send the updated values to the Switch
    delay(1000);
}
