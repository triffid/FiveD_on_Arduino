#ifndef	_PINOUT_H
#define	_PINOUT_H

#include	"arduino.h"

//RS485 Interface pins
#define RX_ENABLE_PIN DIO4
#define TX_ENABLE_PIN AIO2

// Control pins for the A3949 chips
#define H1D DIO7
#define H1E DIO5
#define H2D DIO8
#define H2E DIO6

//Trimpot is on AIO0, pin 23
#define TRIM_POT AIO0

//Debug LED will blink on RS485 transmission
#define DEBUG_LED DIO13

//Read analog voltage from thermistor
#define TEMP_PIN AIO3

// list of PWM-able pins and corresponding timers
// timer1 is used for step timing so don't use OC1A/OC1B (DIO9/DIO10)
// OC0A												DIO6
// OC0B												DIO5
// OC1A												DIO9
// OC1B												DIO10
// OC2A												DIO11
// OC2B												DIO3

#define	HEATER_PIN						DIO11
#define	HEATER_PWM						OCR2A

/*
	Heater
*/

#ifdef	HEATER_PWM
	#define	enable_heater()			do { TCCR2A |=  MASK(COM2A1); } while (0)
	#define	disable_heater()		do { TCCR2A &= ~MASK(COM2A1); } while (0)
#else
	#define	enable_heater()			WRITE(HEATER_PIN, 1)
	#define	disable_heater()		WRITE(HEATER_PIN, 0)
#endif


#endif	/* _PINOUT_H */
