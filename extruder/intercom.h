#ifndef	_INTERCOM_H
#define	_INTERCOM_H

#include	<stdint.h>
#include	<avr/io.h>
#include	<avr/pgmspace.h>
#include	<avr/interrupt.h>


// initialise serial subsystem
void intercom_init(void);

//Update the message we are sending over intercom
void update_send_cmd(uint8_t new_send_cmd);

void start_send(void);

//Read the message we are receiving over intercom
uint8_t get_read_cmd(void);

#endif	/* _INTERCOM_H */
