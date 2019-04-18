
#include "Switch.hpp"
#include <Arduino.h>

void Switch_Controller::init(void) {
    init(115200);
}

void Switch_Controller::init(long baud) {
    Serial.begin(baud);
    delay(10);
    reset();
    delay(10);
}

void Switch_Controller::send_part(int pos) {
    char tmp[2];

    if(old[pos] == array[pos]) { // if there's no change
        return; // don't bother sending
    }

    tmp[0] = ((pos+1)<<4) | (array[pos]>>4);
    tmp[1] = array[pos] & 0b1111;
    Serial.write(tmp, 2);

    old[pos] = array[pos];
}

void Switch_Controller::send(void) {
    for(int i=0; i<7; i++) {
        send_part(i);
    }
}

void Switch_Controller::reset(void) {
    map.btn.reg = 0x00;
    map.hat     = 0x08;
    map.lx      = 0x80;
    map.ly      = 0x80;
    map.rx      = 0x80;
    map.ry      = 0x80;
    send();
}

void Switch_Controller::set_hat(void) {
    uint8_t val;

    if(hat_x < 0) { // left
        if(hat_y < 0) {
            val = 0x05; // down
        }
        else if(hat_y == 0) { // neutral
            val = 0x06;
        }
        else { // up
            val = 0x07;
        }
    }
    else if(hat_x == 0) { // neutral
        if(hat_y < 0) {
            val = 0x04;
        }
        else if(hat_y == 0) {
            val = 0x08;
        }
        else {
            val = 0x00;
        }
    }
    else { // right
        if(hat_y < 0) {
            val = 0x03;
        }
        else if(hat_y == 0) {
            val = 0x02;
        }
        else {
            val = 0x01;
        }
    }
    map.hat = val;
}
