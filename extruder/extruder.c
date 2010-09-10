#include	<stddef.h>
#include	<stdint.h>
#include	<string.h>

#include	<avr/io.h>
#include	<avr/interrupt.h>

#include "pinout.h"
#include "timer.h"
#include "serial.h"
#include "analog.h"
#include "watchdog.h"

void io_init(void) {


	// setup I/O pins
	WRITE(DEBUG_LED, 0); SET_OUTPUT(DEBUG_LED);
	WRITE(H1D,0); SET_OUTPUT(H1D);
	WRITE(H1E,0); SET_OUTPUT(H1E);
	WRITE(H2D,0); SET_OUTPUT(H2D);
	WRITE(H2E,0); SET_OUTPUT(H2E);

	SET_INPUT(TRIM_POT);
	SET_INPUT(TEMP_PIN);

	#ifdef	HEATER_PIN
		WRITE(HEATER_PIN, 0); SET_OUTPUT(HEATER_PIN);
	#endif

	#if defined(HEATER_PWM) || defined(FAN_PWM)
		// setup PWM timer: fast PWM, no prescaler
		TCCR2A = MASK(WGM21) | MASK(WGM20);
		TCCR2B = MASK(CS20);
		TIMSK2 = 0;
		OCR2A = 0;
		OCR2B = 255;
	#endif


}

void init(void) {

	// set up watchdog
	wd_init();

	// setup analog reading
	analog_init();

	// set up serial
	serial_init();

	// set up inputs and outputs
	io_init();

	// enable interrupts
	sei();

	// reset watchdog
	wd_reset();
}

int main (void)
{
	init();



	// main loop
	for (;;)
	{
		wd_reset();
		enable_heater();

		uint16_t trim = analog_read(TRIM_POT_CHANNEL);
		//Test the trim pot and pwm output
		HEATER_PWM = trim >> 2;

		if (trim > 800) WRITE(DEBUG_LED,1);
		else              WRITE(DEBUG_LED,0);


		// if queue is full, no point in reading chars- host will just have to wait
		/*if ((serial_rxchars() != 0) && (queue_full() == 0)) {
			uint8_t c = serial_popchar();
			scan_char(c);
		}*/

	
	}
}
