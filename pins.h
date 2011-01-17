#ifndef	_PINS_H
#define	_PINS_H

#include	<stdint.h>

typedef struct {
	uint8_t	*port;
	uint8_t	pin;
	uint8_t	asserted;
} pin;

uint8_t	read_pin(pin *p);

void set_output(pin *p);
void set_input(pin *p);

void assert_pin(pin *p);
void deassert_pin(pin *p);

void write_pin(pin *p, uint8_t value);

#endif	/* _PINS_H */
