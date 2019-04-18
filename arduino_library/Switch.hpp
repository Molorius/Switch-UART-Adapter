
#ifndef SWITCH_HPP_
#define SWITCH_HPP_

#include <stdint.h>

typedef union {
	struct {
		uint16_t Y:1;
		uint16_t B:1;
		uint16_t A:1;
		uint16_t X:1;
		uint16_t L:1;
		uint16_t R:1;
		uint16_t ZL:1;
		uint16_t ZR:1;
		uint16_t MINUS:1;
		uint16_t PLUS:1;
		uint16_t LCLICK:1;
		uint16_t RCLICK:1;
		uint16_t HOME:1;
		uint16_t CAPTURE:1;
		uint16_t :2;
	} bit;
	uint16_t reg;
    struct {
        uint8_t ONE;
        uint8_t ZERO;
    } pos;
} _BTN_t;

typedef struct {
	_BTN_t btn;
	uint8_t hat;
	uint8_t lx;
	uint8_t ly;
	uint8_t rx;
	uint8_t ry;
	uint8_t vendorspec;
} map_t;

class Switch_Controller {
    public:
        void init(void);
        void init(long baud);
        void reset(void);

        void send(void);
		
		int hat_x = 0;
		int hat_y = 0;
		void set_hat(void);

        map_t map;
    private:
        uint8_t* array = (uint8_t*) &map.btn.pos.ONE;
		uint8_t old[7] = { 1 }; // start with ones so there's a change
        void send_part(int pos);
        void send_char(uint16_t data);
};


#endif