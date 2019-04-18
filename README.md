
Switch-UART-Controller
======================

This allows an AVR with native USB support to mimick a wired Switch controller. A different device tells the AVR what buttons/joysticks to press over UART. It uses a default baud of 115200, but this can be edited in the makefile. 

Uses
====

This can be used to easily make a custom controller using any computer. A desktop can talk to it through a UART adapter, an Arduino Uno can be used as a custom controller (see the arduino_library folder), etc.

A way to easily flash devices with a DFU bootloader can be found in the flash_helper/ folder. 

UART Commands
=============

Commands are purely used for button presses. A 4-bit address is sent alongside the 8-bit corresponding value, split between two 8-bit packets. It is sent like so:
aaaavvvv 0000vvvv

Where aaaa is the 4-bit address , and vvvv vvvv is the 8-bit value to be written to that address, split in half. 0000 is required. 

So say you want to set the left joystick y-axis to 0x3C. The address 0x05 = 0b0101, the value is 0x3C = 0b00111100.
The first byte is the address shifted right by 4 and the upper 4 bits of the value downshifted, so 0b01010011.
The second byte is the lower 4 bits of the value, so 0b00001100.
0b01010011 0b00001100 = 0x53 0x0C. 

There are a few other UART Switch projects out there with more human-readable command set, but this is very efficient and easy to program. 

Address Map
-----------


| Address  |  Name  | Description                               | Unpressed Value |
|----------|--------|-------------------------------------------|-----------------|
|   01h    |  BTN_0 | Buttons such as Minus, Home, and Capture. |      00h        |
|   02h    |  BTN_1 | Buttons such as A, B, and X.              |      00h        |
|   03h    |   HAT  | Directional pad.                          |      08h        |
|   04h    |   LX   | Left joystick x-axis.                     |      80h        |
|   05h    |   LY   | Left joystick y-axis.                     |      80h        |
|   06h    |   RX   | Right joystick x-axis.                    |      80h        |
|   07h    |   RY   | Right joystick y-axis.                    |      80h        |

### 00h - BTN_0

Presses or releases the button. 0 is release, 1 is press. 

| Bit | Button     |
|-----|------------|
| 7:6 | [reserved] |
|  5  | CAPTURE    |
|  4  | HOME       |
|  3  | RCLICK     |
|  2  | LCLICK     |
|  1  | PLUS       |
|  0  | MINUS      |

### 01h - BTN_1

Presses or releases the button. 0 is release, 1 is press. 

| Bit | Button |
|-----|--------|
|  7  | ZR     |
|  6  | ZL     |
|  5  | R      |
|  4  | L      |
|  3  | X      |
|  2  | A      |
|  1  | B      |
|  0  | Y      |

### 02h - HAT

Control the Directional Pad (d-pad). 

| Value | Direction  |
|-------|------------|
|  00h  | Up         |
|  01h  | Up-Right   |
|  02h  | Right      |
|  03h  | Down-Right |
|  04h  | Down       |
|  05h  | Down-Left  |
|  06h  | Left       |
|  07h  | Up-Left    |
|  08h  | Neutral    |

### 03h:06h

Registers 03h through 06h control the joysticks. 00h corresponds to completely left/down, 80h is neutral, and FFh is completely right/up. 
