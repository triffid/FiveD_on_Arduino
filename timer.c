#include	"timer.h"

#include	<avr/interrupt.h>

#include	"dda_queue.h"
#include	"watchdog.h"

volatile uint32_t	next_step_time;

uint8_t						clock_counter_250ms = 0;
uint8_t						clock_counter_1s = 0;
volatile uint8_t	clock_flag = 0;

ISR(TIMER1_COMPA_vect) {
	if (next_step_time > 64000)
		next_step_time -= 64000;
	else {
		if (next_step_time > 0) {
			OCR1B = next_step_time & 0xFFFF;
			TIMSK1 |= MASK(OCIE1B);
		}
	}
}

ISR(TIMER1_COMPB_vect) {
	// led on
	WRITE(SCK, 1);

	// disable this interrupt. if we set a new timeout, it will be re-enabled when appropriate
	TIMSK1 &= ~MASK(OCIE1B);

	// ensure we don't interrupt again unless timer is reset
	next_step_time = 0;

	/*
		clock stuff
	*/
	clock_counter_250ms += 2;
	if (clock_counter_250ms >= 250) {
		clock_counter_250ms -= 250;
		clock_flag |= CLOCK_FLAG_250MS;
		clock_counter_1s += 1;
		if (clock_counter_1s >= 4) {
			clock_counter_1s -= 4;
			clock_flag |= CLOCK_FLAG_1S;
		}
	}

	/*
		stepper tick
	*/
	queue_step();

	// led off
	WRITE(SCK, 0);
}

void timer_init()
{
	// no outputs
	TCCR1A = 0;
	// CTC mode
	TCCR1B = MASK(WGM12) | MASK(CS10);
	// COMPA interrupt only
	TIMSK1 = MASK(OCIE1A);
	// F_CPU / 250 = 2ms
	OCR1A = F_CPU / 500;
}

void setTimer(uint32_t delay)
{
	// save interrupt flag
	uint8_t sreg = SREG;
	// disable interrupts
	cli();

	if (delay > 0) {
		// mangle timer variables
		next_step_time = delay + TCNT1;
		if (delay <= 16) {
			// force interrupt
			// FIXME: datasheet says force compare doesn't work in CTC mode, find best way to do this
			TIMSK1 |= MASK(OCIE1B);
			TCCR1C |= MASK(FOC1B);
		}
		else if (delay <= 64000) {
			TIMSK1 |= MASK(OCIE1B);
		}
	}
	else {
		next_step_time = 0;
	}

	// restore interrupt flag
	SREG = sreg;
}

void timer_stop() {
	// disable all interrupts
	TIMSK1 = 0;
	// turn timer off
	TCCR1B = 0;
}
