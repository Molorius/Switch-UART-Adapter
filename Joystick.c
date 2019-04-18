/*
             LUFA Library
     Copyright (C) Dean Camera, 2019.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/

/*
  Copyright 2019  Dean Camera (dean [at] fourwalledcubicle [dot] com)

  Permission to use, copy, modify, distribute, and sell this
  software and its documentation for any purpose is hereby granted
  without fee, provided that the above copyright notice appear in
  all copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaims all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/

/** \file
 *
 *  Main source file for the Joystick demo. This file contains the main tasks of
 *  the demo and is responsible for the initial application hardware configuration.
 */

#include <util/delay.h>
#include "Joystick.h"
#include "uart.h"

#define LEDS_TX          LEDS_LED1
#define LEDS_RX          LEDS_LED2

/** Buffer to hold the previously generated HID report, for comparison purposes inside the HID class driver. */
static uint8_t PrevJoystickHIDReportBuffer[sizeof(hori_t)];
static volatile hori_t hori_joystick;
static volatile char hori_array[8];

static inline void arduino_setup(void) {
	DDRD  |= (1<<7); // set reset line to output
	PORTD |= (1<<7); // pull reset line high
}
static inline void arduino_on(void) {
	PORTD |= (1<<7); // pull reset line high
}
static inline void arduino_off(void) {
	PORTD &= ~(1<<7); // pull reset line low
}

hori_t hori_reset(void) {
	hori_t ret;
	ret.btn.reg = 0;
	ret.hat = 0x08;
	ret.lx = 0x80;
	ret.ly = 0x80;
	ret.rx = 0x80;
	ret.ry = 0x80;
	return ret;
}


/** LUFA HID Class driver interface configuration and state information. This structure is
 *  passed to all HID Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another.
 */
USB_ClassInfo_HID_Device_t Joystick_HID_Interface =
	{
		.Config =
			{
				.InterfaceNumber              = INTERFACE_ID_Joystick,
				.ReportINEndpoint             =
					{
						.Address              = JOYSTICK_EPADDR,
						.Size                 = JOYSTICK_EPSIZE,
						.Banks                = 1,
					},
				.PrevReportINBuffer           = PrevJoystickHIDReportBuffer,
				.PrevReportINBufferSize       = sizeof(PrevJoystickHIDReportBuffer),
			},
	};


/** Main program entry point. This routine contains the overall program flow, including initial
 *  setup of all components and the main program loop.
 */

typedef union {
	struct {
		uint8_t val:4; // bits 0.. 3
		uint8_t pos:4; // bits 4.. 7
	} bit;
	uint8_t reg;
} cmd_t;

static volatile uint16_t rx_val = 0;
static inline void rx_on(void) {
	rx_val = 0;
	LEDs_TurnOnLEDs(LEDS_RX);
}

void rx_off(void) {
	cli();
	rx_val++;
	if(rx_val > 2000) {
		rx_val = 0;
		LEDs_TurnOffLEDs(LEDS_RX);
	}
	sei();
}

int main(void)
{

	arduino_setup();
	SetupHardware();
	
	LEDs_TurnOffLEDs(LEDS_ALL_LEDS);
	//LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
	GlobalInterruptEnable();
	
	for (;;)
	{
		HID_Device_USBTask(&Joystick_HID_Interface);
		USB_USBTask();
		rx_off();
	}
}



ISR(USART1_RX_vect) {
	// ppppvvvv 0000vvvv
	// where p is the position in the array+1,
	// and v is the 8-bit value to be saved
	cmd_t c;
	static cmd_t last;

	rx_on();
	c.reg = UDR1; // get the byte
	if((c.bit.pos == 0) && (last.bit.pos > 0)) { // if we got a complete command
		hori_array[last.bit.pos-1] = (last.bit.val << 4) | c.bit.val; // save the value
	}
	last.reg = c.reg;
}

/** Configures the board hardware and chip peripherals for the demo's functionality. */
void SetupHardware(void)
{
#if (ARCH == ARCH_AVR8)
	/* Disable watchdog if enabled by bootloader/fuses */
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	/* Disable clock division */
	clock_prescale_set(clock_div_1);
#elif (ARCH == ARCH_XMEGA)
	/* Start the PLL to multiply the 2MHz RC oscillator to 32MHz and switch the CPU core to run from it */
	XMEGACLK_StartPLL(CLOCK_SRC_INT_RC2MHZ, 2000000, F_CPU);
	XMEGACLK_SetCPUClockSource(CLOCK_SRC_PLL);

	/* Start the 32MHz internal RC oscillator and start the DFLL to increase it to 48MHz using the USB SOF as a reference */
	XMEGACLK_StartInternalOscillator(CLOCK_SRC_INT_RC32MHZ);
	XMEGACLK_StartDFLL(CLOCK_SRC_INT_RC32MHZ, DFLL_REF_INT_USBSOF, F_USB);

	PMIC.CTRL = PMIC_LOLVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_HILVLEN_bm;

	
#endif

	/* Hardware Initialization */
	//Joystick_Init();
	LEDs_Init();
	//Buttons_Init();
	USB_Init();
	uart_init();
	//uart_attach_std();
	UCSR1B |= _BV(RXCIE1); // allow for usart rx interrupts

	// start timer to run usb task
	// OCR1A = (Input Frequency / Prescale) / Target Frequency - 1
	/*
	TCCR1B |= (1 << WGM12); // configure timer 1 for ctc mode
	TIMSK1 |= (1 << OCIE1A); // enable ctc interrupt
	OCR1A   = ((F_CPU / 64) / 1000) - 1; // 124; // ((F_CPU / 64) / 1000) - 1; // every ms
	TCCR1B |= ((1 << CS10) | (1 << CS11)); // start timer at fcpu/64 (prescaler=64)

	TCCR1B |= (1 << WGM12);
	*/
}

ISR(TIMER1_COMPA_vect) {
	
}

/** Event handler for the library USB Connection event. */
void EVENT_USB_Device_Connect(void)
{
	hori_t tmp;
	//LEDs_SetAllLEDs(LEDMASK_USB_ENUMERATING);
	LEDs_TurnOnLEDs(LEDS_TX);
	/*
	cli();
	hori_array[0] = 0;
	hori_array[1] = 0;
	hori_array[2] = 0x08;
	hori_array[3] = 0x80;
	hori_array[4] = 0x80;
	hori_array[5] = 0x80;
	hori_array[6] = 0x80;
	hori_array[7] = 0x80;
	sei();
	*/
}

/** Event handler for the library USB Disconnection event. */
void EVENT_USB_Device_Disconnect(void)
{
	//LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
	LEDs_TurnOffLEDs(LEDS_TX);
	arduino_off();
	cli();
	for(int i=0; i<8; i++) {
		hori_array[i] = 1;
	}
	sei();
	
	//memset((void*) hori_array, 0, sizeof(hori_t)); // zero it!
}

/** Event handler for the library USB Configuration Changed event. */
void EVENT_USB_Device_ConfigurationChanged(void)
{
	bool ConfigSuccess = true;

	ConfigSuccess &= HID_Device_ConfigureEndpoints(&Joystick_HID_Interface);

	USB_Device_EnableSOFEvents();
	if(ConfigSuccess) {
		arduino_on();
		LEDs_TurnOnLEDs(LEDS_TX);
	}
	else {
		arduino_off();
		LEDs_TurnOffLEDs(LEDS_TX);
	}

	//LEDs_SetAllLEDs(ConfigSuccess ? LEDMASK_USB_READY : LEDMASK_USB_ERROR);
}

/** Event handler for the library USB Control Request reception event. */
void EVENT_USB_Device_ControlRequest(void)
{
	HID_Device_ProcessControlRequest(&Joystick_HID_Interface);
}

/** Event handler for the USB device Start Of Frame event. */
void EVENT_USB_Device_StartOfFrame(void)
{
	HID_Device_MillisecondElapsed(&Joystick_HID_Interface);
}

/** HID class driver callback function for the creation of HID reports to the host.
 *
 *  \param[in]     HIDInterfaceInfo  Pointer to the HID class interface configuration structure being referenced
 *  \param[in,out] ReportID    Report ID requested by the host if non-zero, otherwise callback should set to the generated report ID
 *  \param[in]     ReportType  Type of the report to create, either HID_REPORT_ITEM_In or HID_REPORT_ITEM_Feature
 *  \param[out]    ReportData  Pointer to a buffer where the created report should be stored
 *  \param[out]    ReportSize  Number of bytes written in the report (or zero if no report is to be sent)
 *
 *  \return Boolean \c true to force the sending of the report, \c false to let the library determine if it needs to be sent
 */
bool CALLBACK_HID_Device_CreateHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
                                         uint8_t* const ReportID,
                                         const uint8_t ReportType,
                                         void* ReportData,
                                         uint16_t* const ReportSize)
{
	hori_t* JoystickReport = (hori_t*)ReportData;
	//*JoystickReport = hori_joystick;
	//char* JoystickReport = (char*)ReportData;
	//*JoystickReport = &hori_array[0];

	cli();
	memcpy(JoystickReport, (const void*)hori_array, sizeof(hori_array));
	sei();

	/*
	uint8_t JoyStatus_LCL    = Joystick_GetStatus();
	uint8_t ButtonStatus_LCL = Buttons_GetStatus();

	if (JoyStatus_LCL & JOY_UP)
	  JoystickReport->Y = -100;
	else if (JoyStatus_LCL & JOY_DOWN)
	  JoystickReport->Y =  100;

	if (JoyStatus_LCL & JOY_LEFT)
	  JoystickReport->X = -100;
	else if (JoyStatus_LCL & JOY_RIGHT)
	  JoystickReport->X =  100;

	if (JoyStatus_LCL & JOY_PRESS)
	  JoystickReport->Button |= (1 << 1);

	if (ButtonStatus_LCL & BUTTONS_BUTTON1)
	  JoystickReport->Button |= (1 << 0);

	*/
	*ReportSize = sizeof(hori_t);
	return false;
}

/** HID class driver callback function for the processing of HID reports from the host.
 *
 *  \param[in] HIDInterfaceInfo  Pointer to the HID class interface configuration structure being referenced
 *  \param[in] ReportID    Report ID of the received report from the host
 *  \param[in] ReportType  The type of report that the host has sent, either HID_REPORT_ITEM_Out or HID_REPORT_ITEM_Feature
 *  \param[in] ReportData  Pointer to a buffer where the received report has been stored
 *  \param[in] ReportSize  Size in bytes of the received HID report
 */
void CALLBACK_HID_Device_ProcessHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
                                          const uint8_t ReportID,
                                          const uint8_t ReportType,
                                          const void* ReportData,
                                          const uint16_t ReportSize)
{
	// Unused (but mandatory for the HID class driver) in this demo, since there are no Host->Device reports
}

