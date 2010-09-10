#ifndef	_ANALOG_H
#define	_ANALOG_H

#include	<stdint.h>

#define	REFERENCE_AREF	0
#define	REFERENCE_AVCC	64
#define	REFERENCE_1V1		192


#define REFERENCE REFERENCE_AVCC

void 			analog_init(void);
uint16_t	analog_read(uint8_t channel);

#endif	/* _ANALOG_H */
