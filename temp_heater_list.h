#ifndef	_TEMP_HEATER_LIST_H
#define	_TEMP_HEATER_LIST_H

#include	<stdint.h>

#include	"machine.h"

/*
	NOTES

	no point in specifying a port- all the different temp sensors we have must be on a particular port. The MAX6675 must be on the SPI, and the thermistor and AD595 must be on an analog port.

	we still need to specify which analog pins we use in machine.h for the analog sensors however, otherwise the analog subsystem won't read them.
*/

void temp_heater_init(void);
void temp_heater_tick(void);
void temp_heater_save_settings(void);
uint8_t	temp_achieved(void);

void temp_set(uint8_t index, uint16_t temperature);
void temp_print(uint8_t index);

void pid_set_p(uint8_t index, int32_t p);
void pid_set_i(uint8_t index, int32_t i);
void pid_set_d(uint8_t index, int32_t d);
void pid_set_i_limit(uint8_t index, int32_t i_limit);

#endif	/* _TEMP_HEATER_LIST_H */
