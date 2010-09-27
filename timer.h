#ifndef	_TIMER_H
#define	_TIMER_H

#include	<stdint.h>
#ifndef SIMULATION
	#include	<avr/io.h>
#endif
#include "simulation.h"

// time-related constants
#define	US	* (F_CPU / 1000000)
#define	MS	* (F_CPU / 1000)

/*
	clock stuff
*/
extern volatile uint8_t	clock_flag;

#define	CLOCK_FLAG_250MS							1
#define	CLOCK_FLAG_1S									2
#define	ifclock(F)	for (;clock_flag & (F);clock_flag &= ~(F))

/*
	timer stuff
*/
void timer_init(void) __attribute__ ((cold));

void setTimer(uint32_t delay);

void timer_stop(void);

// #define enableTimerInterrupt()	do { TIMSK1 |= (1<<OCIE1A); } while (0)
// #define disableTimerInterrupt() do { TIMSK1 &= ~(1<<OCIE1A); } while (0)

#endif	/* _TIMER_H */
