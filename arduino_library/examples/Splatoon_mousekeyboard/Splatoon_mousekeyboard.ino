
// Programmed for mouse & keyboard Splatoon 2
/*
FOR DEMONSTRATION ONLY

This is complicated and hacky. Use an easier example.

Mouse & keyboard adapter for Splatoon 2.
Requires a USB host shield. 
This is a pretty hacky method of getting mouse
and keyboard to work, but it shows off several
features of the Switch adapter. This will not work
without modification unless you have my exact mouse
and keyboard.
A regular usb keyboard will probably work with this
driver, you just need to add the vid and pid to the
device_list. 
*/

#include <SPI.h>

#include <usbhub.h>
#include "Switch.hpp"
#include "MyHID.hpp"
#include "keycodes.h"

Switch_Controller Switch;

typedef union {
  struct {
    uint16_t left:1;
    uint16_t right:1;
    uint16_t middle:1;
    uint16_t side:1;
    uint16_t extra:1;
    uint16_t :3;
  } bit;
  uint16_t reg;
} my_mouse_btn_t;

typedef struct {
  my_mouse_btn_t btn;
  int16_t rel_x;
  int16_t rel_y;
  int8_t rel_wheel;
} my_mouse_t;

uint8_t MouseInitSuccessful(void) {
  PRINT("Gaming mouse connected");
}

int js_x(int left, int right) {
  int ret = 0x80;
  
  if(left && right) {
    
  }
  else if(left) {
    ret = 0;
  }
  else if(right) {
    ret = 0xff;
  }
  return ret;
}
int js_y(int up, int down) {
  int ret = 0x80;
  if(up && down) {
    
  }
  else if(up) {
    ret = 0x00;
  }
  else if(down) {
    ret = 0xff;
  }
  return ret;
}

class MouseToJoystickAxis {
  public:
    int val;
    
    void Task() {
      unsigned long now;
      unsigned long elapsed;

      now = millis();
      elapsed = now - start;
      if(elapsed >= dt) {
        start = now;
        val = js_mid;
      }
      
    }
    void movement(int m) {
      
      if(m == 0) {
        val = js_mid;
      }
      else {
        val = map(abs(m), 1, mouse_max, js_deadzone, js_max-js_mid);
        if(m < 0) {
          val = js_mid - val;
        }
        else {
          val += js_mid;
        }
        if(val < js_min) {
          val = js_min;
        }
        else if(val > js_max) {
          val = js_max;
        }
      }
      start = millis(); // reset the timer
    }
  private:
    unsigned long start=0;
    const int js_min = 0;
    const int js_mid = 0x80;
    const int js_max = 0xff;
    const int js_deadzone = 0x15;
    const int mouse_max = 20;
    const int dt = 100; // milliseconds
     
};

MouseToJoystickAxis rx_converter;
MouseToJoystickAxis ry_converter;

void MouseParseHID(uint8_t* buf, uint8_t len) {
  my_mouse_t data;
  #ifdef DEBUG
  char bufout[50];
  #endif
  
  memcpy(&data, buf, min(len, sizeof(my_mouse_t)));
  #ifdef DEBUG
  sprintf(bufout, "%i %i %i", data.rel_x, data.rel_y, data.rel_wheel);
  PRINT(bufout);
  #endif

  // buttons and movement are all parsed, deal with them however!

  Switch.map.btn.bit.ZL = data.btn.bit.side; // squid form = side button
  Switch.map.btn.bit.ZR = data.btn.bit.left; // shoot = left
  Switch.map.btn.bit.R  = data.btn.bit.right; // bomb = right
  Switch.map.btn.bit.RCLICK = data.btn.bit.extra; // special = front side button

  rx_converter.movement(data.rel_x);
  ry_converter.movement(data.rel_y);
  Switch.map.rx = rx_converter.val;
  Switch.map.ry = rx_converter.val;
  #ifndef DEBUG
  Switch.send();
  #endif
}

uint8_t KeyboardInitSuccessful(void) {
  PRINT("Keyboard connected");
}

/*
void KeyboardParseHIDRaw(uint8_t* buf, uint8_t len) { // this is purely for debugging
  char outbuf[100] = {0};
  char smallbuff[30] = {0};

  for(int i=0; i<8; i++) {
    sprintf(smallbuff, "%02X ", buf[i]);
    //sprintf(smallbuff, "%i ", usb_kbd_keycode[buf[i]]);
    strcat(outbuf, smallbuff);
  }
  PRINT(outbuf);
}
*/



void KeyboardParseHID(uint8_t* buf, uint8_t len) {
  char keys[KEY_MAX_] = {0};

  for(int i=2; i<8; i++) {
    keys[usb_kbd_keycode[buf[i]]] = 1;
  }
  keys[KEY_LEFTCTRL]   |= (buf[0]>>0) & 1;
  keys[KEY_LEFTSHIFT]  |= (buf[0]>>1) & 1;
  keys[KEY_LEFTALT]    |= (buf[0]>>2) & 1;
  keys[KEY_LEFTMETA]   |= (buf[0]>>3) & 1;
  keys[KEY_RIGHTCTRL]  |= (buf[0]>>4) & 1;
  keys[KEY_RIGHTSHIFT] |= (buf[0]>>5) & 1;
  keys[KEY_RIGHTALT]   |= (buf[0]>>6) & 1;

  // keys are all parsed, deal with them however!
  
  Switch.map.lx = js_x(keys[KEY_A], keys[KEY_D]);
  Switch.map.ly = js_y(keys[KEY_W], keys[KEY_S]);

  Switch.map.btn.bit.A = keys[KEY_K];
  Switch.map.btn.bit.B = keys[KEY_J] | keys[KEY_SPACE];
  Switch.map.btn.bit.X = keys[KEY_I] | keys[KEY_R];
  Switch.map.btn.bit.Y = keys[KEY_U];
  Switch.map.btn.bit.R = keys[KEY_8];
  Switch.map.btn.bit.L = keys[KEY_7];
  Switch.map.btn.bit.PLUS = keys[KEY_ENTER];
  Switch.map.btn.bit.MINUS = keys[KEY_RIGHTSHIFT];
  Switch.map.btn.bit.HOME = keys[KEY_ESC];
  Switch.map.btn.bit.CAPTURE = keys[KEY_F1];

  Switch.hat_x = -keys[KEY_LEFT] + keys[KEY_RIGHT];
  Switch.hat_y = -keys[KEY_DOWN] + keys[KEY_UP];
  Switch.set_hat();

  #ifndef DEBUG
  Switch.send();
  #endif
}


const static hid_device_t device_list[] = {
  { { 0x04D9, 0xFC38 }, &MouseInitSuccessful, &MouseParseHID, 0 }, // my gaming mouse
  { { 0x046D, 0xC316 }, &KeyboardInitSuccessful, &KeyboardParseHID, 0 }, // my wired keyboard
  { { 0x0000, 0x0000 }, NULL, NULL, 1 }, // end of list
};

USB Usb;
USBHub Hub(&Usb);
USBHub Hub1(&Usb);
MyHID myhid1(&Usb, device_list);
MyHID myhid2(&Usb, device_list);
MyHID myhid3(&Usb, device_list);
MyHID myhid4(&Usb, device_list);

void setup() {
  BEGIN;
  PRINT("Start");
  #ifndef DEBUG
  Switch.init();
  #endif
  if(Usb.Init() == -1) {
    PRINT("OSC didn't start");
  }
  delay(200);
}

void loop() {
  Usb.Task();
  rx_converter.Task();
  ry_converter.Task();
  #ifndef DEBUG
  Switch.map.rx = rx_converter.val;
  Switch.map.ry = ry_converter.val;
  Switch.send();
  #endif
}
