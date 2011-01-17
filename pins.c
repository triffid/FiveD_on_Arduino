#include	"pins.h"

uint8_t	read_pin(pin *p) {
	if (p->pin) {
		if (*(p->port - 2) & p->pin)
			return p->asserted;
		else
			return p->asserted ^ 255;
	}
	return 0;
}

void set_output(pin *p) {
	if (p->pin)
		*(p->port - 1) |= p->pin;
}

void set_input(pin *p) {
	if (p->pin)
		*(p->port - 1) &= ~(p->pin);
}

void assert_pin(pin *p) {
	if (p->pin) {
		if (p->asserted)
			*p->port |= p->pin;
		else
			*p->port &= ~(p->pin);
	}
}

void deassert_pin(pin *p) {
	if (p->pin) {
		if (p->asserted)
			*p->port &= ~(p->pin);
		else
			*p->port |= p->pin;
	}
}

void write_pin(pin *p, uint8_t value) {
	if (p->pin) {
		if (value)
			*p->port |= p->pin;
		else
			*p->port &= ~(p->pin);
	}
}
