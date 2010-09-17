#ifndef	_TEMP_HEATER_LIST_H
#define	_TEMP_HEATER_LIST_H

#include	<stdint.h>

#define	NUM_TEMP_HEATERS	4

typedef struct temp_heater {
		uint8_t						temp_type;
		volatile uint8_t	*temp_port;
		uint8_t						temp_pin;

		volatile uint8_t	*heater_port;
		uint8_t						heater_pin;
		volatile uint8_t	*heater_pwm;

		uint16_t					next_read_time;
};

temp_heater temp_heaters[NUM_TEMP_HEATERS];

void temp_heater_init(void);
void temp_heater_tick(void);

#endif	/* _TEMP_HEATER_LIST_H */
