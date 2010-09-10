#include	<stddef.h>
#include	<stdint.h>
#include	<string.h>

#include	<avr/io.h>
#include	<avr/interrupt.h>

#include "pinout.h"

void io_init(void) {
	// disable modules we don't use
	PRR = MASK(PRTWI);
	ACSR = MASK(ACD);

	// setup I/O pins
	WRITE(X_STEP_PIN, 0);	SET_OUTPUT(X_STEP_PIN);
	WRITE(X_DIR_PIN,  0);	SET_OUTPUT(X_DIR_PIN);
	WRITE(X_MIN_PIN,  1);	SET_INPUT(X_MIN_PIN);

	WRITE(Y_STEP_PIN, 0);	SET_OUTPUT(Y_STEP_PIN);
	WRITE(Y_DIR_PIN,  0);	SET_OUTPUT(Y_DIR_PIN);
	WRITE(Y_MIN_PIN,  1);	SET_INPUT(Y_MIN_PIN);

	WRITE(Z_STEP_PIN, 0);	SET_OUTPUT(Z_STEP_PIN);
	WRITE(Z_DIR_PIN,  0);	SET_OUTPUT(Z_DIR_PIN);
	WRITE(Z_MIN_PIN,  1);	SET_INPUT(Z_MIN_PIN);

	WRITE(E_STEP_PIN, 0);	SET_OUTPUT(E_STEP_PIN);
	WRITE(E_DIR_PIN,  0);	SET_OUTPUT(E_DIR_PIN);

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

	// set up serial
	serial_init();

	// set up inputs and outputs
	io_init();

	// enable interrupts
	sei();

	// reset watchdog
	wd_reset();
}

void clock_250ms(void) {
	// reset watchdog
	wd_reset();
}

int main (void)
{
	init();

	// main loop
	for (;;)
	{
		// if queue is full, no point in reading chars- host will just have to wait
		/*if ((serial_rxchars() != 0) && (queue_full() == 0)) {
			uint8_t c = serial_popchar();
			scan_char(c);
		}*/

		ifclock(CLOCK_FLAG_250MS) {
			clock_250ms();
		}
	}
}
